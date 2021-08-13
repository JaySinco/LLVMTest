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
        LOG(INFO) << std::endl << tree->toStringTree(&parser, true);
    } catch (const std::exception &err) {
        LOG(ERROR) << err.what();
    }
}
