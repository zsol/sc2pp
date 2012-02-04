#ifndef SC2PP_DETAIL_PARSERS_TIMESTAMP_HPP
#define SC2PP_DETAIL_PARSERS_TIMESTAMP_HPP

#include <sc2pp/detail/parsers_common.hpp>

namespace sc2pp { namespace parsers {
template <typename Iterator>
struct timestamp_grammar_t
    : public boost::spirit::qi::grammar<Iterator,
                                        boost::spirit::qi::locals<int, num_t>,
                                        num_t()>
{
    timestamp_grammar_t() : timestamp_grammar_t::base_type(timestamp, "Timestamp")
    {
        USE_SPIRIT_PARSER_(byte_);
        USE_SPIRIT_PARSER(repeat);
        USE_SPIRIT_PARSER(eps);
        USE_SPIRIT_PARSER(_val);
        USE_SPIRIT_PARSER(_1);
        USE_SPIRIT_PARSER(_a);
        USE_SPIRIT_PARSER(_b);
        using boost::phoenix::static_cast_;
        bits_type bits;

        timestamp =
                bits(2)[_b = _1] >> bits(6)[_a = _1]
                >> repeat(_b)[eps[_a <<= 8] >> byte_[_a += _1]]
                >> eps[_val = _a];

        HANDLE_ERROR(timestamp);
    }

    boost::spirit::qi::rule<Iterator,
                            boost::spirit::qi::locals<int, num_t>,
                            num_t()> timestamp;

};
}}

#endif // SC2PP_DETAIL_PARSERS_TIMESTAMP_HPP
