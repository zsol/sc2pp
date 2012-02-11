#ifndef PARSERS_GAME_EVENT_FWD_HPP
#define PARSERS_GAME_EVENT_FWD_HPP

#include <sc2pp/detail/parsers_common.hpp>
#include <sc2pp/detail/parsers_timestamp_fwd.hpp>
#include <sc2pp/detail/parsers_ability_event_fwd.hpp>


namespace sc2pp { namespace parsers {

namespace qi = boost::spirit::qi;

template <typename Iterator>
struct game_event_grammar_t
        : public qi::grammar<Iterator, qi::locals<num_t, int>, game_event_ptr()>
{
    game_event_grammar_t();

    timestamp_grammar_t<Iterator> timestamp;
    ability_event_grammar_t<Iterator> ability_event;
    byteint_grammar_t<Iterator> byteint;

    qi::rule<Iterator, qi::locals<num_t, int>, game_event_ptr()> game_event;
    qi::rule<Iterator, qi::locals<int>, game_event_ptr(num_t, int, int)> unknown_event;
    qi::rule<Iterator, game_event_ptr(num_t, int)> initial_event;
    qi::rule<Iterator, game_event_ptr(num_t, int)> player_joined_event;
    qi::rule<Iterator, game_event_ptr(num_t, int)> game_started_event;
    qi::rule<Iterator, game_event_ptr(num_t, int)> action_event;
    qi::rule<Iterator, game_event_ptr(num_t, int)> player_left_event;
    qi::rule<Iterator, qi::locals<int>, game_event_ptr(num_t, int)> resource_transfer_event;
    qi::rule<Iterator, num_t() > resource;
    qi::rule<Iterator, qi::locals<int>,
            selection_event_t::selection_modifier_ptr()> selection_modifier;
    qi::rule<Iterator, qi::locals<int>,
            std::vector<std::pair<int, int> >()> selected_types;
    qi::rule<Iterator, qi::locals<int>, std::vector<int>()> selected_ids;
    qi::rule<Iterator,
            qi::locals<selection_event_t::selection_modifier_ptr, std::vector<std::pair<int, int> >, int>,
            game_event_ptr(num_t, int)> selection_event;
    qi::rule<Iterator,
            qi::locals<int, selection_event_t::mask_t::bitmask_t>,
            selection_event_t::selection_modifier_ptr()> selection_bitmask;
    qi::rule<Iterator, qi::locals<int>, int()> type;
    qi::rule<Iterator, qi::locals<int>, game_event_ptr(num_t, int)> camera_event;
    qi::rule<Iterator, void()> camera_payload;
    qi::rule<Iterator, qi::locals<int, int>, game_event_ptr(num_t, int)> hotkey_event;

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

#endif // PARSERS_GAME_EVENT_FWD_HPP
