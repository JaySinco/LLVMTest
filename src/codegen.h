#pragma once
#include "prec.h"
#include "jit-compiler.h"
#include "gen-cpp/TParserBaseVisitor.h"

class CodeGen: public TParserBaseVisitor
{
public:
    CodeGen();
    void initModuleAndPass();
    void printModule();
    llvm::Function *getFunction(const std::string &name);
    bool writeFunctionBody(llvm::Function *func, TParser::ExpressionContext *expr);

    antlrcpp::Any visitCallExpression(TParser::CallExpressionContext *ctx) override;
    antlrcpp::Any visitParenthesesExpression(TParser::ParenthesesExpressionContext *ctx) override;
    antlrcpp::Any visitAdditiveExpression(TParser::AdditiveExpressionContext *ctx) override;
    antlrcpp::Any visitLiteralExpression(TParser::LiteralExpressionContext *ctx) override;
    antlrcpp::Any visitIdExpression(TParser::IdExpressionContext *ctx) override;
    antlrcpp::Any visitRelationalExpression(TParser::RelationalExpressionContext *ctx) override;
    antlrcpp::Any visitConditionalExpression(TParser::ConditionalExpressionContext *ctx) override;
    antlrcpp::Any visitEqualityExpression(TParser::EqualityExpressionContext *ctx) override;
    antlrcpp::Any visitMultiplicativeExpression(
        TParser::MultiplicativeExpressionContext *ctx) override;
    antlrcpp::Any visitFunctionSignature(TParser::FunctionSignatureContext *ctx) override;
    antlrcpp::Any visitExternalFunction(TParser::ExternalFunctionContext *ctx) override;
    antlrcpp::Any visitFunctionDefinition(TParser::FunctionDefinitionContext *ctx) override;
    antlrcpp::Any visitExpressionStatement(TParser::ExpressionStatementContext *ctx) override;
    antlrcpp::Any visitProgram(TParser::ProgramContext *ctx) override;

    llvm::LLVMContext llctx;
    llvm::IRBuilder<> builder{llctx};
    JITCompiler jit;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::legacy::FunctionPassManager> passManager;
    std::map<std::string, llvm::Value *> namedValues;
    std::map<std::string, std::vector<std::string>> signatures;
};
