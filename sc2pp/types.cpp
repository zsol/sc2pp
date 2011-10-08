#include <iostream>

#include <sc2pp/types.h>

namespace {
    struct object_equal : public boost::static_visitor<bool>
    {
        template <typename T, typename U>
        bool operator()(T const &, U const &) const { return false; }
        
        template <typename T>
        bool operator()(T const & lhs, T const & rhs) const { return lhs == rhs; }
    };

    struct object_ostreamer : public boost::static_visitor<std::ostream&>
    {
        object_ostreamer(std::ostream& ostream) : os(ostream) {}

        std::ostream& operator()(long l) const { return os << l; }
        std::ostream& operator()(std::string const & str) const {return os << str;}
        std::ostream& operator()(sc2pp::byte_array const & arr) const { return os << arr; }
        std::ostream& operator()(sc2pp::byte_map const & map) const { return os << map; }
    private:
        std::ostream& os;
    };
}

namespace sc2pp {
    bool operator==(object_type const & a, object_type const & b)
    {
        return boost::apply_visitor(::object_equal(), a, b);
    }

    bool byte_array::operator==(byte_array const & other) const 
    { 
        ::object_equal visitor;
        return std::equal(array.begin(), array.end(), other.array.begin(),
                          boost::apply_visitor(visitor));
    }

    bool byte_map::operator==(byte_map const & other) const 
    { 
        ::object_equal visitor;
        return std::equal(map.begin(), map.end(), other.map.begin(),
                          [&visitor](map_type::value_type const & a, map_type::value_type const & b) {
                              return a.first == b.first && boost::apply_visitor(visitor, a.second, b.second); 
                          });
    }

    std::ostream& operator<<(std::ostream& os, byte_array const & arr)
    {
        os << "[";
        bool first = true;
        for (auto obj : arr.array)
        {
            if (not first) os << ", ";
            first = false;
            os << obj;
        }
        return os << "]";
    }

    std::ostream& operator<<(std::ostream& os, byte_map const & map)
    {
        os << "{";
        bool first = true;
        for (auto keyvalue : map.map)
        {
            if (not first) os << ", ";
            first = false;
            os << "(" << keyvalue.first << " -> " << keyvalue.second << ")";
        }
        return os << "}";
        
    }

    std::ostream& operator<<(std::ostream& os, object_type const & obj)
    {
        ::object_ostreamer visitor(os);
        return boost::apply_visitor(visitor, obj);
    }

}

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:

