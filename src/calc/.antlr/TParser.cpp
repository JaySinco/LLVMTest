
// Generated from d:\Jaysinco\Prototyping\src\calc\TParser.g4 by ANTLR 4.8

#include "TParser.h"

using namespace antlrcpp;
using namespace antlr4;

TParser::TParser(TokenStream *input): Parser(input)
{
    _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

TParser::~TParser() { delete _interpreter; }

std::string TParser::getGrammarFileName() const { return "TParser.g4"; }

const std::vector<std::string> &TParser::getRuleNames() const { return _ruleNames; }

dfa::Vocabulary &TParser::getVocabulary() const { return _vocabulary; }

//----------------- ProgramContext
//------------------------------------------------------------------

TParser::ProgramContext::ProgramContext(ParserRuleContext *parent, size_t invokingState)
    : ParserRuleContext(parent, invokingState)
{
}

tree::TerminalNode *TParser::ProgramContext::EOF() { return getToken(TParser::EOF, 0); }

std::vector<TParser::StatementContext *> TParser::ProgramContext::statement()
{
    return getRuleContexts<TParser::StatementContext>();
}

TParser::StatementContext *TParser::ProgramContext::statement(size_t i)
{
    return getRuleContext<TParser::StatementContext>(i);
}

size_t TParser::ProgramContext::getRuleIndex() const { return TParser::RuleProgram; }

TParser::ProgramContext *TParser::program()
{
    ProgramContext *_localctx = _tracker.createInstance<ProgramContext>(_ctx, getState());
    enterRule(_localctx, 0, TParser::RuleProgram);
    size_t _la = 0;

    auto onExit = finally([=] { exitRule(); });
    try {
        enterOuterAlt(_localctx, 1);
        setState(15);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while ((((_la & ~0x3fULL) == 0) &&
                ((1ULL << _la) & ((1ULL << TParser::Def) | (1ULL << TParser::Extern) |
                                  (1ULL << TParser::LeftParen) | (1ULL << TParser::Identifier) |
                                  (1ULL << TParser::Number))) != 0)) {
            setState(12);
            statement();
            setState(17);
            _errHandler->sync(this);
            _la = _input->LA(1);
        }
        setState(18);
        match(TParser::EOF);

    } catch (RecognitionException &e) {
        _errHandler->reportError(this, e);
        _localctx->exception = std::current_exception();
        _errHandler->recover(this, _localctx->exception);
    }

    return _localctx;
}

//----------------- StatementContext
//------------------------------------------------------------------

TParser::StatementContext::StatementContext(ParserRuleContext *parent, size_t invokingState)
    : ParserRuleContext(parent, invokingState)
{
}

size_t TParser::StatementContext::getRuleIndex() const { return TParser::RuleStatement; }

void TParser::StatementContext::copyFrom(StatementContext *ctx)
{
    ParserRuleContext::copyFrom(ctx);
}

//----------------- FunctionDefinitionContext
//------------------------------------------------------------------

tree::TerminalNode *TParser::FunctionDefinitionContext::Def() { return getToken(TParser::Def, 0); }

TParser::FunctionSignatureContext *TParser::FunctionDefinitionContext::functionSignature()
{
    return getRuleContext<TParser::FunctionSignatureContext>(0);
}

TParser::ExpressionContext *TParser::FunctionDefinitionContext::expression()
{
    return getRuleContext<TParser::ExpressionContext>(0);
}

tree::TerminalNode *TParser::FunctionDefinitionContext::Semi()
{
    return getToken(TParser::Semi, 0);
}

TParser::FunctionDefinitionContext::FunctionDefinitionContext(StatementContext *ctx)
{
    copyFrom(ctx);
}

//----------------- ExternalFunctionContext
//------------------------------------------------------------------

tree::TerminalNode *TParser::ExternalFunctionContext::Extern()
{
    return getToken(TParser::Extern, 0);
}

TParser::FunctionSignatureContext *TParser::ExternalFunctionContext::functionSignature()
{
    return getRuleContext<TParser::FunctionSignatureContext>(0);
}

tree::TerminalNode *TParser::ExternalFunctionContext::Semi() { return getToken(TParser::Semi, 0); }

TParser::ExternalFunctionContext::ExternalFunctionContext(StatementContext *ctx) { copyFrom(ctx); }

//----------------- ExpressionStatementContext
//------------------------------------------------------------------

TParser::ExpressionContext *TParser::ExpressionStatementContext::expression()
{
    return getRuleContext<TParser::ExpressionContext>(0);
}

tree::TerminalNode *TParser::ExpressionStatementContext::Semi()
{
    return getToken(TParser::Semi, 0);
}

TParser::ExpressionStatementContext::ExpressionStatementContext(StatementContext *ctx)
{
    copyFrom(ctx);
}

TParser::StatementContext *TParser::statement()
{
    StatementContext *_localctx = _tracker.createInstance<StatementContext>(_ctx, getState());
    enterRule(_localctx, 2, TParser::RuleStatement);

    auto onExit = finally([=] { exitRule(); });
    try {
        setState(32);
        _errHandler->sync(this);
        switch (_input->LA(1)) {
            case TParser::LeftParen:
            case TParser::Identifier:
            case TParser::Number: {
                _localctx = dynamic_cast<StatementContext *>(
                    _tracker.createInstance<TParser::ExpressionStatementContext>(_localctx));
                enterOuterAlt(_localctx, 1);
                setState(20);
                expression(0);
                setState(21);
                match(TParser::Semi);
                break;
            }

            case TParser::Extern: {
                _localctx = dynamic_cast<StatementContext *>(
                    _tracker.createInstance<TParser::ExternalFunctionContext>(_localctx));
                enterOuterAlt(_localctx, 2);
                setState(23);
                match(TParser::Extern);
                setState(24);
                functionSignature();
                setState(25);
                match(TParser::Semi);
                break;
            }

            case TParser::Def: {
                _localctx = dynamic_cast<StatementContext *>(
                    _tracker.createInstance<TParser::FunctionDefinitionContext>(_localctx));
                enterOuterAlt(_localctx, 3);
                setState(27);
                match(TParser::Def);
                setState(28);
                functionSignature();
                setState(29);
                expression(0);
                setState(30);
                match(TParser::Semi);
                break;
            }

            default:
                throw NoViableAltException(this);
        }

    } catch (RecognitionException &e) {
        _errHandler->reportError(this, e);
        _localctx->exception = std::current_exception();
        _errHandler->recover(this, _localctx->exception);
    }

    return _localctx;
}

//----------------- ExpressionContext
//------------------------------------------------------------------

TParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
    : ParserRuleContext(parent, invokingState)
{
}

size_t TParser::ExpressionContext::getRuleIndex() const { return TParser::RuleExpression; }

void TParser::ExpressionContext::copyFrom(ExpressionContext *ctx)
{
    ParserRuleContext::copyFrom(ctx);
}

//----------------- ParenthesesExpressionContext
//------------------------------------------------------------------

tree::TerminalNode *TParser::ParenthesesExpressionContext::LeftParen()
{
    return getToken(TParser::LeftParen, 0);
}

TParser::ExpressionContext *TParser::ParenthesesExpressionContext::expression()
{
    return getRuleContext<TParser::ExpressionContext>(0);
}

tree::TerminalNode *TParser::ParenthesesExpressionContext::RightParen()
{
    return getToken(TParser::RightParen, 0);
}

TParser::ParenthesesExpressionContext::ParenthesesExpressionContext(ExpressionContext *ctx)
{
    copyFrom(ctx);
}

//----------------- IdExpressionContext
//------------------------------------------------------------------

tree::TerminalNode *TParser::IdExpressionContext::Identifier()
{
    return getToken(TParser::Identifier, 0);
}

TParser::IdExpressionContext::IdExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

//----------------- CallExpressionContext
//------------------------------------------------------------------

tree::TerminalNode *TParser::CallExpressionContext::Identifier()
{
    return getToken(TParser::Identifier, 0);
}

tree::TerminalNode *TParser::CallExpressionContext::LeftParen()
{
    return getToken(TParser::LeftParen, 0);
}

tree::TerminalNode *TParser::CallExpressionContext::RightParen()
{
    return getToken(TParser::RightParen, 0);
}

TParser::ExpressionListContext *TParser::CallExpressionContext::expressionList()
{
    return getRuleContext<TParser::ExpressionListContext>(0);
}

TParser::CallExpressionContext::CallExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

//----------------- AdditiveExpressionContext
//------------------------------------------------------------------

std::vector<TParser::ExpressionContext *> TParser::AdditiveExpressionContext::expression()
{
    return getRuleContexts<TParser::ExpressionContext>();
}

TParser::ExpressionContext *TParser::AdditiveExpressionContext::expression(size_t i)
{
    return getRuleContext<TParser::ExpressionContext>(i);
}

tree::TerminalNode *TParser::AdditiveExpressionContext::Plus()
{
    return getToken(TParser::Plus, 0);
}

tree::TerminalNode *TParser::AdditiveExpressionContext::Minus()
{
    return getToken(TParser::Minus, 0);
}

TParser::AdditiveExpressionContext::AdditiveExpressionContext(ExpressionContext *ctx)
{
    copyFrom(ctx);
}

//----------------- RelationalExpressionContext
//------------------------------------------------------------------

std::vector<TParser::ExpressionContext *> TParser::RelationalExpressionContext::expression()
{
    return getRuleContexts<TParser::ExpressionContext>();
}

TParser::ExpressionContext *TParser::RelationalExpressionContext::expression(size_t i)
{
    return getRuleContext<TParser::ExpressionContext>(i);
}

tree::TerminalNode *TParser::RelationalExpressionContext::Less()
{
    return getToken(TParser::Less, 0);
}

tree::TerminalNode *TParser::RelationalExpressionContext::Greater()
{
    return getToken(TParser::Greater, 0);
}

tree::TerminalNode *TParser::RelationalExpressionContext::LessEqual()
{
    return getToken(TParser::LessEqual, 0);
}

tree::TerminalNode *TParser::RelationalExpressionContext::GreaterEqual()
{
    return getToken(TParser::GreaterEqual, 0);
}

TParser::RelationalExpressionContext::RelationalExpressionContext(ExpressionContext *ctx)
{
    copyFrom(ctx);
}

//----------------- ConditionalExpressionContext
//------------------------------------------------------------------

std::vector<TParser::ExpressionContext *> TParser::ConditionalExpressionContext::expression()
{
    return getRuleContexts<TParser::ExpressionContext>();
}

TParser::ExpressionContext *TParser::ConditionalExpressionContext::expression(size_t i)
{
    return getRuleContext<TParser::ExpressionContext>(i);
}

tree::TerminalNode *TParser::ConditionalExpressionContext::Question()
{
    return getToken(TParser::Question, 0);
}

tree::TerminalNode *TParser::ConditionalExpressionContext::Colon()
{
    return getToken(TParser::Colon, 0);
}

TParser::ConditionalExpressionContext::ConditionalExpressionContext(ExpressionContext *ctx)
{
    copyFrom(ctx);
}

//----------------- EqualityExpressionContext
//------------------------------------------------------------------

std::vector<TParser::ExpressionContext *> TParser::EqualityExpressionContext::expression()
{
    return getRuleContexts<TParser::ExpressionContext>();
}

TParser::ExpressionContext *TParser::EqualityExpressionContext::expression(size_t i)
{
    return getRuleContext<TParser::ExpressionContext>(i);
}

tree::TerminalNode *TParser::EqualityExpressionContext::Equal()
{
    return getToken(TParser::Equal, 0);
}

tree::TerminalNode *TParser::EqualityExpressionContext::NotEqual()
{
    return getToken(TParser::NotEqual, 0);
}

TParser::EqualityExpressionContext::EqualityExpressionContext(ExpressionContext *ctx)
{
    copyFrom(ctx);
}

//----------------- MultiplicativeExpressionContext
//------------------------------------------------------------------

std::vector<TParser::ExpressionContext *> TParser::MultiplicativeExpressionContext::expression()
{
    return getRuleContexts<TParser::ExpressionContext>();
}

TParser::ExpressionContext *TParser::MultiplicativeExpressionContext::expression(size_t i)
{
    return getRuleContext<TParser::ExpressionContext>(i);
}

tree::TerminalNode *TParser::MultiplicativeExpressionContext::Star()
{
    return getToken(TParser::Star, 0);
}

tree::TerminalNode *TParser::MultiplicativeExpressionContext::Div()
{
    return getToken(TParser::Div, 0);
}

TParser::MultiplicativeExpressionContext::MultiplicativeExpressionContext(ExpressionContext *ctx)
{
    copyFrom(ctx);
}

//----------------- LiteralExpressionContext
//------------------------------------------------------------------

tree::TerminalNode *TParser::LiteralExpressionContext::Number()
{
    return getToken(TParser::Number, 0);
}

TParser::LiteralExpressionContext::LiteralExpressionContext(ExpressionContext *ctx)
{
    copyFrom(ctx);
}

TParser::ExpressionContext *TParser::expression() { return expression(0); }

TParser::ExpressionContext *TParser::expression(int precedence)
{
    ParserRuleContext *parentContext = _ctx;
    size_t parentState = getState();
    TParser::ExpressionContext *_localctx =
        _tracker.createInstance<ExpressionContext>(_ctx, parentState);
    TParser::ExpressionContext *previousContext = _localctx;
    (void)previousContext;  // Silence compiler, in case the context is not used by generated code.
    size_t startState = 4;
    enterRecursionRule(_localctx, 4, TParser::RuleExpression, precedence);

    size_t _la = 0;

    auto onExit = finally([=] { unrollRecursionContexts(parentContext); });
    try {
        size_t alt;
        enterOuterAlt(_localctx, 1);
        setState(47);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx)) {
            case 1: {
                _localctx = _tracker.createInstance<LiteralExpressionContext>(_localctx);
                _ctx = _localctx;
                previousContext = _localctx;

                setState(35);
                match(TParser::Number);
                break;
            }

            case 2: {
                _localctx = _tracker.createInstance<IdExpressionContext>(_localctx);
                _ctx = _localctx;
                previousContext = _localctx;
                setState(36);
                match(TParser::Identifier);
                break;
            }

            case 3: {
                _localctx = _tracker.createInstance<CallExpressionContext>(_localctx);
                _ctx = _localctx;
                previousContext = _localctx;
                setState(37);
                match(TParser::Identifier);
                setState(38);
                match(TParser::LeftParen);
                setState(40);
                _errHandler->sync(this);

                _la = _input->LA(1);
                if ((((_la & ~0x3fULL) == 0) &&
                     ((1ULL << _la) &
                      ((1ULL << TParser::LeftParen) | (1ULL << TParser::Identifier) |
                       (1ULL << TParser::Number))) != 0)) {
                    setState(39);
                    expressionList();
                }
                setState(42);
                match(TParser::RightParen);
                break;
            }

            case 4: {
                _localctx = _tracker.createInstance<ParenthesesExpressionContext>(_localctx);
                _ctx = _localctx;
                previousContext = _localctx;
                setState(43);
                match(TParser::LeftParen);
                setState(44);
                expression(0);
                setState(45);
                match(TParser::RightParen);
                break;
            }
        }
        _ctx->stop = _input->LT(-1);
        setState(69);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx);
        while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
            if (alt == 1) {
                if (!_parseListeners.empty()) triggerExitRuleEvent();
                previousContext = _localctx;
                setState(67);
                _errHandler->sync(this);
                switch (
                    getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 4, _ctx)) {
                    case 1: {
                        auto newContext = _tracker.createInstance<MultiplicativeExpressionContext>(
                            _tracker.createInstance<ExpressionContext>(parentContext, parentState));
                        _localctx = newContext;
                        pushNewRecursionContext(newContext, startState, RuleExpression);
                        setState(49);

                        if (!(precpred(_ctx, 5)))
                            throw FailedPredicateException(this, "precpred(_ctx, 5)");
                        setState(50);
                        dynamic_cast<MultiplicativeExpressionContext *>(_localctx)->op =
                            _input->LT(1);
                        _la = _input->LA(1);
                        if (!(_la == TParser::Star

                              || _la == TParser::Div)) {
                            dynamic_cast<MultiplicativeExpressionContext *>(_localctx)->op =
                                _errHandler->recoverInline(this);
                        } else {
                            _errHandler->reportMatch(this);
                            consume();
                        }
                        setState(51);
                        expression(6);
                        break;
                    }

                    case 2: {
                        auto newContext = _tracker.createInstance<AdditiveExpressionContext>(
                            _tracker.createInstance<ExpressionContext>(parentContext, parentState));
                        _localctx = newContext;
                        pushNewRecursionContext(newContext, startState, RuleExpression);
                        setState(52);

                        if (!(precpred(_ctx, 4)))
                            throw FailedPredicateException(this, "precpred(_ctx, 4)");
                        setState(53);
                        dynamic_cast<AdditiveExpressionContext *>(_localctx)->op = _input->LT(1);
                        _la = _input->LA(1);
                        if (!(_la == TParser::Plus

                              || _la == TParser::Minus)) {
                            dynamic_cast<AdditiveExpressionContext *>(_localctx)->op =
                                _errHandler->recoverInline(this);
                        } else {
                            _errHandler->reportMatch(this);
                            consume();
                        }
                        setState(54);
                        expression(5);
                        break;
                    }

                    case 3: {
                        auto newContext = _tracker.createInstance<RelationalExpressionContext>(
                            _tracker.createInstance<ExpressionContext>(parentContext, parentState));
                        _localctx = newContext;
                        pushNewRecursionContext(newContext, startState, RuleExpression);
                        setState(55);

                        if (!(precpred(_ctx, 3)))
                            throw FailedPredicateException(this, "precpred(_ctx, 3)");
                        setState(56);
                        dynamic_cast<RelationalExpressionContext *>(_localctx)->op = _input->LT(1);
                        _la = _input->LA(1);
                        if (!((((_la & ~0x3fULL) == 0) &&
                               ((1ULL << _la) &
                                ((1ULL << TParser::Less) | (1ULL << TParser::Greater) |
                                 (1ULL << TParser::LessEqual) | (1ULL << TParser::GreaterEqual))) !=
                                   0))) {
                            dynamic_cast<RelationalExpressionContext *>(_localctx)->op =
                                _errHandler->recoverInline(this);
                        } else {
                            _errHandler->reportMatch(this);
                            consume();
                        }
                        setState(57);
                        expression(4);
                        break;
                    }

                    case 4: {
                        auto newContext = _tracker.createInstance<EqualityExpressionContext>(
                            _tracker.createInstance<ExpressionContext>(parentContext, parentState));
                        _localctx = newContext;
                        pushNewRecursionContext(newContext, startState, RuleExpression);
                        setState(58);

                        if (!(precpred(_ctx, 2)))
                            throw FailedPredicateException(this, "precpred(_ctx, 2)");
                        setState(59);
                        dynamic_cast<EqualityExpressionContext *>(_localctx)->op = _input->LT(1);
                        _la = _input->LA(1);
                        if (!(_la == TParser::Equal

                              || _la == TParser::NotEqual)) {
                            dynamic_cast<EqualityExpressionContext *>(_localctx)->op =
                                _errHandler->recoverInline(this);
                        } else {
                            _errHandler->reportMatch(this);
                            consume();
                        }
                        setState(60);
                        expression(3);
                        break;
                    }

                    case 5: {
                        auto newContext = _tracker.createInstance<ConditionalExpressionContext>(
                            _tracker.createInstance<ExpressionContext>(parentContext, parentState));
                        _localctx = newContext;
                        pushNewRecursionContext(newContext, startState, RuleExpression);
                        setState(61);

                        if (!(precpred(_ctx, 1)))
                            throw FailedPredicateException(this, "precpred(_ctx, 1)");
                        setState(62);
                        match(TParser::Question);
                        setState(63);
                        expression(0);
                        setState(64);
                        match(TParser::Colon);
                        setState(65);
                        expression(1);
                        break;
                    }
                }
            }
            setState(71);
            _errHandler->sync(this);
            alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx);
        }
    } catch (RecognitionException &e) {
        _errHandler->reportError(this, e);
        _localctx->exception = std::current_exception();
        _errHandler->recover(this, _localctx->exception);
    }
    return _localctx;
}

