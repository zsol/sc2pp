#ifndef PARSERS_TIMESTAMP_FWD_HPP
#define PARSERS_TIMESTAMP_FWD_HPP

#include <sc2pp/detail/parsers_common.hpp>

namespace sc2pp { namespace parsers {
template <typename Iterator>
struct timestamp_grammar_t
        : public boost::spirit::qi::grammar<Iterator,
        boost::spirit::qi::locals<int, num_t>,
        num_t()>
{
    timestamp_grammar_t();
    boost::spirit::qi::rule<Iterator,
                            boost::spirit::qi::locals<int, num_t>,
                            num_t()> timestamp;

};

}}

#endif // PARSERS_TIMESTAMP_FWD_HPP
