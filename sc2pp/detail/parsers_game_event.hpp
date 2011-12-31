#ifndef SC2PP_DETAIL_PARSERS_GAME_EVENT_HPP
#define SC2PP_DETAIL_PARSERS_GAME_EVENT_HPP

#include <sc2pp/detail/parsers_common.hpp>
#include <sc2pp/detail/parsers_timestamp.hpp>

namespace sc2pp { namespace parsers {
template <typename Iterator>
struct game_event_grammar_t
    : public boost::spirit::qi::grammar<Iterator,
                                        boost::spirit::qi::locals<num_t, int>,
                                        game_event_ptr()>
{
    game_event_grammar_t() : game_event_grammar_t::base_type(game_event, "Game Event")
    {
        using boost::spirit::byte_;
        using boost::spirit::big_dword;
        using boost::spirit::repeat;
        using boost::spirit::inf;
        using boost::spirit::eps;
        using boost::spirit::_val;
        using boost::spirit::omit;
        using boost::spirit::_1;
        using boost::spirit::_a;
        using boost::spirit::_b;
        using boost::spirit::_r1;
        using boost::spirit::_r2;
        using boost::spirit::_pass;
        using boost::spirit::as_string;
        using boost::phoenix::static_cast_;
        using boost::phoenix::if_;
        using boost::phoenix::bind;


        game_event %=
                omit[timestamp[_a = _1] >> &byte_[_b = (static_cast_<int>(_1) >> 3) & 0x1f]] >
                (
                    initial_event(_a, _b) |
                    action_event(_a, _b) |
                    unknown_event(_a, _b)
                );

        unknown_event =
            byte_[_a = _1 & 0x7]
            >> byte_[_val = bind(unknown_event_t::make, _r1, _r2, _a, _1)];

        initial_event %=
                omit[byte_[if_((_1 & 0x7) != 0)[_pass = false]]] >
                (
                    player_joined_event(_r1, _r2) |
                    game_started_event(_r1, _r2)
                );

        player_joined_event =
            (byte_(0xB) | byte_(0xC) | byte_(0x2C))
            >> eps[_val = bind(player_joined_event_t::make, _r1, _r2)];

        game_started_event =
            byte_(0x5)
            >> eps[_val = bind(game_started_event_t::make, _r1, _r2)];

        action_event %=
                omit[byte_[if_((_1 & 0x7) != 1)[_pass = false]]] >
                (
                    player_left_event(_r1, _r2) |
                    resource_transfer_event(_r1, _r2)
                );

        player_left_event =
            byte_(0x9)
            >> eps[_val = bind(player_left_event_t::make, _r1, _r2)];

        resource_transfer_event =
            &byte_[if_((_1 & 0xf) != 0xf)[_pass = false]] >> byte_[_a = static_cast_<int>(_1) >> 4] >>
            byte_(0x84) >>
            repeat(4)[resource][_val = bind(resource_transfer_event_t::make, _r1, _r2, _a, _1)];

        resource =
            big_dword[_val = (static_cast_<num_t>(_1) >> 8) * (static_cast_<num_t>(_1) & 0xf0) + (static_cast_<num_t>(_1) & 0x0f)];


        // camera_event %=
        //     omit[byte_[if_((_1 & 0x7) != 3)[_pass = false]]];


        HANDLE_ERROR(game_event);
        HANDLE_ERROR(unknown_event);
        HANDLE_ERROR(player_joined_event);
        HANDLE_ERROR(game_started_event);
        HANDLE_ERROR(initial_event);
        HANDLE_ERROR(action_event);
        HANDLE_ERROR(player_left_event);
        HANDLE_ERROR(resource_transfer_event);
        // boost::spirit::qi::on_error<boost::spirit::qi::fail>(*this, errorhandler<boost::spirit::unused_type, Iterator>);

    }

    timestamp_grammar_t<Iterator> timestamp;

    boost::spirit::qi::rule<Iterator,
                            boost::spirit::qi::locals<num_t, int>,
                            game_event_ptr()> game_event;
    boost::spirit::qi::rule<Iterator,
                            boost::spirit::qi::locals<int>,
                            game_event_ptr(num_t, int)> unknown_event;
    boost::spirit::qi::rule<Iterator,
                            game_event_ptr(num_t, int)> initial_event;
    boost::spirit::qi::rule<Iterator,
                            game_event_ptr(num_t, int)> player_joined_event;
    boost::spirit::qi::rule<Iterator,
                            game_event_ptr(num_t, int)> game_started_event;
    boost::spirit::qi::rule<Iterator,
                            game_event_ptr(num_t, int)> action_event;
    boost::spirit::qi::rule<Iterator,
                            game_event_ptr(num_t, int)> player_left_event;
    boost::spirit::qi::rule<Iterator,
                            boost::spirit::qi::locals<int, std::vector<num_t> >,
                            game_event_ptr(num_t, int)> resource_transfer_event;
    boost::spirit::qi::rule<Iterator,
                            num_t() > resource;

};
}}

#endif // SC2PP_DETAIL_PARSERS_GAME_EVENT_HPP