//----------------- ExpressionListContext
//------------------------------------------------------------------

TParser::ExpressionListContext::ExpressionListContext(ParserRuleContext *parent,
                                                      size_t invokingState)
    : ParserRuleContext(parent, invokingState)
{
}

std::vector<TParser::ExpressionContext *> TParser::ExpressionListContext::expression()
{
    return getRuleContexts<TParser::ExpressionContext>();
}

TParser::ExpressionContext *TParser::ExpressionListContext::expression(size_t i)
{
    return getRuleContext<TParser::ExpressionContext>(i);
}

std::vector<tree::TerminalNode *> TParser::ExpressionListContext::Comma()
{
    return getTokens(TParser::Comma);
}

tree::TerminalNode *TParser::ExpressionListContext::Comma(size_t i)
{
    return getToken(TParser::Comma, i);
}

size_t TParser::ExpressionListContext::getRuleIndex() const { return TParser::RuleExpressionList; }

TParser::ExpressionListContext *TParser::expressionList()
{
    ExpressionListContext *_localctx =
        _tracker.createInstance<ExpressionListContext>(_ctx, getState());
    enterRule(_localctx, 6, TParser::RuleExpressionList);
    size_t _la = 0;

    auto onExit = finally([=] { exitRule(); });
    try {
        enterOuterAlt(_localctx, 1);
        setState(72);
        expression(0);
        setState(77);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == TParser::Comma) {
            setState(73);
            match(TParser::Comma);
            setState(74);
            expression(0);
            setState(79);
            _errHandler->sync(this);
            _la = _input->LA(1);
        }

    } catch (RecognitionException &e) {
        _errHandler->reportError(this, e);
        _localctx->exception = std::current_exception();
        _errHandler->recover(this, _localctx->exception);
    }

    return _localctx;
}

