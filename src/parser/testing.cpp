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
        auto tree = parsing.program()->singleExpression();
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

replxx::Replxx::completions_t hook_completion(const std::string &context, int &contextLen,
                                              const replxx::Replxx &rx, scope::Scope &scope)
{
    hint::Hints hints;
    std::string input = rx.get_state().text();
    int pos = rx.get_state().cursor_position();
    while (pos > 0 && input.at(pos - 1) == ' ') {
        --pos;
    }
    if (pos > 0) {
        antlr4::ANTLRInputStream ais(input);
        parser::lexers lexer(&ais);
        lexer.removeErrorListeners();
        ErrorListener lerr(true);
        lexer.addErrorListener(&lerr);
        antlr4::CommonTokenStream tokens(&lexer);
        parser::parsers parsing(&tokens);
        parsing.removeErrorListeners();
        parsing.addErrorListener(&lerr);
        auto tree = parsing.program();
        if (auto caret = hint::getTreeFromPos(tree, pos - 1, 1)) {
            hints = hint::completion(&parsing, *caret, scope);
        }
    } else {
        hints = hint::completeExpr(scope);
    }
    size_t fixed = 0;
    for (const auto &h: hints) {
        fixed = std::max(h.text.size(), fixed);
    }
    std::ostringstream ss;
    for (const auto &h: hints) {
        ss << fmt::format("{:{}s}   {}\n", h.text, fixed, h.note);
    }
    replxx::Replxx::completions_t comps;
    if (hints.size() > 0) {
        comps.push_back({ss.str(), replxx::Replxx::Color::GRAY});
        comps.push_back("");
    }
    return comps;
}

int main(int argc, char **argv)
{
    using namespace std::placeholders;
    replxx::Replxx rx;
    scope::LinearScope scope;
    rx.set_completion_callback(std::bind(&hook_completion, _1, _2, std::cref(rx), std::ref(scope)));

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
