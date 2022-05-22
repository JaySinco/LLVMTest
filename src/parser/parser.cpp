#include "error-handler.h"
#include <fmt/ranges.h>

namespace qi = boost::spirit::qi;
namespace enc = boost::spirit::standard_wide;
namespace phx = boost::phoenix;

namespace parser
{

struct employee
{
    int age;
    std::wstring surname;
    std::wstring forename;
    double salary;
};

}  // namespace parser

BOOST_FUSION_ADAPT_STRUCT(parser::employee,
                          (int, age)(std::wstring, surname)(std::wstring, forename)(double, salary))

namespace parser
{

template <typename Iterator>
struct grammar: qi::grammar<Iterator, employee()>
{
    grammar(const std::filesystem::path& source_file)
        : grammar::base_type(start), err_handler(source_file)
    {
        quoted = qi::lexeme['"' > *(enc::char_ - '"') > '"'];
        epl = qi::lit(L"employee") > '{' > qi::int_ > ',' > quoted > ',' > quoted > ',' >
              qi::double_ > '}';
        start = qi::skip(enc::space)[epl > qi::eoi];

        qi::on_error<qi::fail>(start, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
    }

    qi::rule<Iterator, std::wstring()> quoted;
    qi::rule<Iterator, employee()> epl;
    qi::rule<Iterator, employee()> start;

    phx::function<error_handler<Iterator>> err_handler;
};

}  // namespace parser

namespace parser
{

using iterator = boost::spirit::line_pos_iterator<std::wstring::const_iterator>;

void parse(const std::filesystem::path& source_file)
{
    auto raw = utils::readFile(source_file.wstring());
    std::wstring input = utils::s2ws(*raw, true);
    iterator beg(input.begin());
    iterator end(input.end());
    grammar<iterator> g(source_file);
    employee attr;
    bool ok = qi::parse(beg, end, g, attr);
    LOG(INFO) << ok << " " << utils::ws2s(attr.surname);
}

}  // namespace parser

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
