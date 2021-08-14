#include "codegen.h"
#include "utils.h"
#include "gen-cpp/TLexer.h"

CodeGen::CodeGen()
    : llctx(std::make_unique<llvm::LLVMContext>()),
      module(std::make_unique<llvm::Module>("demo", *llctx)),
      builder(std::make_unique<llvm::IRBuilder<>>(*llctx)),
      passManager(std::make_unique<llvm::legacy::FunctionPassManager>(module.get()))
{
    // this->passManager->add(llvm::createInstructionCombiningPass());
    // this->passManager->add(llvm::createReassociatePass());
    // this->passManager->add(llvm::createGVNPass());
    // this->passManager->add(llvm::createCFGSimplificationPass());
    // this->passManager->doInitialization();
}

antlrcpp::Any CodeGen::visitLiteralExpression(TParser::LiteralExpressionContext *ctx)
{
    double num = std::stod(ctx->Number()->getText());
    return static_cast<llvm::Value *>(llvm::ConstantFP::get(*this->llctx, llvm::APFloat(num)));
}

antlrcpp::Any CodeGen::visitIdExpression(TParser::IdExpressionContext *ctx)
{
    std::string id = ctx->Identifier()->getText();
    if (this->namedValues.find(id) == this->namedValues.end()) {
        throw std::runtime_error("unknown variable: {}"_format(id));
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
            o = this->builder->CreateFCmpULT(l, r);
            break;
        case TLexer::Greater:
            o = this->builder->CreateFCmpUGT(l, r);
            break;
        case TLexer::LessEqual:
            o = this->builder->CreateFCmpULE(l, r);
            break;
        case TLexer::GreaterEqual:
            o = this->builder->CreateFCmpUGE(l, r);
            break;
        default:
            throw std::runtime_error("unknown op: {}"_format(ctx->op->getText()));
    }
    return this->builder->CreateUIToFP(o, llvm::Type::getDoubleTy(*this->llctx), "bool");
}

antlrcpp::Any CodeGen::visitEqualityExpression(TParser::EqualityExpressionContext *ctx)
{
    llvm::Value *l = this->visit(ctx->expression(0));
    llvm::Value *r = this->visit(ctx->expression(1));
    llvm::Value *o = nullptr;
    switch (ctx->op->getType()) {
        case TLexer::Equal:
            o = this->builder->CreateFCmpUEQ(l, r);
            break;
        case TLexer::NotEqual:
            o = this->builder->CreateFCmpUNE(l, r);
            break;
        default:
            throw std::runtime_error("unknown op: {}"_format(ctx->op->getText()));
    }
    return this->builder->CreateUIToFP(o, llvm::Type::getDoubleTy(*this->llctx), "bool");
}

antlrcpp::Any CodeGen::visitMultiplicativeExpression(TParser::MultiplicativeExpressionContext *ctx)
{
    llvm::Value *l = this->visit(ctx->expression(0));
    llvm::Value *r = this->visit(ctx->expression(1));
    switch (ctx->op->getType()) {
        case TLexer::Star:
            return this->builder->CreateFMul(l, r);
        case TLexer::Div:
            return this->builder->CreateFDiv(l, r);
        default:
            throw std::runtime_error("unknown op: {}"_format(ctx->op->getText()));
    }
}

antlrcpp::Any CodeGen::visitAdditiveExpression(TParser::AdditiveExpressionContext *ctx)
{
    llvm::Value *l = this->visit(ctx->expression(0));
    llvm::Value *r = this->visit(ctx->expression(1));
    switch (ctx->op->getType()) {
        case TLexer::Plus:
            return this->builder->CreateFAdd(l, r);
        case TLexer::Minus:
            return this->builder->CreateFSub(l, r);
        default:
            throw std::runtime_error("unknown op: {}"_format(ctx->op->getText()));
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

antlrcpp::Any CodeGen::visitCallExpression(TParser::CallExpressionContext *ctx)
{
    std::string name = ctx->Identifier()->getText();
    llvm::Function *callee = this->module->getFunction(name);
    if (!callee) {
        throw std::runtime_error("unknown function: {}"_format(name));
    }
    std::vector<TParser::ExpressionContext *> exprList;
    if (ctx->expressionList() != nullptr) {
        exprList = ctx->expressionList()->expression();
    }
    if (callee->arg_size() != exprList.size()) {
        throw std::runtime_error(
            "{} expected {} args, got {}"_format(name, callee->arg_size(), exprList.size()));
    }
    std::vector<llvm::Value *> argv;
    std::transform(
        exprList.begin(), exprList.end(), std::back_inserter(argv),
        [&](TParser::ExpressionContext *c) { return static_cast<llvm::Value *>(this->visit(c)); });
    return static_cast<llvm::Value *>(this->builder->CreateCall(callee, argv));
}

antlrcpp::Any CodeGen::visitFunctionSignature(TParser::FunctionSignatureContext *ctx)
{
    std::vector<antlr4::tree::TerminalNode *> idList;
    if (ctx->argumentList() != nullptr) {
        idList = ctx->argumentList()->Identifier();
    }
    std::vector<llvm::Type *> paramType(idList.size(), llvm::Type::getDoubleTy(*this->llctx));
    llvm::FunctionType *funcType =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(*this->llctx), paramType, false);
    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                                  ctx->Identifier()->getText(), this->module.get());
    for (int i = 0; i < idList.size(); ++i) {
        func->getArg(i)->setName(idList.at(i)->getText());
    }
    return func;
}

antlrcpp::Any CodeGen::visitExternalFunction(TParser::ExternalFunctionContext *ctx)
{
    this->visit(ctx->functionSignature());
    return nullptr;
}

antlrcpp::Any CodeGen::visitFunctionDefinition(TParser::FunctionDefinitionContext *ctx)
{
    std::string funcName = ctx->functionSignature()->Identifier()->getText();
    if (this->module->getFunction(funcName) != nullptr) {
        throw std::runtime_error("function already exist: {}"_format(funcName));
    }
    llvm::Function *func = this->visit(ctx->functionSignature());
    llvm::BasicBlock *block = llvm::BasicBlock::Create(*this->llctx, "entry", func);
    this->builder->SetInsertPoint(block);
    this->namedValues.clear();
    for (auto &arg: func->args()) {
        this->namedValues[arg.getName().str()] = &arg;
    }
    llvm::Value *ret = this->visit(ctx->expression());
    this->builder->CreateRet(ret);
    std::string errmsg;
    llvm::raw_string_ostream ss(errmsg);
    if (llvm::verifyFunction(*func, &ss)) {
        throw std::runtime_error(errmsg);
    }
    this->passManager->run(*func);
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