//----------------- ArgumentListContext
//------------------------------------------------------------------

TParser::ArgumentListContext::ArgumentListContext(ParserRuleContext *parent, size_t invokingState)
    : ParserRuleContext(parent, invokingState)
{
}

std::vector<tree::TerminalNode *> TParser::ArgumentListContext::Identifier()
{
    return getTokens(TParser::Identifier);
}

tree::TerminalNode *TParser::ArgumentListContext::Identifier(size_t i)
{
    return getToken(TParser::Identifier, i);
}

std::vector<tree::TerminalNode *> TParser::ArgumentListContext::Comma()
{
    return getTokens(TParser::Comma);
}

tree::TerminalNode *TParser::ArgumentListContext::Comma(size_t i)
{
    return getToken(TParser::Comma, i);
}

size_t TParser::ArgumentListContext::getRuleIndex() const { return TParser::RuleArgumentList; }

TParser::ArgumentListContext *TParser::argumentList()
{
    ArgumentListContext *_localctx = _tracker.createInstance<ArgumentListContext>(_ctx, getState());
    enterRule(_localctx, 8, TParser::RuleArgumentList);
    size_t _la = 0;

    auto onExit = finally([=] { exitRule(); });
    try {
        enterOuterAlt(_localctx, 1);
        setState(80);
        match(TParser::Identifier);
        setState(85);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == TParser::Comma) {
            setState(81);
            match(TParser::Comma);
            setState(82);
            match(TParser::Identifier);
            setState(87);
            _errHandler->sync(this);
            _la = _input->LA(1);
        }

    } catch (RecognitionException &e) {
        _errHandler->reportError(this, e);
        _localctx->exception = std::current_exception();
        _errHandler->recover(this, _localctx->exception);
    }

    return _localctx;
}

