#include "../utils.h"
#include ".antlr/lexers.h"
#include ".antlr/parsers.h"
#include "replxx.hxx"

class ErrorListener: public antlr4::BaseErrorListener
{
public:
    ErrorListener(bool allowFail): allowFail(allowFail){};

    void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
                     size_t charPositionInLine, const std::string &msg,
                     std::exception_ptr e) override
    {
        std::string location(4 + charPositionInLine, ' ');
        location += "^";
        if (!allowFail) {
            throw std::runtime_error(
                fmt::format("{}\nline {}:{} {}\n", location, line, charPositionInLine, msg));
        }
    }

private:
    bool allowFail;
};

bool eval(const std::string &code)
{
    try {
        antlr4::ANTLRInputStream ais(code);
        parser::lexers lexer(&ais);
        lexer.removeErrorListeners();
        ErrorListener lerr(false);
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

replxx::Replxx::completions_t completion(const std::string &context, int &contextLen,
                                         const replxx::Replxx &rx)
{
    replxx::Replxx::completions_t completions;
    antlr4::ANTLRInputStream ais(rx.get_state().text());
    parser::lexers lexer(&ais);
    lexer.removeErrorListeners();
    ErrorListener lerr(true);
    lexer.addErrorListener(&lerr);
    antlr4::CommonTokenStream tokens(&lexer);
    parser::parsers parsing(&tokens);
    parsing.removeErrorListeners();
    parsing.addErrorListener(&lerr);
    auto tree = parsing.singleExpression();
    int curPos = rx.get_state().cursor_position();
    return completions;
}

int main(int argc, char **argv)
{
    using namespace std::placeholders;
    replxx::Replxx rx;
    rx.set_completion_callback(std::bind(&completion, _1, _2, std::cref(rx)));

    while (true) {
        const char *text = nullptr;
        do {
            text = rx.input(">>> ");
        } while ((text == nullptr) && (errno == EAGAIN));

        if (text == nullptr) {
            break;
        }
        std::string line(text);
        if (line.size() <= 0) {
            continue;
        }
        rx.history_add(line);
        eval(line);
    }
    return 0;
}
