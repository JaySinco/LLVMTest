
// Generated from d:\Jaysinco\Prototyping\src\calc\TLexer.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"




class  TLexer : public antlr4::Lexer {
public:
  enum {
    Def = 1, Extern = 2, Plus = 3, Minus = 4, Star = 5, Div = 6, Semi = 7, 
    LeftParen = 8, RightParen = 9, Comma = 10, Question = 11, Colon = 12, 
    Less = 13, Greater = 14, Equal = 15, NotEqual = 16, LessEqual = 17, 
    GreaterEqual = 18, Identifier = 19, Number = 20, Whitespace = 21, Newline = 22
  };

  TLexer(antlr4::CharStream *input);
  ~TLexer();

  virtual std::string getGrammarFileName() const override;
  virtual const std::vector<std::string>& getRuleNames() const override;

  virtual const std::vector<std::string>& getChannelNames() const override;
  virtual const std::vector<std::string>& getModeNames() const override;
  virtual const std::vector<std::string>& getTokenNames() const override; // deprecated, use vocabulary instead
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;

  virtual const std::vector<uint16_t> getSerializedATN() const override;
  virtual const antlr4::atn::ATN& getATN() const override;

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;
  static std::vector<std::string> _channelNames;
  static std::vector<std::string> _modeNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

