#include "codegen.h"
#include "utils.h"
#include "gen-cpp/TLexer.h"
#define NullVal static_cast<llvm::Value *>(nullptr);

CodeGen::CodeGen() { this->initModuleAndPass(); }

void CodeGen::initModuleAndPass()
{
    static int id = 0;
    this->module_ = std::make_unique<llvm::Module>("__module_{}"_format(id++), this->llctx);
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
        LOG(ERROR) << "unknown variable: " << id;
        return NullVal;
    }
    return this->namedValues.at(id);
}

antlrcpp::Any CodeGen::visitRelationalExpression(TParser::RelationalExpressionContext *ctx)
{
    llvm::Value *l = this->visit(ctx->expression(0));
    if (l == nullptr) return NullVal;
    llvm::Value *r = this->visit(ctx->expression(1));
    if (r == nullptr) return NullVal;
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
            LOG(ERROR) << "unknow op: " << ctx->op->getText();
            return NullVal;
    }
    return this->builder.CreateUIToFP(o, llvm::Type::getDoubleTy(this->llctx), "bool");
}

antlrcpp::Any CodeGen::visitEqualityExpression(TParser::EqualityExpressionContext *ctx)
{
    llvm::Value *l = this->visit(ctx->expression(0));
    if (l == nullptr) return NullVal;
    llvm::Value *r = this->visit(ctx->expression(1));
    if (r == nullptr) return NullVal;
    llvm::Value *o = nullptr;
    switch (ctx->op->getType()) {
        case TLexer::Equal:
            o = this->builder.CreateFCmpUEQ(l, r);
            break;
        case TLexer::NotEqual:
            o = this->builder.CreateFCmpUNE(l, r);
            break;
        default:
            LOG(ERROR) << "unknow op: " << ctx->op->getText();
            return NullVal;
    }
    return this->builder.CreateUIToFP(o, llvm::Type::getDoubleTy(this->llctx), "bool");
}

antlrcpp::Any CodeGen::visitMultiplicativeExpression(TParser::MultiplicativeExpressionContext *ctx)
{
    llvm::Value *l = this->visit(ctx->expression(0));
    if (l == nullptr) return NullVal;
    llvm::Value *r = this->visit(ctx->expression(1));
    if (r == nullptr) return NullVal;
    switch (ctx->op->getType()) {
        case TLexer::Star:
            return this->builder.CreateFMul(l, r);
        case TLexer::Div:
            return this->builder.CreateFDiv(l, r);
        default:
            LOG(ERROR) << "unknow op: " << ctx->op->getText();
            return NullVal;
    }
}

