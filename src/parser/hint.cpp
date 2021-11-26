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

Hints completion(antlr4::Parser *parser, antlr4::tree::ParseTree *caret)
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
                if (auto exprSeq =
                        dynamic_cast<parser::parsers::ExpressionSequenceContext *>(node->parent)) {
                    if (auto argsExpr = dynamic_cast<parser::parsers::ArgumentsExpressionContext *>(
                            exprSeq->parent)) {
                        return fmt::format(
                            "<Arguments> completion based on {}th arg of func => \n{}",
                            indexOf(node, exprSeq) + 1,
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

}  // namespace hint
