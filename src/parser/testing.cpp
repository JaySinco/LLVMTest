#include "../utils.h"
#include ".antlr/lexers.h"
#include ".antlr/parsers.h"
#include "replxx.hxx"
#include <optional>

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

std::optional<antlr4::tree::ParseTree *> getTreeFromPos(antlr4::tree::ParseTree *root,
                                                        size_t col /* 0..n-1 */,
                                                        size_t row /* 1..n   */)
{
    if (auto node = dynamic_cast<antlr4::tree::TerminalNode *>(root)) {
        auto tok = node->getSymbol();
        if (tok->getLine() != row) {
            return {};
        }
        if (auto last = tok->getCharPositionInLine() + tok->getStopIndex() - tok->getStartIndex();
            tok->getCharPositionInLine() <= col && last >= col) {
            return node;
        }
        return {};
    } else {
        auto ctx = dynamic_cast<antlr4::ParserRuleContext *>(root);
        auto start = ctx->getStart();
        auto stop = ctx->getStop();
        if (start == nullptr || stop == nullptr) {
            return {};
        }
        if (start->getLine() > row ||
            (start->getLine() == row && col < start->getCharPositionInLine())) {
            return {};
        }
        auto last = stop->getCharPositionInLine() + stop->getStopIndex() - stop->getStartIndex();
        if (stop->getLine() < row || (stop->getLine() == row && last < col)) {
            return {};
        }
        for (auto child: ctx->children) {
            if (auto target = getTreeFromPos(child, col, row)) {
                return target;
            }
        }
        return ctx;
    }
}

int indexOf(antlr4::tree::TerminalNode *comma, parser::parsers::ElementListContext *parent)
{
    int count = 0;
    for (auto child: parent->children) {
        if (auto node = dynamic_cast<antlr4::tree::TerminalNode *>(child)) {
            if (node->getSymbol()->getType() == parser::lexers::Comma) {
                ++count;
            }
        }
        if (child == comma) {
            break;
        }
    }
    return count;
}

std::optional<std::string> completion(antlr4::Parser *parser, antlr4::tree::ParseTree *caret)
{
    if (auto node = dynamic_cast<antlr4::tree::TerminalNode *>(caret)) {
        auto type = node->getSymbol()->getType();
        switch (type) {
            case parser::lexers::Dot:
                if (auto memberDotExpr =
                        dynamic_cast<parser::parsers::MemberDotExpressionContext *>(node->parent)) {
                    return fmt::format(
                        "<MemberDot> completion based on => \n{}",
                        memberDotExpr->singleExpression()->toStringTree(parser, true));
                }
                break;

            case parser::lexers::Comma:
                if (auto elemList =
                        dynamic_cast<parser::parsers::ElementListContext *>(node->parent)) {
                    if (auto argsExpr = dynamic_cast<parser::parsers::ArgumentsExpressionContext *>(
                            elemList->parent)) {
                        return fmt::format(
                            "<Arguments> completion based on {}th arg of func => \n{}",
                            indexOf(node, elemList) + 1,
                            argsExpr->singleExpression()->toStringTree(parser, true));
                    }
                }
                break;

            case parser::lexers::OpenParen:
                if (auto argsExpr =
                        dynamic_cast<parser::parsers::ArgumentsExpressionContext *>(node->parent)) {
                    return fmt::format("<Arguments> completion based on 1st arg of func => \n{}",
                                       argsExpr->singleExpression()->toStringTree(parser, true));
                }

            default:
                break;
        }
    }
    return {};
}

replxx::Replxx::completions_t hook_completion(const std::string &context, int &contextLen,
                                              const replxx::Replxx &rx)
{
    replxx::Replxx::completions_t completions;
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
        if (auto caret = getTreeFromPos(tree, pos - 1, 1)) {
            if (auto pl = completion(&parsing, *caret)) {
                completions.push_back(*pl);
                completions.push_back("");
            }
        }
    }
    return completions;
}

int main(int argc, char **argv)
{
    using namespace std::placeholders;
    replxx::Replxx rx;
    rx.set_completion_callback(std::bind(&hook_completion, _1, _2, std::cref(rx)));

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
