#ifndef PARSERS_ABILITY_EVENT_HPP
#define PARSERS_ABILITY_EVENT_HPP

#include <sc2pp/detail/parsers_common.hpp>

namespace sc2pp { namespace parsers {
template <typename Iterator>
struct ability_event_grammar_t
        : boost::spirit::qi::grammar<Iterator,
                                     game_event_ptr(num_t, int)>
{
    ability_event_grammar_t() : ability_event_grammar_t::base_type(ability_event, "Ability Event")
    {
        using boost::spirit::byte_;
        using boost::spirit::big_dword;
        using boost::spirit::big_word;
        using boost::spirit::little_word;
        using boost::spirit::word;
        using boost::spirit::repeat;
        using boost::spirit::inf;
        using boost::spirit::eps;
        using boost::spirit::_val;
        using boost::spirit::omit;
        using boost::spirit::_1;
        using boost::spirit::_2;
        using boost::spirit::_a;
        using boost::spirit::_b;
        using boost::spirit::_c;
        using boost::spirit::_r1;
        using boost::spirit::_r2;
        using boost::spirit::_r3;
        using boost::spirit::_pass;
        using boost::spirit::as_string;
        using boost::phoenix::static_cast_;
        using boost::phoenix::if_;
        using boost::phoenix::bind;
        using boost::phoenix::construct;

        ability_event %=
                omit[bits(4, 0xb) > bits(4)] >
                (
                    cancel(_r1, _r2) | command_card(_r1, _r2) |
                    no_idea(_r1, _r2) | targeted(_r1, _r2) | move(_r1, _r2)
                );

        command_card =
                byte_ >> bits(5) >> bits(1, 0x1) > bits(2) >
                (little_word >> bits(4))[_a = (static_cast_<int>(_1) << 4) | _2] >
                ((bits(2, 0x1) > command_card_location(_r1, _r2, _a)) |
                 (bits(2, 0x2) > command_card_target(_r1, _r2, _a)) |
                 eps[_val = construct<ability_event_ptr>()]);

        command_card_location =
                coordinate[_a = _1] > coordinate[_b = _1] > repeat(4)[byte_] >
                eps[_val = construct<ability_event_ptr>()];

        command_card_target =
                word > big_dword[_a = _1] > big_word[_b = _1] > repeat(10)[byte_] /*coords?*/ >
                eps[_val = construct<ability_event_ptr>()];

        cancel =
                (byte_(0x29) | byte_(0x19) | byte_(0x14)) >> bits(5) >> bits(1, 0x1) >> bits(2) >
                big_tbyte[_a = _1] > big_dword[_b = _1] >
                eps[_val = construct<ability_event_ptr>()];

        no_idea =
                bits(2) >> bits(1, 0x1) >> bits(5, 0x0) >> bits(6) >> bits(1, 0x1) >> bits(1) >
                big_tbyte[_a = _1] >
                (
                    (eps[_pass = _a & 0x20] > repeat(9)[byte_]) |
                    (eps[_pass = _a & 0x40] > repeat(18)[byte_]) |
                    eps
                ) > eps[_val = construct<ability_event_ptr>()];

        targeted =
                byte_ >> bits(7) >> bits(1, 0x1) >
                little_word[_a = _1] > big_dword[_b = _1] > big_word[_c = _1] > repeat(10)[byte_] /*coords?*/ >
                eps[_val = construct<ability_event_ptr>()];

        move =
                byte_(0x8) >> bits(6) >> bits(1, 0x1) >> bits(1) >
                coordinate[_a = _1] > coordinate[_b = _1] > repeat(5)[byte_] >
                eps[_val = construct<ability_event_ptr>()];

        HANDLE_ERROR(ability_event);
        HANDLE_ERROR(command_card);
        HANDLE_ERROR(command_card_location);
        HANDLE_ERROR(command_card_target);
        HANDLE_ERROR(cancel);
        HANDLE_ERROR(targeted);
        HANDLE_ERROR(move);
        HANDLE_ERROR(no_idea);

    }

    big_tbyte_grammar_t<Iterator> big_tbyte;
    coordinate_grammar_t<Iterator> coordinate;

    boost::spirit::qi::rule<Iterator,
            game_event_ptr(num_t, int)> ability_event;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<int>,
            game_event_ptr(num_t, int)> command_card;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<float, float>,
            game_event_ptr(num_t, int, int)> command_card_location;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<num_t, num_t>,
            game_event_ptr(num_t, int, int)> command_card_target;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<num_t, num_t>,
            game_event_ptr(num_t, int)> cancel;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<num_t>,
            game_event_ptr(num_t, int)> no_idea;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<int, num_t, int>,
            game_event_ptr(num_t, int)> targeted;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<float, float>,
            game_event_ptr(num_t, int)> move;
};

}}

#endif // PARSERS_ABILITY_EVENT_HPP
