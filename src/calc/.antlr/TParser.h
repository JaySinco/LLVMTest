
// Generated from d:\Jaysinco\Prototyping\src\calc\TParser.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"




class  TParser : public antlr4::Parser {
public:
  enum {
    Def = 1, Extern = 2, Plus = 3, Minus = 4, Star = 5, Div = 6, Semi = 7, 
    LeftParen = 8, RightParen = 9, Comma = 10, Question = 11, Colon = 12, 
    Less = 13, Greater = 14, Equal = 15, NotEqual = 16, LessEqual = 17, 
    GreaterEqual = 18, Identifier = 19, Number = 20, Whitespace = 21, Newline = 22
  };

  enum {
    RuleProgram = 0, RuleStatement = 1, RuleExpression = 2, RuleExpressionList = 3, 
    RuleArgumentList = 4, RuleFunctionSignature = 5
  };

  TParser(antlr4::TokenStream *input);
  ~TParser();

  virtual std::string getGrammarFileName() const override;
  virtual const antlr4::atn::ATN& getATN() const override { return _atn; };
  virtual const std::vector<std::string>& getTokenNames() const override { return _tokenNames; }; // deprecated: use vocabulary instead.
  virtual const std::vector<std::string>& getRuleNames() const override;
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;


  class ProgramContext;
  class StatementContext;
  class ExpressionContext;
  class ExpressionListContext;
  class ArgumentListContext;
  class FunctionSignatureContext; 

  class  ProgramContext : public antlr4::ParserRuleContext {
  public:
    ProgramContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);

   
  };

  ProgramContext* program();

  class  StatementContext : public antlr4::ParserRuleContext {
  public:
    StatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    StatementContext() = default;
    void copyFrom(StatementContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  FunctionDefinitionContext : public StatementContext {
  public:
    FunctionDefinitionContext(StatementContext *ctx);

    antlr4::tree::TerminalNode *Def();
    FunctionSignatureContext *functionSignature();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *Semi();
  };

  class  ExternalFunctionContext : public StatementContext {
  public:
    ExternalFunctionContext(StatementContext *ctx);

    antlr4::tree::TerminalNode *Extern();
    FunctionSignatureContext *functionSignature();
    antlr4::tree::TerminalNode *Semi();
  };

  class  ExpressionStatementContext : public StatementContext {
  public:
    ExpressionStatementContext(StatementContext *ctx);

    ExpressionContext *expression();
    antlr4::tree::TerminalNode *Semi();
  };

  StatementContext* statement();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ExpressionContext() = default;
    void copyFrom(ExpressionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  ParenthesesExpressionContext : public ExpressionContext {
  public:
    ParenthesesExpressionContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *LeftParen();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *RightParen();
  };

  class  IdExpressionContext : public ExpressionContext {
  public:
    IdExpressionContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *Identifier();
  };

  class  CallExpressionContext : public ExpressionContext {
  public:
    CallExpressionContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *Identifier();
    antlr4::tree::TerminalNode *LeftParen();
    antlr4::tree::TerminalNode *RightParen();
    ExpressionListContext *expressionList();
  };

  class  AdditiveExpressionContext : public ExpressionContext {
  public:
    AdditiveExpressionContext(ExpressionContext *ctx);

    antlr4::Token *op = nullptr;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *Plus();
    antlr4::tree::TerminalNode *Minus();
  };

  class  RelationalExpressionContext : public ExpressionContext {
  public:
    RelationalExpressionContext(ExpressionContext *ctx);

    antlr4::Token *op = nullptr;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *Less();
    antlr4::tree::TerminalNode *Greater();
    antlr4::tree::TerminalNode *LessEqual();
    antlr4::tree::TerminalNode *GreaterEqual();
  };

  class  ConditionalExpressionContext : public ExpressionContext {
  public:
    ConditionalExpressionContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *Question();
    antlr4::tree::TerminalNode *Colon();
  };

  class  EqualityExpressionContext : public ExpressionContext {
  public:
    EqualityExpressionContext(ExpressionContext *ctx);

    antlr4::Token *op = nullptr;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *Equal();
    antlr4::tree::TerminalNode *NotEqual();
  };

  class  MultiplicativeExpressionContext : public ExpressionContext {
  public:
    MultiplicativeExpressionContext(ExpressionContext *ctx);

    antlr4::Token *op = nullptr;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *Star();
    antlr4::tree::TerminalNode *Div();
  };

  class  LiteralExpressionContext : public ExpressionContext {
  public:
    LiteralExpressionContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *Number();
  };

  ExpressionContext* expression();
  ExpressionContext* expression(int precedence);
  class  ExpressionListContext : public antlr4::ParserRuleContext {
  public:
    ExpressionListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);

   
  };

  ExpressionListContext* expressionList();

  class  ArgumentListContext : public antlr4::ParserRuleContext {
  public:
    ArgumentListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> Identifier();
    antlr4::tree::TerminalNode* Identifier(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);

   
  };

  ArgumentListContext* argumentList();

  class  FunctionSignatureContext : public antlr4::ParserRuleContext {
  public:
    FunctionSignatureContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Identifier();
    antlr4::tree::TerminalNode *LeftParen();
    antlr4::tree::TerminalNode *RightParen();
    ArgumentListContext *argumentList();

   
  };

  FunctionSignatureContext* functionSignature();


  virtual bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;
  bool expressionSempred(ExpressionContext *_localctx, size_t predicateIndex);

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

