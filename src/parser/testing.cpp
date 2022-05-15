#include "../utils.h"
#include <glog/logging.h>
#include <boost/spirit/include/qi.hpp>

namespace qi = boost::spirit::qi;
namespace enc = qi::standard_wide;
using CsvColumn = std::wstring;
using CsvLine = std::vector<CsvColumn>;

struct CsvDoubleQuote_: qi::symbols<wchar_t, wchar_t>
{
    CsvDoubleQuote_() { add(L"\"\"", L'"'); }

} CsvDoubleQuote;

template <typename It>
struct CsvGrammar: qi::grammar<It, CsvLine(), qi::blank_type>
{
    CsvGrammar(const std::wstring& colSep): CsvGrammar::base_type(line)
    {
        line = column % qi::no_skip[colSep];
        column = quoted | qi::no_skip[*(enc::char_ - colSep)];
        quoted = qi::no_skip[L'"' >> *(CsvDoubleQuote | ~enc::char_(L'"')) >> L'"'];
    }

private:
    qi::rule<It, CsvLine(), qi::blank_type> line;
    qi::rule<It, CsvColumn(), qi::blank_type> column;
    qi::rule<It, std::wstring(), qi::blank_type> quoted;
};

int main(int argc, char** argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    std::vector<std::wstring> cols;
    std::wstring line = L"a,测试,c";
    CsvGrammar<std::wstring::const_iterator> parser(L",");
    bool ok = qi::phrase_parse(line.begin(), line.end(), parser, qi::blank, cols);
    for (auto& c: cols) {
        LOG(INFO) << utils::ws2s(c);
    }
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
