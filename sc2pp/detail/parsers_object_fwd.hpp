#ifndef PARSERS_OBJECT_FWD_HPP
#define PARSERS_OBJECT_FWD_HPP

#include <sc2pp/detail/parsers_common.hpp>

namespace sc2pp { namespace parsers {

namespace qi = boost::spirit::qi;

template <typename Iterator>
struct object_grammar_t : public qi::grammar<Iterator, object_type()>
{
    object_grammar_t();
    qi::rule<Iterator, qi::locals<int>, std::string() > byte_string;
    qi::rule<Iterator, int()> single_byte_integer;
    qi::rule<Iterator, int()> single_byte_integer_; // an integer without 'type' byte
    qi::rule<Iterator, num_t()> four_byte_integer;
    qi::rule<Iterator, qi::locals<hugenum_t, int>, hugenum_t()> variable_length_integer;
    qi::rule<Iterator, qi::locals<int>, byte_array()> array;
    qi::rule<Iterator, qi::locals<int, int>, byte_map()> map;
    qi::rule<Iterator, object_type()> object;

    boost::phoenix::function<apply_sign_impl> apply_sign;
    boost::phoenix::function<apply_huge_sign_impl> apply_huge_sign;

};

}}
#endif // PARSERS_OBJECT_FWD_HPP
