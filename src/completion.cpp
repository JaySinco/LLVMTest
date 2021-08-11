#include "completion.h"
#include "utils.h"

CodeCompletion::CodeCompletion(antlr4::Parser &parser): parser(parser) {}

CandidatesCollection CodeCompletion::collectCandidates(size_t caretTokenIndex,
                                                       antlr4::ParserRuleContext *context)
{
    this->shortcutMap.clear();
    this->candidates.rules.clear();
    this->candidates.tokens.clear();
    this->statesProcessed = 0;
    this->precedenceStack.clear();
    this->tokens.clear();

    this->tokenStartIndex = context != nullptr ? context->start->getTokenIndex() : 0;
    antlr4::TokenStream *tokenStream = this->parser.getTokenStream();

    size_t offset = this->tokenStartIndex;
    while (true) {
        antlr4::Token *token = tokenStream->get(offset++);
        if (token->getChannel() == antlr4::Token::DEFAULT_CHANNEL) {
            this->tokens.push_back(token);

            if (token->getTokenIndex() >= caretTokenIndex ||
                token->getType() == antlr4::Token::EOF) {
                break;
            }
        }
        if (token->getType() == antlr4::Token::EOF) {
            break;
        }
    }

    RuleWithStartTokenList callStack;
    size_t startRule = context != nullptr ? context->getRuleIndex() : 0;
    this->processRule(this->parser.getATN().ruleToStartState[startRule], 0, callStack, 0, 0);
    if (this->showResult) {
        // to do...
    }
    return this->candidates;
}
