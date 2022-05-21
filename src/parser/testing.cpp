#include "../utils.h"
#include "prec.h"
#include <glog/logging.h>
#include <fmt/ranges.h>

namespace qi = boost::spirit::qi;
namespace enc = boost::spirit::standard_wide;
// namespace phx = boost::phoenix;

template <typename Iterator, typename Attr>
struct grammar: qi::grammar<Iterator, Attr()>
{
    grammar(): grammar::base_type(start)
    {
        auto g = *qi::double_;
        start = qi::skip(enc::blank)[g];
    }

    qi::rule<Iterator, Attr()> start;
};

int main(int argc, char** argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    auto raw = utils::readFile((__DIRNAME__ / "input.txt").wstring());
    std::wstring input = utils::s2ws(*raw, true);
    auto beg = input.begin();
    auto end = input.end();

    std::vector<double> attr;
    grammar<decltype(beg), decltype(attr)> g;
    bool ok = qi::parse(beg, end, g, attr);
    if (beg != end) {
        LOG(ERROR) << "left => " << utils::ws2s({&*beg, size_t(end - beg)});
    }
    LOG(INFO) << ok << " " << fmt::to_string(attr);
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
