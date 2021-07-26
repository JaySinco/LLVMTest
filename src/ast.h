#pragma once
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/io.hpp>

namespace x3 = boost::spirit::x3;

namespace ast
{
struct err_t
{
    std::string message;
};

enum class op_t
{
    EQUAL,
    NOT_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
};

struct expr_value;

using comp_value = std::pair<op_t, std::string>;

struct match_value
{
    std::vector<std::string> select_;
    boost::optional<comp_value> compare_;
};

using unit_value = boost::variant<match_value, x3::forward_ast<expr_value>>;
using not_value = std::pair<boost::optional<char>, unit_value>;
using and_value = std::vector<not_value>;
using or_value = std::vector<and_value>;

struct expr_value: or_value
{
    using or_value::operator=;
};

}  // namespace ast

BOOST_FUSION_ADAPT_STRUCT(ast::match_value, select_, compare_);