//----------------- FunctionSignatureContext
//------------------------------------------------------------------

TParser::FunctionSignatureContext::FunctionSignatureContext(ParserRuleContext *parent,
                                                            size_t invokingState)
    : ParserRuleContext(parent, invokingState)
{
}

tree::TerminalNode *TParser::FunctionSignatureContext::Identifier()
{
    return getToken(TParser::Identifier, 0);
}

tree::TerminalNode *TParser::FunctionSignatureContext::LeftParen()
{
    return getToken(TParser::LeftParen, 0);
}

tree::TerminalNode *TParser::FunctionSignatureContext::RightParen()
{
    return getToken(TParser::RightParen, 0);
}

TParser::ArgumentListContext *TParser::FunctionSignatureContext::argumentList()
{
    return getRuleContext<TParser::ArgumentListContext>(0);
}

size_t TParser::FunctionSignatureContext::getRuleIndex() const
{
    return TParser::RuleFunctionSignature;
}

TParser::FunctionSignatureContext *TParser::functionSignature()
{
    FunctionSignatureContext *_localctx =
        _tracker.createInstance<FunctionSignatureContext>(_ctx, getState());
    enterRule(_localctx, 10, TParser::RuleFunctionSignature);
    size_t _la = 0;

    auto onExit = finally([=] { exitRule(); });
    try {
        enterOuterAlt(_localctx, 1);
        setState(88);
        match(TParser::Identifier);
        setState(89);
        match(TParser::LeftParen);
        setState(91);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == TParser::Identifier) {
            setState(90);
            argumentList();
        }
        setState(93);
        match(TParser::RightParen);

    } catch (RecognitionException &e) {
        _errHandler->reportError(this, e);
        _localctx->exception = std::current_exception();
        _errHandler->recover(this, _localctx->exception);
    }

    return _localctx;
}

