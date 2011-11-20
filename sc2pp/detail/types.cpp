#include <sc2pp/detail/types.hpp>

using namespace boost;

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

        template <typename Arg>
        std::ostream& operator()(Arg const & arg) const { return os << arg; }
    private:
        std::ostream& os;
    };
}

namespace sc2pp { namespace detail {
    
        bool operator==(object_type const & a, object_type const & b)
        {
            return boost::apply_visitor(::object_equal(), a, b);
        }

        num_t get_num(object_type const & obj)
        {
            try {
                return get<num_t>(obj);
            } catch (bad_get const & e)
            {
                hugenum_t const& n = get<hugenum_t>(obj);
                static const hugenum_t max = hugenum_t(1) << (8 * sizeof(num_t) - 1);
                if (n < max) return n.get_si();
                // TODO: else signal error!
            }
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
}

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:
