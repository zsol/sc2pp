#ifndef SC2PP_PARSERS_H
#define SC2PP_PARSERS_H

#include <map>

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/variant.hpp>
#include <boost/variant/recursive_variant.hpp>

namespace sc2pp { namespace parsers {
        typedef unsigned char uint8_t;

        struct byte_array;
        struct byte_map;

        typedef boost::variant<long, 
                               std::string, 
                               boost::recursive_wrapper<byte_array>,
                               boost::recursive_wrapper<byte_map> >
        object_type;
    
        bool operator==(object_type const & a, object_type const & b);

        struct byte_array
        {
            typedef std::vector<object_type> array_type;
            byte_array() {}
            byte_array(array_type const & a) : array(a) {}
            array_type array;
            bool operator==(byte_array const & other) const;
        };

        struct byte_map
        {
            typedef std::map<int, object_type> map_type;
            byte_map() {}
            byte_map(map_type const & a) : map(a) {}
            map_type map;
            bool operator==(byte_map const & other) const;
        };

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

BOOST_FUSION_ADAPT_STRUCT(sc2pp::parsers::byte_array, 
                          (std::vector<sc2pp::parsers::object_type>, array))
BOOST_FUSION_ADAPT_STRUCT(sc2pp::parsers::byte_map, 
                          (sc2pp::parsers::byte_map::map_type, map))

#endif

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:

