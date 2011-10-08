#ifndef SC2PP_PARSERS_H
#define SC2PP_PARSERS_H

#include <boost/spirit/include/qi.hpp>

#include <sc2pp/types.h>

namespace sc2pp { namespace parsers {
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
                                        long()> four_byte_integer_rule_type;
        typedef boost::spirit::qi::rule<const uint8_t*,
                                        boost::spirit::qi::locals<long, int>, 
                                        long()> variable_length_integer_rule_type;
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
  
    } 
}

#endif

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:

