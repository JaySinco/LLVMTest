#include "utils.h"
#include "antlr4-runtime.h"
#include "gen-cpp/TLexer.h"
#include "gen-cpp/TParser.h"
#include "gen-cpp/TParserBaseListener.h"
#include "gen-cpp/TParserBaseVisitor.h"
#include <fstream>

class ErrorListener: public antlr4::BaseErrorListener
{
    virtual void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol,
                             size_t line, size_t charPositionInLine, const std::string &msg,
                             std::exception_ptr e) override
    {
        throw std::runtime_error("line {}:{} {}"_format(line, charPositionInLine, msg));
    }
};

class WalkListener: public grammar::TParserBaseListener
{
public:
};

class EvalVisitor: public grammar::TParserBaseVisitor
{
public:
    virtual antlrcpp::Any visitPrintExpr(grammar::TParser::PrintExprContext *ctx) override
    {
        LOG(INFO) << ctx->expr()->getText() << " = " << visit(ctx->expr()).as<double>();
        return 0.0;
    }

    virtual antlrcpp::Any visitClear(grammar::TParser::ClearContext *ctx) override
    {
        this->vmap.clear();
        return 0.0;
    }

    virtual antlrcpp::Any visitAssign(grammar::TParser::AssignContext *ctx) override
    {
        std::string id = ctx->ID()->getText();
        double value = visit(ctx->expr());
        LOG(INFO) << id << " = " << value;
        this->vmap[id] = value;
        return value;
    }

    virtual antlrcpp::Any visitParens(grammar::TParser::ParensContext *ctx) override
    {
        return visit(ctx->expr());
    }

    virtual antlrcpp::Any visitMulDiv(grammar::TParser::MulDivContext *ctx) override
    {
        double left = visit(ctx->expr(0));
        double right = visit(ctx->expr(1));
        if (ctx->op->getType() == grammar::TParser::Mul) {
            return left * right;
        }
        return left / right;
    }

    virtual antlrcpp::Any visitAddSub(grammar::TParser::AddSubContext *ctx) override
    {
        double left = visit(ctx->expr(0));
        double right = visit(ctx->expr(1));
        if (ctx->op->getType() == grammar::TParser::Plus) {
            return left + right;
        }
        return left - right;
    }

    virtual antlrcpp::Any visitId(grammar::TParser::IdContext *ctx) override
    {
        std::string id = ctx->ID()->getText();
        if (vmap.count(id) <= 0) {
            throw std::runtime_error("variable not exist: {}"_format(id));
        };
        return this->vmap[id];
    }

    virtual antlrcpp::Any visitInt(grammar::TParser::IntContext *ctx) override
    {
        return std::stod(ctx->INT()->getText());
    }

private:
    std::map<std::string, double> vmap;
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
        auto tree = parser.prog();
        WalkListener lwalk;
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&lwalk, tree);
        LOG(INFO) << std::endl << tree->toStringTree(&parser, true);
        EvalVisitor ev;
        ev.visit(tree);
    } catch (const std::exception &err) {
        LOG(ERROR) << err.what();
    }
}
