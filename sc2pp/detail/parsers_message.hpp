#ifndef SC2PP_DETAIL_PARSERS_MESSAGE_HPP
#define SC2PP_DETAIL_PARSERS_MESSAGE_HPP

#include <sc2pp/detail/parsers_common.hpp>
#include <sc2pp/detail/parsers_timestamp.hpp>

namespace sc2pp { namespace parsers {

template <typename Iterator>
struct message_grammar_t
    : public boost::spirit::qi::grammar<Iterator,
                                        boost::spirit::locals<num_t, int>,
                                        message_event_ptr()>
{
    message_grammar_t() : message_grammar_t::base_type(message_event, "Message")
    {
        USE_SPIRIT_PARSER_(byte_);
        USE_SPIRIT_PARSER(little_dword);
        USE_SPIRIT_PARSER(repeat);
        USE_SPIRIT_PARSER(eps);
        USE_SPIRIT_PARSER(_val);
        USE_SPIRIT_PARSER(omit);
        USE_SPIRIT_PARSER(_1);
        USE_SPIRIT_PARSER(_a);
        USE_SPIRIT_PARSER(_b);
        USE_SPIRIT_PARSER(_r1);
        USE_SPIRIT_PARSER(_r2);
        USE_SPIRIT_PARSER(_pass);
        USE_SPIRIT_PARSER(as_string);
        using boost::phoenix::static_cast_;
        using boost::phoenix::if_;
        using boost::phoenix::bind;


        ping_event =
            byte_(0x83) > little_dword[_a = _1] > little_dword[_b = _1]
            >> eps[_val = p::bind(ping_event_t::make, _r1, _r2, _a, _b)];

        message =
                &byte_[if_((_1 & 0x80) != 0)[_pass = false]]
                > byte_[_b = static_cast_<message_t::target_t>(_1 & 0x3), _a = (_1 & 0x18) << 3] >> byte_[_a += _1]
                >> as_string[repeat(_a)[byte_]][_val = p::bind(message_t::make, _r1, _r2, _b, _1)];

        unknown_message =
            byte_(0x80) > repeat(4)[byte_][_val = p::bind(unknown_message_t::make, _r1, _r2, vector_to_array(_1))];

        message_event %= omit[timestamp[_a = _1] >> byte_[_b = _1 & 0xF]]
            >> (ping_event(_a, _b) | message(_a, _b) | unknown_message(_a, _b));


        HANDLE_ERROR(message_event);
        HANDLE_ERROR(ping_event);
        HANDLE_ERROR(message);
        HANDLE_ERROR(unknown_message);
        // boost::spirit::qi::on_error<boost::spirit::qi::fail>(*this, errorhandler<boost::spirit::unused_type, Iterator>);
    }

    timestamp_grammar_t<Iterator> timestamp;

    boost::spirit::qi::rule<Iterator,
                            boost::spirit::qi::locals<num_t, int>,
                            message_event_ptr()> message_event;
    boost::spirit::qi::rule<Iterator,
                            boost::spirit::qi::locals<int, int>,
                            message_event_ptr(num_t, int)> ping_event;
    boost::spirit::qi::rule<Iterator,
                            boost::spirit::qi::locals<int, message_t::target_t>,
                            message_event_ptr(num_t, int)> message;
    boost::spirit::qi::rule<Iterator,
                            message_event_ptr(num_t, int)> unknown_message;

    boost::phoenix::function<vector_to_array_impl> vector_to_array;
};

}}

#endif // SC2PP_DETAIL_PARSERS_MESSAGE_HPP
