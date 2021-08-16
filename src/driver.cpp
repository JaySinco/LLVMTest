#include "utils.h"
#include "codegen.h"
#include "antlr4-runtime.h"
#include "gen-cpp/TLexer.h"
#include "gen-cpp/TParser.h"
#include <boost/algorithm/string/trim.hpp>
#include <fstream>

class ErrorListener: public antlr4::BaseErrorListener
{
    virtual void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol,
                             size_t line, size_t charPositionInLine, const std::string &msg,
                             std::exception_ptr e) override
    {
        THROW_ERROR("line {}:{} {}"_format(line, charPositionInLine, msg));
    }
};

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    llvm::ExitOnError exitOnErr;

    auto cg = std::make_unique<CodeGen>();
    while (true) {
        try {
            std::cout << ">> ";
            std::string line;
            std::getline(std::cin, line);
            boost::trim_right(line);
            if (line.back() != ';') {
                line.push_back(';');
            }
            antlr4::ANTLRInputStream ais(line);
            TLexer lexer(&ais);
            lexer.removeErrorListeners();
            ErrorListener lerr;
            lexer.addErrorListener(&lerr);
            antlr4::CommonTokenStream tokens(&lexer);
            TParser parser(&tokens);
            parser.removeErrorListeners();
            parser.addErrorListener(&lerr);
            auto tree = parser.program();
            VLOG(1) << "ast-tree =>\n" << tree->toStringTree(&parser, true);
            cg->visit(tree);
        } catch (std::exception &e) {
            LOG(ERROR) << e.what();
        }
    }
}
