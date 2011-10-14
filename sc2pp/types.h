#ifndef SC2PP_TYPES_H
#define SC2PP_TYPES_H

#include <map>
#include <vector>
#include <iosfwd>

#include <boost/variant.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/include/std_pair.hpp>

namespace sc2pp {

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
        operator array_type const&() const { return array; }
        operator array_type&() { return array; }
        
        object_type& operator[](size_t n) { return array[n]; }
        object_type const& operator[](size_t n) const { return array[n]; }
    };

    struct byte_map
    {
        typedef std::map<int, object_type> map_type;
        byte_map() {}
        byte_map(map_type const & a) : map(a) {}
        map_type map;
        bool operator==(byte_map const & other) const;
        operator map_type const&() const { return map; }
        operator map_type&() { return map; }
        
        object_type& operator[](int n) { return map[n]; }
    };
    
    std::ostream& operator<<(std::ostream&, byte_array const &);
    std::ostream& operator<<(std::ostream&, byte_map const &);
    std::ostream& operator<<(std::ostream&, object_type const &);
    
    
}

BOOST_FUSION_ADAPT_STRUCT(sc2pp::byte_array, 
                          (std::vector<sc2pp::object_type>, array))
BOOST_FUSION_ADAPT_STRUCT(sc2pp::byte_map, 
                          (sc2pp::byte_map::map_type, map))

#endif

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:
