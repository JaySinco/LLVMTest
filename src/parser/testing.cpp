#include "./hint.h"
#include <replxx.hxx>
#include <optional>

class ErrorListener: public antlr4::BaseErrorListener
{
public:
    ErrorListener(bool allowFail): allowFail(allowFail){};

    void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
                     size_t charPositionInLine, const std::string &msg,
                     std::exception_ptr e) override
    {
        if (!allowFail) {
            throw type::Error{line, charPositionInLine, msg};
        }
    }

private:
    bool allowFail;
};

bool eval(const std::string &code, scope::Scope &scope)
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
        if (auto prop = expr::infer(tree, scope)) {
            std::cout << "(" << (*prop).type->toString() << ((*prop).lvalue ? " &" : "") << ")"
                      << std::endl;
        } else {
            throw prop.error();
        }
        return true;
    } catch (type::Error &e) {
        std::string spaces(4 + e.charPosInLine, ' ');
        std::cerr << fmt::format("{}^\n{}:{} {}\n", spaces, e.line, e.charPosInLine, e.msg);
        return false;
    }
}

replxx::Replxx::hints_t hook_hint(std::string const &context, int &contextLen,
                                  replxx::Replxx::Color &color, const replxx::Replxx &rx)
{
    replxx::Replxx::hints_t hints;
    std::string input = rx.get_state().text();
    antlr4::ANTLRInputStream ais(input);
    parser::lexers lexer(&ais);
    lexer.removeErrorListeners();
    ErrorListener lerr(true);
    lexer.addErrorListener(&lerr);
    antlr4::CommonTokenStream tokens(&lexer);
    parser::parsers parsing(&tokens);
    parsing.removeErrorListeners();
    parsing.addErrorListener(&lerr);
    auto tree = parsing.singleExpression();
    int pos = rx.get_state().cursor_position();
    while (pos > 0 && input.at(pos - 1) == ' ') {
        --pos;
    }
    if (pos > 0) {
        if (auto caret = hint::getTreeFromPos(tree, pos - 1, 1)) {
            if (auto pl = hint::completion(&parsing, *caret)) {
                hints.push_back(*pl);
                hints.push_back("");
            }
        }
    }
    return hints;
}

int main(int argc, char **argv)
{
    using namespace std::placeholders;
    replxx::Replxx rx;
    rx.set_max_hint_rows(5);
    rx.set_hint_callback(std::bind(&hook_hint, _1, _2, _3, std::cref(rx)));
    scope::LinearScope scope;

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
        eval(line, scope);
    }
    return 0;
}
