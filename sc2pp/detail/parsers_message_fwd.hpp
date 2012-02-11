#ifndef PARSERS_MESSAGE_FWD_HPP
#define PARSERS_MESSAGE_FWD_HPP

#include <sc2pp/detail/parsers_common.hpp>
#include <sc2pp/detail/parsers_timestamp_fwd.hpp>

namespace sc2pp { namespace parsers {

namespace qi = boost::spirit::qi;

template <typename Iterator>
struct message_grammar_t
    : public qi::grammar<Iterator,
        boost::spirit::locals<num_t, int>,
        message_event_ptr()>
{
    message_grammar_t();
    timestamp_grammar_t<Iterator> timestamp;

    qi::rule<Iterator, qi::locals<num_t, int>, message_event_ptr()> message_event;
    qi::rule<Iterator, qi::locals<int, int>, message_event_ptr(num_t, int)> ping_event;
    qi::rule<Iterator, qi::locals<int, message_t::target_t>,
            message_event_ptr(num_t, int)> message;
    qi::rule<Iterator, message_event_ptr(num_t, int)> unknown_message;

    boost::phoenix::function<vector_to_array_impl> vector_to_array;

};

}}
#endif // PARSERS_MESSAGE_FWD_HPP
