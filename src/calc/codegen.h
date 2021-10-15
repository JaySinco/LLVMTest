#pragma once
#include "prec.h"
#include "jit-compiler.h"
#include ".antlr/TParser.h"

class CodeGen
{
public:
    CodeGen();
    bool eval(const std::string &code);
    bool evalModule(const std::string &ir);

private:
    void generate(TParser::ProgramContext *ctx);
    void generate(TParser::StatementContext *ctx);
    void generate(TParser::ExternalFunctionContext *ctx);
    void generate(TParser::FunctionDefinitionContext *ctx);
    void generate(TParser::ExpressionStatementContext *ctx);
    llvm::Value *generate(TParser::ExpressionContext *ctx);
    llvm::Value *generate(TParser::CallExpressionContext *ctx);
    llvm::Value *generate(TParser::ParenthesesExpressionContext *ctx);
    llvm::Value *generate(TParser::AdditiveExpressionContext *ctx);
    llvm::Value *generate(TParser::LiteralExpressionContext *ctx);
    llvm::Value *generate(TParser::IdExpressionContext *ctx);
    llvm::Value *generate(TParser::RelationalExpressionContext *ctx);
    llvm::Value *generate(TParser::ConditionalExpressionContext *ctx);
    llvm::Value *generate(TParser::EqualityExpressionContext *ctx);
    llvm::Value *generate(TParser::MultiplicativeExpressionContext *ctx);
    llvm::Function *generate(TParser::FunctionSignatureContext *ctx);

    void initModuleAndPass();
    void printFunction(llvm::Function *func);
    llvm::Function *getFunction(const std::string &name);
    bool writeFunctionBody(llvm::Function *func, TParser::ExpressionContext *expr);

    llvm::LLVMContext llctx;
    llvm::IRBuilder<> builder{llctx};
    JITCompiler jit;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::legacy::FunctionPassManager> passManager;
    std::map<std::string, llvm::Value *> namedValues;
    std::map<std::string, std::vector<std::string>> signatures;
};
