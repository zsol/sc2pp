#ifndef PARSERS_ABILITY_EVENT_HPP
#define PARSERS_ABILITY_EVENT_HPP

#include <sc2pp/detail/parsers_ability_event_fwd.hpp>

namespace sc2pp { namespace parsers {
template <typename Iterator>
ability_event_grammar_t<Iterator>::ability_event_grammar_t()
    : ability_event_grammar_t::base_type(ability_event, "Ability Event")
{
    USE_SPIRIT_PARSER_(byte_);
    USE_SPIRIT_PARSER(big_dword);
    USE_SPIRIT_PARSER(big_word);
    USE_SPIRIT_PARSER(little_word);
    USE_SPIRIT_PARSER(word);
    USE_SPIRIT_PARSER(repeat);
    USE_SPIRIT_PARSER(eps);
    USE_SPIRIT_PARSER(_val);
    USE_SPIRIT_PARSER(omit);
    USE_SPIRIT_PARSER(_1);
    USE_SPIRIT_PARSER(_2);
    USE_SPIRIT_PARSER(_a);
    USE_SPIRIT_PARSER(_b);
    USE_SPIRIT_PARSER(_c);
    USE_SPIRIT_PARSER(_r1);
    USE_SPIRIT_PARSER(_r2);
    USE_SPIRIT_PARSER(_r3);
    USE_SPIRIT_PARSER(_pass);
    using boost::phoenix::static_cast_;
    using boost::phoenix::if_;
    using boost::phoenix::bind;
    using boost::phoenix::construct;
    bits_type bits;

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
            (byte_(0x29) | byte_(0x19) | byte_(0x14) | byte_(0x0c)) >> bits(5) >> bits(1, 0x1) >> bits(2) >
                                                                       big_tbyte[_a = _1] > big_dword[_b = _1] >
            eps[_val = construct<ability_event_ptr>()];

    no_idea =
            byte_ >> bits(6) >> bits(1, 0x1) >> bits(1) >
                     big_tbyte[_a = _1] >
            (
                (eps[_pass = _a & 0x20] > repeat(9)[byte_]) |
                (eps[_pass = _a & 0x40] > repeat(19)[byte_]) |
                eps
                ) > eps[_val = construct<ability_event_ptr>()];

    targeted =
            byte_ >> bits(7) >> bits(1, 0x1) >
                     big_word[_a = _1] > big_dword[_b = _1] > big_word[_c = _1] > repeat(11)[byte_] /*coords? only 10 byte in build<19595*/ >
            eps[_val = construct<ability_event_ptr>()];

    move =
            byte_ >> bits(4) >> bits(4, 0x0) >
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

}}

#endif // PARSERS_ABILITY_EVENT_HPP
