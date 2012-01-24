#ifndef SC2PP_DETAIL_PARSERS_GAME_EVENT_HPP
#define SC2PP_DETAIL_PARSERS_GAME_EVENT_HPP

#include <sc2pp/detail/parsers_common.hpp>
#include <sc2pp/detail/parsers_timestamp.hpp>
#include <sc2pp/detail/parsers_ability_event.hpp>

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
        using boost::spirit::big_word;
        using boost::spirit::repeat;
        using boost::spirit::inf;
        using boost::spirit::eps;
        using boost::spirit::_val;
        using boost::spirit::omit;
        using boost::spirit::_1;
        using boost::spirit::_a;
        using boost::spirit::_b;
        using boost::spirit::_c;
        using boost::spirit::_r1;
        using boost::spirit::_r2;
        using boost::spirit::_pass;
        using boost::spirit::as_string;
        using boost::phoenix::static_cast_;
        using boost::phoenix::if_;
        using boost::phoenix::bind;
        using boost::phoenix::construct;


        game_event =
                timestamp[_a = _1] > bits(5)[_b = _1] >
                (

                    (bits(3, 0x0) > initial_event(_a, _b)) |
                    (bits(3, 0x1) > action_event(_a, _b)) |
                    (bits(3, 0x2) > unknown_event(_a, _b)) |
                    (bits(3, 0x3) > camera_event(_a, _b))
                    )[_val = _1] > bits;

        unknown_event =
            byte_[_a = _1 & 0x7]
            >> byte_[_val = bind(unknown_event_t::make, _r1, _r2, _a, _1)];

        initial_event %=
                (
                    player_joined_event(_r1, _r2) |
                    game_started_event(_r1, _r2)
                );

        player_joined_event =
            (byte_(0xB) | byte_(0xC) | byte_(0x2C))
            > eps[_val = bind(player_joined_event_t::make, _r1, _r2)];

        game_started_event =
            byte_(0x5)
            > eps[_val = bind(game_started_event_t::make, _r1, _r2)];

        action_event %=
                (
                    player_left_event(_r1, _r2) |
                    resource_transfer_event(_r1, _r2) |
                    selection_event(_r1, _r2) |
                    ability_event(_r1, _r2)
                );

        player_left_event =
            byte_(0x9)
            > eps[_val = bind(player_left_event_t::make, _r1, _r2)];

        resource_transfer_event =
                bits(4, 0xf) > bits(4)[_a = _1] >
                byte_(0x84) >
                repeat(4)[resource][_val = bind(resource_transfer_event_t::make, _r1, _r2, _a, _1)];

        resource =
            big_dword[_val = (static_cast_<num_t>(_1) >> 8) * (static_cast_<num_t>(_1) & 0xf0) + (static_cast_<num_t>(_1) & 0x0f)];

        selection_event =
                bits(4, 0xc) > bits(4)[_c = _1] > byte_ >> selection_modifier[_a = _1] > selected_types[_b = _1] >
                selected_ids[_val = make_selection_event(_r1, _r2, _b, _1, _a, _c)];

        selection_modifier =
                (bits(2, 0x1) > selection_bitmask[_val = _1]) |
                (bits(2, 0x2) > byte_[_a = _1] > repeat(_a)[byteint][_val = bind(selection_event_t::deselect_t::make, _1)]) |
                (bits(2, 0x3) > byte_[_a = _1] > repeat(_a)[byteint][_val = bind(selection_event_t::replace_t::make, _1)]) |
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
                byte_[_a = _1] >
                repeat(_a / 8)[byte_[boost::phoenix::bind(bitset_append, _b, _1)]] >
                bits(_a % 8)[boost::phoenix::bind(bitset_append, _b, _1)] >
                eps[boost::phoenix::bind(bitset_resize, _b, _a, false)] >
                eps[_val = boost::phoenix::bind(selection_event_t::mask_t::make, _b)];

        camera_event =
                (
                    (byte_(0x87) > repeat(0, 2)[big_word[_pass = (_1 & 0xf0)]] > big_word) |
                    (byte_(0x0a) > eps[_pass = false]) | // TODO
                    (bits(4, 0x8) > byteint[_a = _1] > bits(4) > repeat(_a*4+1)[byte_]) |
                    (bits(4, 0x1) > bits(4) > repeat(3)[byte_] > (
                         (bits(4) >> bits(1, 0x1) >> bits(3) > byte_) ||
                         (bits(5) >> bits(1, 0x1) >> bits(2) > byte_) ||
                         (bits(6) >> bits(1, 0x1) >> bits(1) > byte_ > byte_)
                         ) )

                )[_val = bind(camera_movement_event_t::make, _r1, _r2)];

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
        // boost::spirit::qi::on_error<boost::spirit::qi::fail>(*this, errorhandler<boost::spirit::unused_type, Iterator>);

    }

    timestamp_grammar_t<Iterator> timestamp;
    ability_event_grammar_t<Iterator> ability_event;
    byteint_grammar_t<Iterator> byteint;

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
                            boost::spirit::qi::locals<int>,
                            game_event_ptr(num_t, int)> resource_transfer_event;
    boost::spirit::qi::rule<Iterator,
                            num_t() > resource;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<int>,
            selection_event_t::selection_modifier_ptr()> selection_modifier;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<int>,
            std::vector<std::pair<int, int> >()> selected_types;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<int>,
            std::vector<int>()> selected_ids;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<selection_event_t::selection_modifier_ptr, std::vector<std::pair<int, int> >, int>,
            game_event_ptr(num_t, int)> selection_event;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<int, selection_event_t::mask_t::bitmask_t>,
            selection_event_t::selection_modifier_ptr()> selection_bitmask;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<int>,
            int()> type;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<int>,
            game_event_ptr(num_t, int)> camera_event;

    struct make_selection_event_impl
    {
        /*
         * +---------+---------+----------------+-----+------+-----+
         * | A1      | A2      | A3             | A4  | A5   | A6  |
         * +---------+---------+----------------+-----+------+-----+
         * |timestamp|playerid |[(type, count)] |[id] |method| slot|
         * +---------+---------+----------------+-----+------+-----+
         */
        template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
        struct result
        {
            typedef game_event_ptr type;
        };
        template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
        typename result<A1, A2, A3, A4, A5, A6>::type operator()(A1 a1, A2 a2, A3 const & a3, A4 const & a4, A5 a5, A6 a6) const
        {
            selection_event_t::objects_t objs;
            objs.reserve(a4.size());
            int typecount = 0; auto it = a3.cbegin();
            for (const auto & obj : a4)
            {
                if (it == a3.cend()) handle_error(a1);
                if (typecount >= it->second) { ++it; typecount = 0; }

                objs.push_back(std::make_pair(obj, it->first));
            }
            return selection_event_t::make(a1, a2, a6, objs, a5);
        }

        template <typename A1>
        void handle_error(A1 a) const
        {
            std::stringstream ss;
            ss << "Number of selected objects do not match total number of selected types "
               << "while parsing event at timestamp " << a;
            throw parse_error(ss.str());
        }
    };

    boost::phoenix::function<make_selection_event_impl> make_selection_event;

};
}}

#endif // SC2PP_DETAIL_PARSERS_GAME_EVENT_HPP
