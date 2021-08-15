#include "codegen.h"
#include "utils.h"
#include "gen-cpp/TLexer.h"

CodeGen::CodeGen() { this->initModuleAndPass(); }

void CodeGen::initModuleAndPass()
{
    this->module_ = std::make_unique<llvm::Module>("demo", this->llctx);
    this->module_->setDataLayout(this->jit.getTargetMachine().createDataLayout());
    this->passManager = std::make_unique<llvm::legacy::FunctionPassManager>(module_.get());
    // this->passManager->add(llvm::createInstructionCombiningPass());
    // this->passManager->add(llvm::createReassociatePass());
    // this->passManager->add(llvm::createGVNPass());
    // this->passManager->add(llvm::createCFGSimplificationPass());
    this->passManager->doInitialization();
}

antlrcpp::Any CodeGen::visitLiteralExpression(TParser::LiteralExpressionContext *ctx)
{
    double num = std::stod(ctx->Number()->getText());
    return static_cast<llvm::Value *>(llvm::ConstantFP::get(this->llctx, llvm::APFloat(num)));
}

antlrcpp::Any CodeGen::visitIdExpression(TParser::IdExpressionContext *ctx)
{
    std::string id = ctx->Identifier()->getText();
    if (this->namedValues.find(id) == this->namedValues.end()) {
        THROW_ERROR("unknown variable: {}"_format(id));
    }
    return this->namedValues.at(id);
}

antlrcpp::Any CodeGen::visitRelationalExpression(TParser::RelationalExpressionContext *ctx)
{
    llvm::Value *l = this->visit(ctx->expression(0));
    llvm::Value *r = this->visit(ctx->expression(1));
    llvm::Value *o = nullptr;
    switch (ctx->op->getType()) {
        case TLexer::Less:
            o = this->builder.CreateFCmpULT(l, r);
            break;
        case TLexer::Greater:
            o = this->builder.CreateFCmpUGT(l, r);
            break;
        case TLexer::LessEqual:
            o = this->builder.CreateFCmpULE(l, r);
            break;
        case TLexer::GreaterEqual:
            o = this->builder.CreateFCmpUGE(l, r);
            break;
        default:
            THROW_ERROR("unknown op: {}"_format(ctx->op->getText()));
    }
    return this->builder.CreateUIToFP(o, llvm::Type::getDoubleTy(this->llctx), "bool");
}

antlrcpp::Any CodeGen::visitEqualityExpression(TParser::EqualityExpressionContext *ctx)
{
    llvm::Value *l = this->visit(ctx->expression(0));
    llvm::Value *r = this->visit(ctx->expression(1));
    llvm::Value *o = nullptr;
    switch (ctx->op->getType()) {
        case TLexer::Equal:
            o = this->builder.CreateFCmpUEQ(l, r);
            break;
        case TLexer::NotEqual:
            o = this->builder.CreateFCmpUNE(l, r);
            break;
        default:
            THROW_ERROR("unknown op: {}"_format(ctx->op->getText()));
    }
    return this->builder.CreateUIToFP(o, llvm::Type::getDoubleTy(this->llctx), "bool");
}

antlrcpp::Any CodeGen::visitMultiplicativeExpression(TParser::MultiplicativeExpressionContext *ctx)
{
    llvm::Value *l = this->visit(ctx->expression(0));
    llvm::Value *r = this->visit(ctx->expression(1));
    switch (ctx->op->getType()) {
        case TLexer::Star:
            return this->builder.CreateFMul(l, r);
        case TLexer::Div:
            return this->builder.CreateFDiv(l, r);
        default:
            THROW_ERROR("unknown op: {}"_format(ctx->op->getText()));
    }
}

antlrcpp::Any CodeGen::visitAdditiveExpression(TParser::AdditiveExpressionContext *ctx)
{
    llvm::Value *l = this->visit(ctx->expression(0));
    llvm::Value *r = this->visit(ctx->expression(1));
    switch (ctx->op->getType()) {
        case TLexer::Plus:
            return this->builder.CreateFAdd(l, r);
        case TLexer::Minus:
            return this->builder.CreateFSub(l, r);
        default:
            THROW_ERROR("unknown op: {}"_format(ctx->op->getText()));
    }
}

antlrcpp::Any CodeGen::visitParenthesesExpression(TParser::ParenthesesExpressionContext *ctx)
{
    return this->visit(ctx->expression());
}

antlrcpp::Any CodeGen::visitConditionalExpression(TParser::ConditionalExpressionContext *ctx)
{
    return nullptr;
}

