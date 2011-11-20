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
                                        std::shared_ptr<message_event_t>()> message_event_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        boost::spirit::qi::locals<int, int>,
                                        std::shared_ptr<message_event_t>(num_t, int)> ping_event_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        boost::spirit::qi::locals<int, message_t::target_t>,
                                        std::shared_ptr<message_event_t>(num_t, int)> message_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        std::shared_ptr<message_event_t>(num_t, int)> unknown_message_rule_type;
        extern timestamp_rule_type timestamp;
        extern ping_event_rule_type ping_event;
        extern message_rule_type message;
        extern unknown_message_rule_type unknown_message;
        extern message_event_rule_type message_event;
  
    } 
}

#endif

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:

