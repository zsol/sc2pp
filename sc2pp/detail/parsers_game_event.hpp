#ifndef SC2PP_DETAIL_PARSERS_GAME_EVENT_HPP
#define SC2PP_DETAIL_PARSERS_GAME_EVENT_HPP

#include <sc2pp/detail/parsers_game_event_fwd.hpp>

namespace sc2pp { namespace parsers {

template <typename Iterator>
game_event_grammar_t<Iterator>::game_event_grammar_t()
    : game_event_grammar_t::base_type(game_event, "Game Event")
{
    USE_SPIRIT_PARSER_(byte_);
    USE_SPIRIT_PARSER(big_dword);
    USE_SPIRIT_PARSER(big_word);
    USE_SPIRIT_PARSER(repeat);
    USE_SPIRIT_PARSER(eps);
    USE_SPIRIT_PARSER(_val);
    USE_SPIRIT_PARSER(omit);
    USE_SPIRIT_PARSER(_1);
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

    game_event =
            timestamp[_a = _1] > bits(5)[_b = _1] >
            (
                (bits(3, 0x1) > action_event(_a, _b)) |
                (bits(3, 0x3) > camera_event(_a, _b)) |
                (bits(3, 0x2) > unknown_event(_a, _b, 2)) |
                (bits(3, 0x4) > unknown_event(_a, _b, 4)) |
                (bits(3, 0x0) > initial_event(_a, _b))
            )[_val = _1] > bits;

    unknown_event =
            ( ( eps[_pass = _r3 == 2] > (
                (byte_(0x6)[_a = 0x6] > repeat(8)[byte_]) |
                (byte_(0x7)[_a = 0x7] > repeat(4)[byte_]) |
                (byte_(0xE)[_a = 0xE] > repeat(4)[byte_])
            ) ) | ( eps[_pass = _r3 == 4] > (
                (byte_(0x16)[_a = 0x16] > repeat(24)[byte_]) |
                (byte_(0xC6)[_a = 0xC6] > repeat(16)[byte_]) |
                (byte_(0x87)[_a = 0x87] > repeat(4)[byte_]) |
                (byte_(0x88)[_a = 0x88] > repeat(4)[byte_]) |
                (byte_(0x00)[_a = 0x00] > repeat(10)[byte_]) |
                (bits(4, 0x2) >> bits(4)[_a = (_1 << 4 | 0x2)] > repeat(2)[byte_]) |
                (bits(4, 0xc) >> bits(4)[_a = (_1 << 4 | 0xc)] > eps)
            ) ) ) > eps[_val = p::bind(unknown_event_t::make, _r1, _r2, _r3, _a)];

    initial_event %=
            (
                player_joined_event(_r1, _r2) |
                game_started_event(_r1, _r2)
            );

    player_joined_event =
            (byte_(0xB) | byte_(0xC) | byte_(0x2C))
            > eps[_val = p::bind(player_joined_event_t::make, _r1, _r2)];

    game_started_event =
            byte_(0x5)
            > eps[_val = p::bind(game_started_event_t::make, _r1, _r2)];

    action_event %=
            (
                selection_event(_r1, _r2) |
                hotkey_event(_r1, _r2) |
                ability_event(_r1, _r2) |
                resource_transfer_event(_r1, _r2) |
                player_left_event(_r1, _r2)
            );

    player_left_event =
            byte_(0x9)
            > eps[_val = p::bind(player_left_event_t::make, _r1, _r2)];

    resource_transfer_event =
            bits(4, 0xf) > bits(4)[_a = _1] >
            byte_(0x84) >
            repeat(4)[resource][_val = p::bind(resource_transfer_event_t::make, _r1, _r2, _a, _1)];

    resource =
            big_dword[_val = (static_cast_<num_t>(_1) >> 8) * (static_cast_<num_t>(_1) & 0xf0) + (static_cast_<num_t>(_1) & 0x0f)];

    selection_event =
            bits(4, 0xc) > bits(4)[_c = _1] > byte_ >> selection_modifier[_a = _1] > selected_types[_b = _1] >
            selected_ids[_val = make_selection_event(_r1, _r2, _b, _1, _a, _c)];

    selection_modifier =
            (bits(2, 0x1) > selection_bitmask[_val = _1]) |
            (bits(2, 0x2) > byte_[_a = _1] > repeat(_a)[byteint][_val = p::bind(selection_event_t::deselect_t::make, _1)]) |
            (bits(2, 0x3) > byte_[_a = _1] > repeat(_a)[byteint][_val = p::bind(selection_event_t::replace_t::make, _1)]) |
            (bits(2, 0x0) > eps[_val = construct<selection_event_t::selection_modifier_ptr>()]);

    selected_types %=
            omit[byte_[_a = _1]] > repeat(_a)[type > byteint];

    type =
            big_word[_a = _1] > byte_[_val = (_a << 8) | _1];

    selected_ids %=
            omit[byte_[_a = _1]] > repeat(_a)[big_dword];

    typedef selection_event_t::mask_t::bitmask_t bitmask_t;
    typedef void (bitmask_t::* bitset_append_ftype)(bitmask_t::block_type);
    typedef void (bitmask_t::* bitset_resize_ftype)(bitmask_t::size_type, bool);
    auto bitset_append = static_cast<bitset_append_ftype>(&bitmask_t::append);
    auto bitset_resize = static_cast<bitset_resize_ftype>(&bitmask_t::resize);

    selection_bitmask =
            byteint[_a = _1] >
            repeat(_a / 8)[byte_[ p::bind(bitset_append, _b, _1)]] >
            repeat(_a % 8)[bits(1)[p::bind(bitset_append, _b, _1)]] >
            eps[ p::bind(bitset_resize, _b, _a, false)] >
            eps[_val = p::bind(selection_event_t::mask_t::make, _b)];

    camera_event =
            (
                (byte_(0x87) > repeat(0, 2)[big_word[_pass = (_1 & 0xf0)]] > big_word) |
                (byte_(0x0a) > eps[_pass = false]) | // TODO
                (bits(4, 0x8) > byteint[_a = _1] > bits(4) > repeat(_a*4+1)[byte_]) |
                (bits(4, 0x1) > bits(4) > repeat(3)[byte_] > camera_payload)
                )[_val = p::bind(camera_movement_event_t::make, _r1, _r2)];

    camera_payload =
            -(
                (bits(4) >> bits(1, 0x1) >> bits(3) > byte_) ||
                (bits(5) >> bits(1, 0x1) >> bits(2) > byte_) ||
                (bits(6) >> bits(1, 0x1) >> bits(1) > byte_)
                ) >> byte_;

    hotkey_event =
            bits(4, 0xD) > bits(4)[_a = _1] >> bits(2)[_b = _1]
            >> selection_modifier[_val = construct<ability_event_ptr>()];

    HANDLE_ERROR(game_event);
    HANDLE_ERROR(unknown_event);
    HANDLE_ERROR(player_joined_event);
    HANDLE_ERROR(game_started_event);
    HANDLE_ERROR(initial_event);
    HANDLE_ERROR(action_event);
    HANDLE_ERROR(player_left_event);
    HANDLE_ERROR(resource_transfer_event);
    HANDLE_ERROR(selection_event);
    HANDLE_ERROR(camera_event);
    HANDLE_ERROR(hotkey_event);
    // boost::spirit::qi::on_error<boost::spirit::qi::fail>(*this, errorhandler<boost::spirit::unused_type, Iterator>);

}

}}

#endif // SC2PP_DETAIL_PARSERS_GAME_EVENT_HPP
