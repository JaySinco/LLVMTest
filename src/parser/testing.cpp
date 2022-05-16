#include "../utils.h"
#include "prec.h"
#include <glog/logging.h>

namespace qi = boost::spirit::qi;
namespace enc = qi::standard_wide;
namespace phx = boost::phoenix;
using phx::placeholders::arg1;

int main(int argc, char** argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    std::string line = "{12 13 14}";
    std::vector<int> q;
    auto g = *qi::int_;
    auto g1 = g[([](auto&& q) { std::cout << q.back() << std::endl; })];
    bool ok =
        qi::phrase_parse(line.begin(), line.end(), '{' >> g1 >> '}' >> qi::int_, qi::blank, q);
}

// lexer grammar TLexer;

// options { language = Cpp; }

// Def: 'def';
// Extern: 'extern';
// Plus: '+';
// Minus: '-';
// Star: '*';
// Div: '/';
// Semi: ';';
// LeftParen: '(';
// RightParen: ')';
// Comma: ',';
// Question: '?';
// Colon: ':';
// Less: '<';
// Greater: '>';
// Equal: '==';
// NotEqual: '!=';
// LessEqual: '<=';
// GreaterEqual: '>=';
// Identifier: NONDIGIT(NONDIGIT | DIGIT) *;
// Number: DIGIT + ('.' DIGIT +) ? ;
// Whitespace: [ \t] +->skip;
// Newline:      ( '\r' '\n'? | '\n') -> skip;

// fragment NONDIGIT: [a - zA - Z_];
// fragment DIGIT: [0 - 9];
//
//
//
//
//
//
//
//
//
//
//
//
//
// parser grammar TParser;

// options {
//     tokenVocab=TLexer;
//     language=Cpp;
// }

// program: statement* EOF;

// statement:
//     expression ';'                           # expressionStatement
//     | 'extern' functionSignature ';'         # externalFunction
//     | 'def' functionSignature expression ';' # functionDefinition
//     ;

// expression:
//     Number                                                   # literalExpression
//     | Identifier                                             # idExpression
//     | Identifier '(' expressionList? ')'                     # callExpression
//     | '(' expression ')'                                     # parenthesesExpression
//     | expression op=('*' | '/') expression                   # multiplicativeExpression
//     | expression op=('+' | '-') expression                   # additiveExpression
//     | expression op=('<' | '>' | '<=' | '>=') expression     # relationalExpression
//     | expression op=('==' | '!=') expression                 # equalityExpression
//     | <assoc=right> expression '?' expression ':' expression # conditionalExpression
//     ;

// expressionList: expression (',' expression)*;

// argumentList: Identifier (',' Identifier)*;

// functionSignature: Identifier '(' argumentList? ')';
