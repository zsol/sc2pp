#ifndef SC2PP_PARSERS_H
#define SC2PP_PARSERS_H

#include <boost/spirit/include/qi.hpp>

#include <sc2pp/detail/types.hpp>
#include <sc2pp/types.hpp>

namespace sc2pp { namespace parsers {
        typedef sc2pp::detail::hugenum_t hugenum_t;
        typedef sc2pp::detail::byte_array byte_array;
        typedef sc2pp::detail::byte_map byte_map;
        typedef sc2pp::detail::object_type object_type;

        /// ugly hack. TODO: refactor
        struct Initializer
        {
            Initializer();
        };
        
        typedef boost::spirit::qi::rule<const uint8_t*, 
                                        boost::spirit::qi::locals<int>, 
                                        std::string() > byte_string_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        int()> single_byte_integer_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        num_t()> four_byte_integer_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        boost::spirit::qi::locals<hugenum_t, int>, 
                                        hugenum_t()> variable_length_integer_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        boost::spirit::qi::locals<int>,
                                        byte_array()> array_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        boost::spirit::qi::locals<int, int>,
                                        byte_map()> map_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        object_type()> object_rule_type;
                                        
        extern byte_string_rule_type byte_string;
        extern single_byte_integer_rule_type single_byte_integer;
        extern four_byte_integer_rule_type four_byte_integer;
        extern variable_length_integer_rule_type variable_length_integer;
        extern array_rule_type array;
        extern map_rule_type map;
        extern object_rule_type object;

        typedef boost::spirit::qi::rule<const uint8_t*,
                                        boost::spirit::qi::locals<int, num_t>,
                                        num_t()> timestamp_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        boost::spirit::qi::locals<num_t, int>,
                                        message_event_ptr()> message_event_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        boost::spirit::qi::locals<int, int>,
                                        message_event_ptr(num_t, int)> ping_event_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        boost::spirit::qi::locals<int, message_t::target_t>,
                                        message_event_ptr(num_t, int)> message_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        message_event_ptr(num_t, int)> unknown_message_rule_type;

        extern timestamp_rule_type timestamp;
        extern ping_event_rule_type ping_event;
        extern message_rule_type message;
        extern unknown_message_rule_type unknown_message;
        extern message_event_rule_type message_event;

        typedef boost::spirit::qi::rule<const uint8_t*,
                                        std::vector<game_event_ptr>()> game_events_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        boost::spirit::qi::locals<num_t, int>,
                                        game_event_ptr()> game_event_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        boost::spirit::qi::locals<int>,
                                        game_event_ptr(num_t, int)> unknown_event_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        game_event_ptr(num_t, int)> initial_event_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        game_event_ptr(num_t, int)> player_joined_event_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        game_event_ptr(num_t, int)> game_started_event_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        game_event_ptr(num_t, int)> action_event_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        game_event_ptr(num_t, int)> player_left_event_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        boost::spirit::qi::locals<int, std::vector<num_t> >,
                                        game_event_ptr(num_t, int)> resource_transfer_event_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        num_t() > resource_rule_type;

        extern game_events_rule_type game_events;
        extern game_event_rule_type game_event;
        extern unknown_event_rule_type unknown_event;
        extern player_joined_event_rule_type player_joined_event;
        extern game_started_event_rule_type game_started_event;
        extern initial_event_rule_type initial_event;
        extern action_event_rule_type action_event;
        extern player_left_event_rule_type player_left_event;
        extern resource_rule_type resource;
        extern resource_transfer_event_rule_type resource_transfer_event;
    } 
}

#endif

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:

