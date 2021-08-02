#include "utils.h"
#include "antlr4-runtime.h"
#include "gen-cpp/TLexer.h"
#include "gen-cpp/TParser.h"
#include "gen-cpp/TParserBaseListener.h"
#include <fstream>

class ErrorListener: public antlr4::BaseErrorListener
{
    void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
                     size_t charPositionInLine, const std::string &msg,
                     std::exception_ptr e) override
    {
        throw std::runtime_error("line {}:{} {}"_format(line, charPositionInLine, msg));
    }
};

class WalkListener: public grammar::TParserBaseListener
{
public:
    void exitExpression(grammar::TParser::ExpressionContext *ctx)
    {
        int left = std::stoi(ctx->left->getText());
        int right = std::stoi(ctx->right->getText());
        if (ctx->PLUS() != nullptr) {
            LOG(INFO) << left + right;
        } else {
            LOG(INFO) << left - right;
        }
    }
};

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    try {
        ErrorListener lerr;
        antlr4::ANTLRInputStream ais(argv[1]);
        grammar::TLexer lexer(&ais);
        lexer.removeErrorListeners();
        lexer.addErrorListener(&lerr);
        antlr4::CommonTokenStream tokens(&lexer);
        grammar::TParser parser(&tokens);
        parser.removeErrorListeners();
        parser.addErrorListener(&lerr);
        WalkListener lwalk;
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&lwalk, parser.expression());
    } catch (const std::exception &err) {
        LOG(ERROR) << err.what();
    }
}
