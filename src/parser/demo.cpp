#include "prec.h"
#include <boost/variant.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>
#include <map>
#include <vector>
#include <iostream>
#include <iomanip>

using Source = boost::string_view;
using Location = Source::const_iterator;

namespace Completion
{
static int fuzzy_match(Source input, boost::string_view candidate, int rate = 1)
{  // start with first-letter boost
    int score = 0;

    while (!(input.empty() || candidate.empty())) {
        if (input.front() != candidate.front()) {
            return score + std::max(fuzzy_match(input.substr(1), candidate,
                                                std::max(rate - 2,
                                                         0)),  // penalty for ignoring an input char
                                    fuzzy_match(input, candidate.substr(1), std::max(rate - 1, 0)));
        }

        input.remove_prefix(1);
        candidate.remove_prefix(1);
        score += ++rate;
    }
    return score;
}

using Candidates = std::vector<std::string>;

class Hints
{
    struct ByLocation
    {
        template <typename T, typename U>
        bool operator()(T const& a, U const& b) const
        {
            return loc(a) < loc(b);
        }

    private:
        static Location loc(Source const& s) { return s.begin(); }
        static Location loc(Location const& l) { return l; }
    };

public:
    std::map<Location, std::string, ByLocation> incomplete;
    std::map<Source, Candidates, ByLocation> suggestions;

    /*explicit*/ operator bool() const { return incomplete.size() || suggestions.size(); }
};
}  // namespace Completion

namespace Ast
{
using NumLiteral = double;
using StringLiteral = std::string;  // de-escaped, not source view

struct Identifier: std::string
{
    using std::string::string;
    using std::string::operator=;
};

struct BinaryExpression;
struct CallExpression;

using Expression = boost::variant<NumLiteral, StringLiteral, Identifier,
                                  boost::recursive_wrapper<BinaryExpression>,
                                  boost::recursive_wrapper<CallExpression> >;

struct BinaryExpression
{
    Expression lhs;
    char op;
    Expression rhs;
};

using ArgList = std::vector<Expression>;

struct CallExpression
{
    Identifier function;
    ArgList args;
};
}  // namespace Ast

BOOST_FUSION_ADAPT_STRUCT(Ast::BinaryExpression, lhs, op, rhs)
BOOST_FUSION_ADAPT_STRUCT(Ast::CallExpression, function, args)

// for debug printing:
namespace
{
struct once_t
{  // an auto-reset flag
    operator bool()
    {
        bool v = flag;
        flag = false;
        return v;
    }
    bool flag = true;
};
}  // namespace

// for debug printing:
namespace Ast
{

static inline std::ostream& operator<<(std::ostream& os, std::vector<Expression> const& args)
{
    os << "(";
    once_t first;
    for (auto& a: args) os << (first ? "" : ", ") << a;
    return os << ")";
}

static inline std::ostream& operator<<(std::ostream& os, BinaryExpression const& e)
{
    return os << boost::fusion::as_vector(e);
}
static inline std::ostream& operator<<(std::ostream& os, CallExpression const& e)
{
    return os << boost::fusion::as_vector(e);
}
}  // namespace Ast

namespace Parsing
{
namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;

template <typename It>
struct Expression: qi::grammar<It, Ast::Expression()>
{
    Expression(Completion::Hints& hints): Expression::base_type(start), _hints(hints)
    {
        using namespace qi;

        start = skip(space)[expression];

        expression =
            term[_val = _1] >> *(char_("-+") >> expression)[_val = make_binary(_val, _1, _2)];
        term = factor[_val = _1] >> *(char_("*/") >> term)[_val = make_binary(_val, _1, _2)];
        factor = simple[_val = _1] >> *(char_("^") >> factor)[_val = make_binary(_val, _1, _2)];

        simple = call | variable | compound | number | string;

        auto implied = [=](char ch) { return copy(omit[lit(ch) | raw[eps][imply(_1, ch)]]); };

        variable = maybe_known(phx::ref(_variables));

        compound %= '(' >> expression >> implied(')');

        // The heuristics:
        // - an unknown identifier followed by (
        // - an unclosed argument list implies )
        call %= (known(phx::ref(_functions))  // known -> imply the parens
                 | &(identifier >> '(') >> unknown(phx::ref(_functions))) >>
                implied('(') >> -(expression % (',' | !(')' | eoi) >> implied(','))) >>
                implied(')');

        // lexemes, primitive rules
        identifier = raw[(alpha | '_') >> *(alnum | '_')];

        // imply the closing quotes
        string %= '"' >> *('\\' >> char_ | ~char_('"')) >> implied('"');  // TODO more escapes

        number = double_;  // TODO integral arguments

        ///////////////////////////////
        // identifier loopkup, suggesting
        {
            maybe_known = known(_domain) | unknown(_domain);

            // distinct to avoid partially-matching identifiers
            using boost::spirit::repository::qi::distinct;
            auto kw = distinct(copy(alnum | '_'));

            known = raw[kw[lazy(_domain)]];
            unknown = raw[identifier[_val = _1]][suggest_for(_1, _domain)];
        }

        BOOST_SPIRIT_DEBUG_NODES((
            start)(expression)(term)(factor)(simple)(compound)(call)(variable)(identifier)(number)(string)
                                 //(maybe_known)(known)(unknown) // qi::symbols<> non-streamable
        )

        _variables += "foo", "bar", "qux";
        _functions += "print", "sin", "tan", "sqrt", "frobnicate";
    }

private:
    Completion::Hints& _hints;

