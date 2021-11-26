#include "./hint.h"

namespace hint
{
std::optional<antlr4::tree::ParseTree *> getTreeFromPos(antlr4::tree::ParseTree *root,
                                                        size_t charPosInLine, size_t line)
{
    if (auto node = dynamic_cast<antlr4::tree::TerminalNode *>(root)) {
        auto tok = node->getSymbol();
        if (tok->getLine() != line) {
            return {};
        }
        if (auto last = tok->getCharPositionInLine() + tok->getStopIndex() - tok->getStartIndex();
            tok->getCharPositionInLine() <= charPosInLine && last >= charPosInLine) {
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
        if (start->getLine() > line ||
            (start->getLine() == line && charPosInLine < start->getCharPositionInLine())) {
            return {};
        }
        auto last = stop->getCharPositionInLine() + stop->getStopIndex() - stop->getStartIndex();
        if (stop->getLine() < line || (stop->getLine() == line && last < charPosInLine)) {
            return {};
        }
        for (auto child: ctx->children) {
            if (auto target = getTreeFromPos(child, charPosInLine, line)) {
                return target;
            }
        }
        return ctx;
    }
}

int indexOf(antlr4::tree::TerminalNode *comma, parser::parsers::ExpressionSequenceContext *parent)
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

Hints completeExpr(const scope::Scope &scope)
{
    Hints hints;
    for (const auto &[k, v]: scope.getAllVar()) {
        hints.push_back(Hint{k, v->toString()});
    }
    return hints;
}

Hints completeAfterDot(antlr4::tree::ParseTree *parent, const scope::Scope &scope)
{
    if (auto memberDotExpr = dynamic_cast<parser::parsers::MemberDotExpressionContext *>(parent)) {
        if (auto expr = memberDotExpr->singleExpression()) {
            if (auto type = expr::infer(expr, scope)) {
                if (auto stru = dynamic_cast<type::Struct *>(type->type.get())) {
                    Hints hints;
                    for (const auto &[k, v]: stru->fields) {
                        hints.push_back(Hint{k, v->toString()});
                    }
                    return hints;
                }
            }
        }
    }
    return {};
}

Hints completeAfterComma(antlr4::tree::ParseTree *parent, const scope::Scope &scope)
{
    if (auto exprSeq = dynamic_cast<parser::parsers::ExpressionSequenceContext *>(parent)) {
        if (auto argsExpr =
                dynamic_cast<parser::parsers::ArgumentsExpressionContext *>(exprSeq->parent)) {
            if (auto type = expr::infer(argsExpr->singleExpression(), scope)) {
                if (auto func = dynamic_cast<type::Function *>(type->type.get())) {
                    Hints hints;
                    hints.push_back(Hint{"_", func->toString()});
                    auto hs = completeExpr(scope);
                    hints.insert(hints.end(), hs.begin(), hs.end());
                    return hints;
                }
            }
        } else {
            return completeExpr(scope);
        }
    }
    return {};
}

Hints completeAfterColon(antlr4::tree::ParseTree *parent, const scope::Scope &scope)
{
    if (auto ternaryExpr = dynamic_cast<parser::parsers::TernaryExpressionContext *>(parent)) {
        return completeExpr(scope);
    } else if (auto propAssign =
                   dynamic_cast<parser::parsers::PropertyAssignmentContext *>(parent)) {
        return completeExpr(scope);
    }
    return {};
}

Hints completeAfterOpenParen(antlr4::tree::ParseTree *parent, const scope::Scope &scope)
{
    if (auto argsExpr = dynamic_cast<parser::parsers::ArgumentsExpressionContext *>(parent)) {
        if (auto type = expr::infer(argsExpr->singleExpression(), scope)) {
            if (auto func = dynamic_cast<type::Function *>(type->type.get())) {
                Hints hints;
                hints.push_back(Hint{"_", func->toString()});
                auto vars = completeExpr(scope);
                hints.insert(hints.end(), vars.begin(), vars.end());
                return hints;
            }
        }
    } else if (auto parenExpr =
                   dynamic_cast<parser::parsers::ParenthesizedExpressionContext *>(parent)) {
        return completeExpr(scope);
    }
    return {};
}

Hints completeAfterOpenBracket(antlr4::tree::ParseTree *parent, const scope::Scope &scope)
{
    if (auto memberIdxExpr =
            dynamic_cast<parser::parsers::MemberIndexExpressionContext *>(parent)) {
        return completeExpr(scope);
    } else if (auto arrLitExpr =
                   dynamic_cast<parser::parsers::ArrayLiteralExpressionContext *>(parent)) {
        return completeExpr(scope);
    }
    return {};
}

Hints completion(antlr4::Parser *parser, antlr4::tree::ParseTree *caret, const scope::Scope &scope)
{
    if (auto node = dynamic_cast<antlr4::tree::TerminalNode *>(caret)) {
        auto type = node->getSymbol()->getType();
        switch (type) {
            case parser::lexers::Dot:
                return completeAfterDot(node->parent, scope);
            case parser::lexers::Comma:
                return completeAfterComma(node->parent, scope);
            case parser::lexers::OpenParen:
                return completeAfterOpenParen(node->parent, scope);
            case parser::lexers::Colon:
                return completeAfterColon(node->parent, scope);
            case parser::lexers::OpenBracket:
                return completeAfterOpenBracket(node->parent, scope);
            default:
                break;
        }
    }
    return {};
}

}  // namespace hint