antlrcpp::Any CodeGen::visitAdditiveExpression(TParser::AdditiveExpressionContext *ctx)
{
    llvm::Value *l = this->visit(ctx->expression(0));
    if (l == nullptr) return NullVal;
    llvm::Value *r = this->visit(ctx->expression(1));
    if (r == nullptr) return NullVal;
    switch (ctx->op->getType()) {
        case TLexer::Plus:
            return this->builder.CreateFAdd(l, r);
        case TLexer::Minus:
            return this->builder.CreateFSub(l, r);
        default:
            LOG(ERROR) << "unknow op: " << ctx->op->getText();
            return NullVal;
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
    if (this->signatures.find(name) == this->signatures.end()) {
        LOG(ERROR) << "unknow function signature: " << name;
        return nullptr;
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
        LOG(ERROR) << "unknown function: " << name;
        return NullVal;
    }
    auto &args = this->signatures[name];
    std::vector<TParser::ExpressionContext *> exprList;
    if (ctx->expressionList() != nullptr) {
        exprList = ctx->expressionList()->expression();
    }
    if (args.size() != exprList.size()) {
        LOG(ERROR) << "{} expected {} args, got {}"_format(name, args.size(), exprList.size());
        return NullVal;
    }
    std::vector<llvm::Value *> argv;
    for (auto expr: exprList) {
        llvm::Value *v = this->visit(expr);
        if (v == nullptr) {
            return NullVal;
        }
        argv.push_back(v);
    }
    llvm::Function *func = this->getFunction(name);
    if (func == nullptr) {
        return NullVal;
    }
    return static_cast<llvm::Value *>(this->builder.CreateCall(func, argv));
}

void CodeGen::printModule()
{
    std::string str;
    llvm::raw_string_ostream ss(str);
    this->module_->print(ss, nullptr);
    VLOG(1) << "module =>\n" << str;
}

bool CodeGen::writeFunctionBody(llvm::Function *func, TParser::ExpressionContext *expr)
{
    llvm::BasicBlock *block = llvm::BasicBlock::Create(this->llctx, "entry", func);
    this->builder.SetInsertPoint(block);
    this->namedValues.clear();
    for (auto &arg: func->args()) {
        this->namedValues[arg.getName().str()] = &arg;
    }
    llvm::Value *retval = this->visit(expr);
    if (retval == nullptr) {
        VLOG(1) << "failed to generate function, erase it";
        func->eraseFromParent();
        return false;
    }
    this->builder.CreateRet(retval);
    std::string errmsg;
    llvm::raw_string_ostream ss(errmsg);
    if (llvm::verifyFunction(*func, &ss)) {
        LOG(ERROR) << "failed to verify function " << func->getName().str() << ": " << errmsg;
        return false;
    }
    this->passManager->run(*func);
    return true;
}

antlrcpp::Any CodeGen::visitFunctionSignature(TParser::FunctionSignatureContext *ctx)
{
    std::string funcName = ctx->Identifier()->getText();
    if (this->signatures.find(funcName) != this->signatures.end()) {
        LOG(ERROR) << "function already exist: " << funcName;
        return static_cast<llvm::Function *>(nullptr);
    }
    std::vector<antlr4::tree::TerminalNode *> idList;
    if (ctx->argumentList() != nullptr) {
        idList = ctx->argumentList()->Identifier();
    }
    auto &args = this->signatures[funcName];
    for (int i = 0; i < idList.size(); ++i) {
        args.push_back(idList.at(i)->getText());
    }
    return getFunction(funcName);
}

antlrcpp::Any CodeGen::visitExternalFunction(TParser::ExternalFunctionContext *ctx)
{
    llvm::Function *func = this->visit(ctx->functionSignature());
    if (func != nullptr) {
        this->printModule();
    }
    return nullptr;
}

antlrcpp::Any CodeGen::visitFunctionDefinition(TParser::FunctionDefinitionContext *ctx)
{
    std::string funcName = ctx->functionSignature()->Identifier()->getText();
    llvm::Function *func = this->visit(ctx->functionSignature());
    if (func == nullptr) {
        return nullptr;
    }
    if (!this->writeFunctionBody(func, ctx->expression())) {
        return nullptr;
    }
    this->printModule();
    this->jit.addModule(std::move(this->module_));
    this->initModuleAndPass();
    return nullptr;
}

antlrcpp::Any CodeGen::visitExpressionStatement(TParser::ExpressionStatementContext *ctx)
{
    std::vector<llvm::Type *> paramType;
    llvm::FunctionType *funcType =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(this->llctx), paramType, false);
    const char *funcName = "__anonymous";
    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                                  funcName, this->module_.get());

    if (!this->writeFunctionBody(func, ctx->expression())) {
        return nullptr;
    }
    this->printModule();
    auto key = this->jit.addModule(std::move(this->module_));
    this->initModuleAndPass();
    if (!key) {
        return nullptr;
    }
    std::shared_ptr<void> moduleGuard(nullptr, [&](void *) { this->jit.removeModule(*key); });
    auto symbol = this->jit.findSymbol(funcName);
    auto addr = symbol.getAddress();
    if (!addr) {
        LOG(ERROR) << "failed to get address: " << llvm::toString(addr.takeError());
        return nullptr;
    }
    auto *fp = (double (*)())(intptr_t)*addr;
    if (fp == nullptr) {
        LOG(ERROR) << "symbol address is NULL";
        return nullptr;
    }
    std::cout << fp() << std::endl;
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