llvm::Function *CodeGen::getFunction(const std::string &name)
{
    if (auto *func = this->module_->getFunction(name)) {
        return func;
    }
    auto &args = this->signatures.at(name);
    std::vector<llvm::Type *> paramType(args.size(), llvm::Type::getDoubleTy(this->llctx));
    llvm::FunctionType *funcType =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(this->llctx), paramType, false);

    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name,
                                                  this->module_.get());
    for (int i = 0; i < args.size(); ++i) {
        func->getArg(i)->setName(args.at(i));
    }
    return func;
}

antlrcpp::Any CodeGen::visitCallExpression(TParser::CallExpressionContext *ctx)
{
    std::string name = ctx->Identifier()->getText();
    if (this->signatures.find(name) == this->signatures.end()) {
        THROW_ERROR("unknown function: {}"_format(name));
    }
    auto &args = this->signatures[name];
    std::vector<TParser::ExpressionContext *> exprList;
    if (ctx->expressionList() != nullptr) {
        exprList = ctx->expressionList()->expression();
    }
    if (args.size() != exprList.size()) {
        THROW_ERROR("{} expected {} args, got {}"_format(name, args.size(), exprList.size()));
    }
    std::vector<llvm::Value *> argv;
    std::transform(
        exprList.begin(), exprList.end(), std::back_inserter(argv),
        [&](TParser::ExpressionContext *c) { return static_cast<llvm::Value *>(this->visit(c)); });
    return static_cast<llvm::Value *>(this->builder.CreateCall(this->getFunction(name), argv));
}

antlrcpp::Any CodeGen::visitFunctionSignature(TParser::FunctionSignatureContext *ctx)
{
    std::vector<antlr4::tree::TerminalNode *> idList;
    if (ctx->argumentList() != nullptr) {
        idList = ctx->argumentList()->Identifier();
    }
    std::string funcName = ctx->Identifier()->getText();
    auto &args = this->signatures[funcName];
    for (int i = 0; i < idList.size(); ++i) {
        args.push_back(idList.at(i)->getText());
    }
    return getFunction(funcName);
}

antlrcpp::Any CodeGen::visitExternalFunction(TParser::ExternalFunctionContext *ctx)
{
    this->visit(ctx->functionSignature());
    return nullptr;
}

antlrcpp::Any CodeGen::visitFunctionDefinition(TParser::FunctionDefinitionContext *ctx)
{
    std::string funcName = ctx->functionSignature()->Identifier()->getText();
    if (this->signatures.find(funcName) != this->signatures.end()) {
        THROW_ERROR("function already exist: {}"_format(funcName));
    }
    llvm::Function *func = this->visit(ctx->functionSignature());
    llvm::BasicBlock *block = llvm::BasicBlock::Create(this->llctx, "entry", func);
    this->builder.SetInsertPoint(block);
    this->namedValues.clear();
    for (auto &arg: func->args()) {
        this->namedValues[arg.getName().str()] = &arg;
    }
    llvm::Value *ret = this->visit(ctx->expression());
    this->builder.CreateRet(ret);
    std::string errmsg;
    llvm::raw_string_ostream ss(errmsg);
    if (llvm::verifyFunction(*func, &ss)) {
        THROW_ERROR(errmsg);
    }
    this->passManager->run(*func);
    this->module_->print(llvm::errs(), nullptr);
    this->jit.addModule(std::move(this->module_));
    this->initModuleAndPass();
    return nullptr;
}

antlrcpp::Any CodeGen::visitExpressionStatement(TParser::ExpressionStatementContext *ctx)
{
    std::vector<llvm::Type *> paramType;
    llvm::FunctionType *funcType =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(this->llctx), paramType, false);
    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                                  "__anonymous", this->module_.get());
    llvm::BasicBlock *block = llvm::BasicBlock::Create(this->llctx, "entry", func);
    this->builder.SetInsertPoint(block);
    llvm::Value *ret = this->visit(ctx->expression());
    this->builder.CreateRet(ret);
    std::string errmsg;
    llvm::raw_string_ostream ss(errmsg);
    if (llvm::verifyFunction(*func, &ss)) {
        THROW_ERROR(errmsg);
    }
    this->passManager->run(*func);
    this->module_->print(llvm::errs(), nullptr);
    auto key = this->jit.addModule(std::move(this->module_));
    this->initModuleAndPass();
    auto sym = this->jit.findSymbol("__anonymous");
    if (!sym) {
        LOG(ERROR) << "failed to find symbol: __anonymous";
    }
    auto *fp = (double (*)())(intptr_t)llvm::cantFail(sym.getAddress());
    std::cout << fp() << std::endl;
    this->jit.removeModule(key);
    return nullptr;
}

antlrcpp::Any CodeGen::visitProgram(TParser::ProgramContext *ctx)
{
    std::vector<TParser::StatementContext *> statList = ctx->statement();
    for (auto &stat: statList) {
        this->visit(stat);
    }
    return nullptr;
}