    using Domain = qi::symbols<char>;
    Domain _variables, _functions;

    qi::rule<It, Ast::Expression()> start;
    qi::rule<It, Ast::Expression(), qi::space_type> expression, term, factor, simple;
    // completables
    qi::rule<It, Ast::Expression(), qi::space_type> compound;
    qi::rule<It, Ast::CallExpression(), qi::space_type> call;

    // implicit lexemes
    qi::rule<It, Ast::Identifier()> variable, identifier;
    qi::rule<It, Ast::NumLiteral()> number;
    qi::rule<It, Ast::StringLiteral()> string;

    // domain identifier lookups
    qi::_r1_type _domain;
    qi::rule<It, Ast::Identifier(Domain const&)> maybe_known, known, unknown;

    ///////////////////////////////
    // binary expression factory
    struct make_binary_f
    {
        Ast::BinaryExpression operator()(Ast::Expression const& lhs, char op,
                                         Ast::Expression const& rhs) const
        {
            return {lhs, op, rhs};
        }
    };
    boost::phoenix::function<make_binary_f> make_binary;

    ///////////////////////////////
    // auto-completing incomplete expressions
    struct imply_f
    {
        Completion::Hints& _hints;
        void operator()(boost::iterator_range<It> where, char implied_char) const
        {
            auto inserted =
                _hints.incomplete.emplace(&*where.begin(), std::string(1, implied_char));
            // add the implied char to existing completion
            if (!inserted.second) inserted.first->second += implied_char;
        }
    };
    boost::phoenix::function<imply_f> imply{imply_f{_hints}};

    ///////////////////////////////
    // suggest_for
    struct suggester
    {
        Completion::Hints& _hints;

        void operator()(boost::iterator_range<It> where, Domain const& symbols) const
        {
            using namespace Completion;
            Source id(&*where.begin(), where.size());
            Candidates c;

            symbols.for_each([&](std::string const& k, ...) { c.push_back(k); });

            auto score = [id](Source v) { return fuzzy_match(id, v); };
            auto byscore = [=](Source a, Source b) { return score(a) > score(b); };

            sort(c.begin(), c.end(), byscore);
            c.erase(remove_if(c.begin(), c.end(), [=](Source s) { return score(s) < 3; }), c.end());

            _hints.suggestions.emplace(id, c);
        }
    };
    boost::phoenix::function<suggester> suggest_for{suggester{_hints}};
};
}  // namespace Parsing

int main()
{
    for (Source const input: {
             "",                          // invalid
             "(3",                        // incomplete, imply ')'
             "3*(6+sqrt(9))^7 - 1e8",     // completely valid
             "(3*(((6+sqrt(9))^7 - 1e8",  // incomplete, imply ")))"
             "print(\"hello \\\"world!",  // completes the string literal and the parameter list
             "foo",                       // okay, known variable
             "baz",                       // (suggest bar)
             "baz(",                      // incomplete, imply ')', unknown function
             "taz(",                      // incomplete, imply ')', unknown function
             "san(",                      // 2 suggestions (sin/tan)
             "print(1, 2, \"three\", complicated(san(78",
             "(print sqrt sin 9)    -0) \"bye",
         }) {
        std::cout << "-------------- '" << input << "'\n";
        Location f = input.begin(), l = input.end();

        Ast::Expression expr;
        Completion::Hints hints;
        bool ok = parse(f, l, Parsing::Expression<Location>{hints}, expr);

        if (hints) {
            std::cout << "Input: '" << input << "'\n";
        }
        for (auto& c: hints.incomplete) {
            std::cout << "Missing " << std::setw(c.first - input.begin()) << ""
                      << "^ implied: '" << c.second << "'\n";
        }
        for (auto& id: hints.suggestions) {
            std::cout << "Unknown " << std::setw(id.first.begin() - input.begin()) << ""
                      << std::string(id.first.size(), '^');
            if (id.second.empty())
                std::cout << " (no suggestions)\n";
            else {
                std::cout << " (did you mean ";
                once_t first;
                for (auto& s: id.second) std::cout << (first ? "" : " or ") << "'" << s << "'";
                std::cout << "?)\n";
            }
        }

        if (ok) {
            std::cout << "AST:    " << expr << "\n";
        } else {
            std::cout << "Parse failed\n";
        }

        if (f != l) std::cout << "Remaining input: '" << std::string(f, l) << "'\n";
    }
}
