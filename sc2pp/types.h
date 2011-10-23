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
#include <boost/date_time/posix_time/posix_time.hpp>

#include <gmpxx.h>

namespace sc2pp {

    typedef unsigned char uint8_t;

    struct byte_array;
    struct byte_map;

    typedef mpz_class hugenum_t;
    typedef long num_t;

    typedef boost::variant<num_t,
                           hugenum_t, 
                           std::string,
                           boost::recursive_wrapper<byte_array>,
                           boost::recursive_wrapper<byte_map> >
    object_type;

    template <typename T>
    bool operator==(object_type const & a, T const & b) { object_type tmp = b; return a == b; }
    template <typename T>
    bool operator==(T const & a, object_type const & b) { return b == a; }

    bool operator==(object_type const & a, object_type const & b);
    num_t get_num(object_type const & obj);

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
    
    struct color_t
    {
        int r;
        int g;
        int b;
        int a;
    };

    struct player_t
    {
        enum result_t { LOST = 0, WON = 1, UNKNOWN = 2 };
        std::string name;
        int bnet_region;
        long bnet_id;
        std::string race;
        color_t color;
        int handicap;
        result_t result;
    };

    struct event_t
    {
    };

    struct replay_t
    {
        replay_t(std::string const& file);
        enum speed_t { };
        std::string version;
        num_t build;
        num_t frames;
        typedef std::vector<player_t> players_t;
        players_t players;
        std::string map;
        boost::posix_time::ptime played_time;
        speed_t speed;

    private:
        void read_header(std::string const&);
    };

    std::ostream& operator<<(std::ostream& stream, replay_t const & rep);
    std::ostream& operator<<(std::ostream& stream, player_t const & rep);

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
