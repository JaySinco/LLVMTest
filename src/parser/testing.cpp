#include "../utils.h"
#include "prec.h"
#include <glog/logging.h>

namespace qi = boost::spirit::qi;
// namespace phx = boost::phoenix;

struct ten_: qi::symbols<char, unsigned>
{
    ten_() { add("sds", 10)("sdc", 12); }
} ten;

template <typename Iterator>
struct roman: qi::grammar<Iterator, unsigned()>
{
};

int main(int argc, char** argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    std::string line = argv[1];
    std::vector<double> vs;
    auto grammar = qi::double_ % ',';
    auto beg = line.begin();
    auto end = line.end();
    bool ok = qi::phrase_parse(beg, end, grammar, qi::blank, vs);
    if (beg != end) {
        LOG(INFO) << "failed!";
        return 0;
    }
    LOG(INFO) << fmt::to_string(vs);
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
