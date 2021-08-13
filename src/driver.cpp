#include "utils.h"
#include "antlr4-runtime.h"
#include "gen-cpp/TLexer.h"
#include "gen-cpp/TParser.h"
#include "gen-cpp/TParserBaseListener.h"
#include "gen-cpp/TParserBaseVisitor.h"
#pragma warning(push)
#pragma warning(disable : 4626)
#pragma warning(disable : 4624)
#pragma warning(disable : 4996)
#pragma warning(disable : 4141)
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#pragma warning(pop)
#include <fstream>

using namespace grammar;

class ErrorListener: public antlr4::BaseErrorListener
{
    virtual void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol,
                             size_t line, size_t charPositionInLine, const std::string &msg,
                             std::exception_ptr e) override
    {
        throw std::runtime_error("line {}:{} {}"_format(line, charPositionInLine, msg));
    }
};

class CodeGenVisitor: public TParserBaseVisitor
{
public:
    CodeGenVisitor()
        : llctx(std::make_unique<llvm::LLVMContext>()),
          module(std::make_unique<llvm::Module>("demo", *llctx)),
          builder(std::make_unique<llvm::IRBuilder<>>(*llctx))
    {
    }

    virtual antlrcpp::Any visitExpressionStatement(
        TParser::ExpressionStatementContext *ctx) override
    {
        llvm::Value *v = this->visit(ctx->expression());
        return nullptr;
    }

    virtual antlrcpp::Any visitLiteralExpression(TParser::LiteralExpressionContext *ctx) override
    {
        double num = std::stod(ctx->Number()->getText());
        llvm::Value *val = llvm::ConstantFP::get(*this->llctx, llvm::APFloat(num));
        return val;
    }

    virtual antlrcpp::Any visitIdExpression(TParser::IdExpressionContext *ctx) override
    {
        std::string id = ctx->Identifier()->getText();
        if (this->namedValues.find(id) == this->namedValues.end()) {
            throw std::runtime_error("unknown variable name: {}"_format(id));
        }
        return this->namedValues.at(id);
    }

    std::unique_ptr<llvm::LLVMContext> llctx;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::map<std::string, llvm::Value *> namedValues;
};

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    try {
        std::ifstream fs("sample/input.txt");
        antlr4::ANTLRInputStream ais(fs);
        grammar::TLexer lexer(&ais);
        lexer.removeErrorListeners();
        ErrorListener lerr;
        lexer.addErrorListener(&lerr);
        antlr4::CommonTokenStream tokens(&lexer);
        grammar::TParser parser(&tokens);
        parser.removeErrorListeners();
        parser.addErrorListener(&lerr);
        auto tree = parser.program();
        LOG(INFO) << std::endl << tree->toStringTree(&parser, true);
        CodeGenVisitor generator;
        generator.visit(tree);
        generator.module->print(llvm::errs(), nullptr);
    } catch (const std::exception &err) {
        LOG(ERROR) << err.what();
    }
}
