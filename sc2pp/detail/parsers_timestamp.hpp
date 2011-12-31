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
        using boost::spirit::byte_;
        using boost::spirit::repeat;
        using boost::spirit::eps;
        using boost::spirit::_val;
        using boost::spirit::omit;
        using boost::spirit::_1;
        using boost::spirit::_a;
        using boost::spirit::_b;
        using boost::phoenix::static_cast_;


        timestamp =
            &byte_[_b = _1 & 0x3] >> byte_[_a = static_cast_<int>(_1) >> 2]
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