bool TParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex)
{
    switch (ruleIndex) {
        case 2:
            return expressionSempred(dynamic_cast<ExpressionContext *>(context), predicateIndex);

        default:
            break;
    }
    return true;
}

bool TParser::expressionSempred(ExpressionContext *_localctx, size_t predicateIndex)
{
    switch (predicateIndex) {
        case 0:
            return precpred(_ctx, 5);
        case 1:
            return precpred(_ctx, 4);
        case 2:
            return precpred(_ctx, 3);
        case 3:
            return precpred(_ctx, 2);
        case 4:
            return precpred(_ctx, 1);

        default:
            break;
    }
    return true;
}

// Static vars and initialization.
std::vector<dfa::DFA> TParser::_decisionToDFA;
atn::PredictionContextCache TParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN TParser::_atn;
std::vector<uint16_t> TParser::_serializedATN;

std::vector<std::string> TParser::_ruleNames = {
    "program", "statement", "expression", "expressionList", "argumentList", "functionSignature"};

std::vector<std::string> TParser::_literalNames = {
    "",    "'def'", "'extern'", "'+'", "'-'", "'*'",  "'/'",  "';'",  "'('", "')'",
    "','", "'?'",   "':'",      "'<'", "'>'", "'=='", "'!='", "'<='", "'>='"};

