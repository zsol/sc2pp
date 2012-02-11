#ifndef PARSERS_ABILITY_EVENT_FWD_HPP
#define PARSERS_ABILITY_EVENT_FWD_HPP

#include <sc2pp/detail/parsers_common.hpp>

namespace sc2pp { namespace parsers {

namespace qi = boost::spirit::qi;

template <typename Iterator>
struct ability_event_grammar_t
        : qi::grammar<Iterator, game_event_ptr(num_t, int)>
{
    ability_event_grammar_t();

    big_tbyte_grammar_t<Iterator> big_tbyte;
    coordinate_grammar_t<Iterator> coordinate;

    qi::rule<Iterator, game_event_ptr(num_t, int)> ability_event;
    qi::rule<Iterator, qi::locals<int>, game_event_ptr(num_t, int)> command_card;
    qi::rule<Iterator, qi::locals<float, float>,
            game_event_ptr(num_t, int, int)> command_card_location;
    qi::rule<Iterator, qi::locals<num_t, num_t>,
            game_event_ptr(num_t, int, int)> command_card_target;
    qi::rule<Iterator, qi::locals<num_t, num_t>,
            game_event_ptr(num_t, int)> cancel;
    qi::rule<Iterator, qi::locals<num_t>, game_event_ptr(num_t, int)> no_idea;
    qi::rule<Iterator, qi::locals<int, num_t, int>,
            game_event_ptr(num_t, int)> targeted;
    qi::rule<Iterator, qi::locals<float, float>, game_event_ptr(num_t, int)> move;
};
}}
#endif // PARSERS_ABILITY_EVENT_FWD_HPP
