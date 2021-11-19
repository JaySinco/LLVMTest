#include "../utils.h"
#include "antlr4-runtime.h"
#include ".antlr/lexers.h"
#include ".antlr/parsers.h"
#include <boost/algorithm/string/trim.hpp>

class ErrorListener: public antlr4::BaseErrorListener
{
    virtual void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol,
                             size_t line, size_t charPositionInLine, const std::string &msg,
                             std::exception_ptr e) override
    {
        std::string location(4 + charPositionInLine, ' ');
        location += "^";
        std::cout << fmt::format("{}\nline {}:{} {}\n", location, line, charPositionInLine, msg)
                  << std::endl;
    }
};

bool eval(const std::string &code)
{
    try {
        antlr4::ANTLRInputStream ais(code);
        parser::lexers lexer(&ais);
        lexer.removeErrorListeners();
        ErrorListener lerr;
        lexer.addErrorListener(&lerr);
        antlr4::CommonTokenStream tokens(&lexer);
        parser::parsers parsing(&tokens);
        parsing.removeErrorListeners();
        parsing.addErrorListener(&lerr);
        auto tree = parsing.singleExpression();
        std::cout << tree->toStringTree(&parsing, true) << std::endl;
        return true;
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char **argv)
{
    while (true) {
        std::cout << ">>> ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }
        boost::trim_right(line);
        if (line.size() <= 0) {
            continue;
        }
        eval(line);
    }
    return 0;
}