std::vector<std::string> TParser::_symbolicNames = {
    "",           "Def",       "Extern",     "Plus",      "Minus",        "Star",       "Div",
    "Semi",       "LeftParen", "RightParen", "Comma",     "Question",     "Colon",      "Less",
    "Greater",    "Equal",     "NotEqual",   "LessEqual", "GreaterEqual", "Identifier", "Number",
    "Whitespace", "Newline"};

dfa::Vocabulary TParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> TParser::_tokenNames;

TParser::Initializer::Initializer()
{
    for (size_t i = 0; i < _symbolicNames.size(); ++i) {
        std::string name = _vocabulary.getLiteralName(i);
        if (name.empty()) {
            name = _vocabulary.getSymbolicName(i);
        }

        if (name.empty()) {
            _tokenNames.push_back("<INVALID>");
        } else {
            _tokenNames.push_back(name);
        }
    }

    _serializedATN = {
        0x3,  0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 0x3,  0x18, 0x62,
        0x4,  0x2,    0x9,    0x2,    0x4,    0x3,    0x9,    0x3,    0x4,    0x4,  0x9,  0x4,
        0x4,  0x5,    0x9,    0x5,    0x4,    0x6,    0x9,    0x6,    0x4,    0x7,  0x9,  0x7,
        0x3,  0x2,    0x7,    0x2,    0x10,   0xa,    0x2,    0xc,    0x2,    0xe,  0x2,  0x13,
        0xb,  0x2,    0x3,    0x2,    0x3,    0x2,    0x3,    0x3,    0x3,    0x3,  0x3,  0x3,
        0x3,  0x3,    0x3,    0x3,    0x3,    0x3,    0x3,    0x3,    0x3,    0x3,  0x3,  0x3,
        0x3,  0x3,    0x3,    0x3,    0x3,    0x3,    0x5,    0x3,    0x23,   0xa,  0x3,  0x3,
        0x4,  0x3,    0x4,    0x3,    0x4,    0x3,    0x4,    0x3,    0x4,    0x3,  0x4,  0x5,
        0x4,  0x2b,   0xa,    0x4,    0x3,    0x4,    0x3,    0x4,    0x3,    0x4,  0x3,  0x4,
        0x3,  0x4,    0x5,    0x4,    0x32,   0xa,    0x4,    0x3,    0x4,    0x3,  0x4,  0x3,
        0x4,  0x3,    0x4,    0x3,    0x4,    0x3,    0x4,    0x3,    0x4,    0x3,  0x4,  0x3,
        0x4,  0x3,    0x4,    0x3,    0x4,    0x3,    0x4,    0x3,    0x4,    0x3,  0x4,  0x3,
        0x4,  0x3,    0x4,    0x3,    0x4,    0x3,    0x4,    0x7,    0x4,    0x46, 0xa,  0x4,
        0xc,  0x4,    0xe,    0x4,    0x49,   0xb,    0x4,    0x3,    0x5,    0x3,  0x5,  0x3,
        0x5,  0x7,    0x5,    0x4e,   0xa,    0x5,    0xc,    0x5,    0xe,    0x5,  0x51, 0xb,
        0x5,  0x3,    0x6,    0x3,    0x6,    0x3,    0x6,    0x7,    0x6,    0x56, 0xa,  0x6,
        0xc,  0x6,    0xe,    0x6,    0x59,   0xb,    0x6,    0x3,    0x7,    0x3,  0x7,  0x3,
        0x7,  0x5,    0x7,    0x5e,   0xa,    0x7,    0x3,    0x7,    0x3,    0x7,  0x3,  0x7,
        0x2,  0x3,    0x6,    0x8,    0x2,    0x4,    0x6,    0x8,    0xa,    0xc,  0x2,  0x6,
        0x3,  0x2,    0x7,    0x8,    0x3,    0x2,    0x5,    0x6,    0x4,    0x2,  0xf,  0x10,
        0x13, 0x14,   0x3,    0x2,    0x11,   0x12,   0x2,    0x6a,   0x2,    0x11, 0x3,  0x2,
        0x2,  0x2,    0x4,    0x22,   0x3,    0x2,    0x2,    0x2,    0x6,    0x31, 0x3,  0x2,
        0x2,  0x2,    0x8,    0x4a,   0x3,    0x2,    0x2,    0x2,    0xa,    0x52, 0x3,  0x2,
        0x2,  0x2,    0xc,    0x5a,   0x3,    0x2,    0x2,    0x2,    0xe,    0x10, 0x5,  0x4,
        0x3,  0x2,    0xf,    0xe,    0x3,    0x2,    0x2,    0x2,    0x10,   0x13, 0x3,  0x2,
        0x2,  0x2,    0x11,   0xf,    0x3,    0x2,    0x2,    0x2,    0x11,   0x12, 0x3,  0x2,
        0x2,  0x2,    0x12,   0x14,   0x3,    0x2,    0x2,    0x2,    0x13,   0x11, 0x3,  0x2,
        0x2,  0x2,    0x14,   0x15,   0x7,    0x2,    0x2,    0x3,    0x15,   0x3,  0x3,  0x2,
        0x2,  0x2,    0x16,   0x17,   0x5,    0x6,    0x4,    0x2,    0x17,   0x18, 0x7,  0x9,
        0x2,  0x2,    0x18,   0x23,   0x3,    0x2,    0x2,    0x2,    0x19,   0x1a, 0x7,  0x4,
        0x2,  0x2,    0x1a,   0x1b,   0x5,    0xc,    0x7,    0x2,    0x1b,   0x1c, 0x7,  0x9,
        0x2,  0x2,    0x1c,   0x23,   0x3,    0x2,    0x2,    0x2,    0x1d,   0x1e, 0x7,  0x3,
        0x2,  0x2,    0x1e,   0x1f,   0x5,    0xc,    0x7,    0x2,    0x1f,   0x20, 0x5,  0x6,
        0x4,  0x2,    0x20,   0x21,   0x7,    0x9,    0x2,    0x2,    0x21,   0x23, 0x3,  0x2,
        0x2,  0x2,    0x22,   0x16,   0x3,    0x2,    0x2,    0x2,    0x22,   0x19, 0x3,  0x2,
        0x2,  0x2,    0x22,   0x1d,   0x3,    0x2,    0x2,    0x2,    0x23,   0x5,  0x3,  0x2,
        0x2,  0x2,    0x24,   0x25,   0x8,    0x4,    0x1,    0x2,    0x25,   0x32, 0x7,  0x16,
        0x2,  0x2,    0x26,   0x32,   0x7,    0x15,   0x2,    0x2,    0x27,   0x28, 0x7,  0x15,
        0x2,  0x2,    0x28,   0x2a,   0x7,    0xa,    0x2,    0x2,    0x29,   0x2b, 0x5,  0x8,
        0x5,  0x2,    0x2a,   0x29,   0x3,    0x2,    0x2,    0x2,    0x2a,   0x2b, 0x3,  0x2,
        0x2,  0x2,    0x2b,   0x2c,   0x3,    0x2,    0x2,    0x2,    0x2c,   0x32, 0x7,  0xb,
        0x2,  0x2,    0x2d,   0x2e,   0x7,    0xa,    0x2,    0x2,    0x2e,   0x2f, 0x5,  0x6,
        0x4,  0x2,    0x2f,   0x30,   0x7,    0xb,    0x2,    0x2,    0x30,   0x32, 0x3,  0x2,
        0x2,  0x2,    0x31,   0x24,   0x3,    0x2,    0x2,    0x2,    0x31,   0x26, 0x3,  0x2,
        0x2,  0x2,    0x31,   0x27,   0x3,    0x2,    0x2,    0x2,    0x31,   0x2d, 0x3,  0x2,
        0x2,  0x2,    0x32,   0x47,   0x3,    0x2,    0x2,    0x2,    0x33,   0x34, 0xc,  0x7,
        0x2,  0x2,    0x34,   0x35,   0x9,    0x2,    0x2,    0x2,    0x35,   0x46, 0x5,  0x6,
        0x4,  0x8,    0x36,   0x37,   0xc,    0x6,    0x2,    0x2,    0x37,   0x38, 0x9,  0x3,
        0x2,  0x2,    0x38,   0x46,   0x5,    0x6,    0x4,    0x7,    0x39,   0x3a, 0xc,  0x5,
        0x2,  0x2,    0x3a,   0x3b,   0x9,    0x4,    0x2,    0x2,    0x3b,   0x46, 0x5,  0x6,
        0x4,  0x6,    0x3c,   0x3d,   0xc,    0x4,    0x2,    0x2,    0x3d,   0x3e, 0x9,  0x5,
        0x2,  0x2,    0x3e,   0x46,   0x5,    0x6,    0x4,    0x5,    0x3f,   0x40, 0xc,  0x3,
        0x2,  0x2,    0x40,   0x41,   0x7,    0xd,    0x2,    0x2,    0x41,   0x42, 0x5,  0x6,
        0x4,  0x2,    0x42,   0x43,   0x7,    0xe,    0x2,    0x2,    0x43,   0x44, 0x5,  0x6,
        0x4,  0x3,    0x44,   0x46,   0x3,    0x2,    0x2,    0x2,    0x45,   0x33, 0x3,  0x2,
        0x2,  0x2,    0x45,   0x36,   0x3,    0x2,    0x2,    0x2,    0x45,   0x39, 0x3,  0x2,
        0x2,  0x2,    0x45,   0x3c,   0x3,    0x2,    0x2,    0x2,    0x45,   0x3f, 0x3,  0x2,
        0x2,  0x2,    0x46,   0x49,   0x3,    0x2,    0x2,    0x2,    0x47,   0x45, 0x3,  0x2,
        0x2,  0x2,    0x47,   0x48,   0x3,    0x2,    0x2,    0x2,    0x48,   0x7,  0x3,  0x2,
        0x2,  0x2,    0x49,   0x47,   0x3,    0x2,    0x2,    0x2,    0x4a,   0x4f, 0x5,  0x6,
        0x4,  0x2,    0x4b,   0x4c,   0x7,    0xc,    0x2,    0x2,    0x4c,   0x4e, 0x5,  0x6,
        0x4,  0x2,    0x4d,   0x4b,   0x3,    0x2,    0x2,    0x2,    0x4e,   0x51, 0x3,  0x2,
        0x2,  0x2,    0x4f,   0x4d,   0x3,    0x2,    0x2,    0x2,    0x4f,   0x50, 0x3,  0x2,
        0x2,  0x2,    0x50,   0x9,    0x3,    0x2,    0x2,    0x2,    0x51,   0x4f, 0x3,  0x2,
        0x2,  0x2,    0x52,   0x57,   0x7,    0x15,   0x2,    0x2,    0x53,   0x54, 0x7,  0xc,
        0x2,  0x2,    0x54,   0x56,   0x7,    0x15,   0x2,    0x2,    0x55,   0x53, 0x3,  0x2,
        0x2,  0x2,    0x56,   0x59,   0x3,    0x2,    0x2,    0x2,    0x57,   0x55, 0x3,  0x2,
        0x2,  0x2,    0x57,   0x58,   0x3,    0x2,    0x2,    0x2,    0x58,   0xb,  0x3,  0x2,
        0x2,  0x2,    0x59,   0x57,   0x3,    0x2,    0x2,    0x2,    0x5a,   0x5b, 0x7,  0x15,
        0x2,  0x2,    0x5b,   0x5d,   0x7,    0xa,    0x2,    0x2,    0x5c,   0x5e, 0x5,  0xa,
        0x6,  0x2,    0x5d,   0x5c,   0x3,    0x2,    0x2,    0x2,    0x5d,   0x5e, 0x3,  0x2,
        0x2,  0x2,    0x5e,   0x5f,   0x3,    0x2,    0x2,    0x2,    0x5f,   0x60, 0x7,  0xb,
        0x2,  0x2,    0x60,   0xd,    0x3,    0x2,    0x2,    0x2,    0xb,    0x11, 0x22, 0x2a,
        0x31, 0x45,   0x47,   0x4f,   0x57,   0x5d,
    };

    atn::ATNDeserializer deserializer;
    _atn = deserializer.deserialize(_serializedATN);

    size_t count = _atn.getNumberOfDecisions();
    _decisionToDFA.reserve(count);
    for (size_t i = 0; i < count; i++) {
        _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
    }
}

TParser::Initializer TParser::_init;
