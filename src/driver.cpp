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
        Throw_Error("line {}:{} {}"_format(line, charPositionInLine, msg));
    }
};

void eval(const std::string &code, CodeGen *codegen)
{
    try {
        antlr4::ANTLRInputStream ais(code);
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
        codegen->visit(tree);
    } catch (std::exception &e) {
        LOG(ERROR) << e.what();
    }
}

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    auto codegen = std::make_unique<CodeGen>();
    eval(utils::readFile(L"sample/input.txt").second, codegen.get());
    while (true) {
        std::cout << ">>> ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }
        boost::trim_right(line);
        if (line.back() != ';') {
            line.push_back(';');
        }
        eval(line, codegen.get());
    }
}
