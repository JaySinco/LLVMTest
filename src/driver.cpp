#include "utils.h"
#include "codegen.h"
#include "antlr4-runtime.h"
#include "gen-cpp/TLexer.h"
#include "gen-cpp/TParser.h"
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
        TLexer lexer(&ais);
        lexer.removeErrorListeners();
        ErrorListener lerr;
        lexer.addErrorListener(&lerr);
        antlr4::CommonTokenStream tokens(&lexer);
        TParser parser(&tokens);
        parser.removeErrorListeners();
        parser.addErrorListener(&lerr);
        auto tree = parser.program();
        LOG(INFO) << std::endl << tree->toStringTree(&parser, true);
        CodeGen codeGen;
        codeGen.visit(tree);
        codeGen.module->print(llvm::errs(), nullptr);
    } catch (std::exception &e) {
        LOG(ERROR) << e.what();
    }
}
