#include <iostream>
#include <memory>

#include <libmpq/mpq.h>

#include <sc2pp/types.h>
#include <sc2pp/parsers.h>

using namespace boost;
using namespace sc2pp::parsers;

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

    replay_t::replay_t(std::string const& inputFile)
    {
        mpq_archive_s* archive;
        int ret = libmpq__archive_open(&archive, inputFile.c_str(), -1);
        if (ret != 0) return; // TODO: signal error here

        unsigned int fileno = 0;
        long size = 0, actual_size = 0;
        libmpq__file_number(archive, "replay.details", &fileno);
        libmpq__file_unpacked_size(archive, fileno, &size);
        
        std::unique_ptr<unsigned char[]> buf(new unsigned char[size]);
        libmpq__file_read(archive, fileno, buf.get(), size, &actual_size);

        object_type details;
        const unsigned char
            *begin = buf.get(),
            *end = buf.get()+actual_size;
        if (not parse(begin, end, parsers::object, details))
        {
            std::cerr << "Failed to parse replay.details!" << std::endl;
            // TODO: signal error here
            return;
        }

        try {
            auto& details_data = boost::get<byte_map>(details);
            auto& players_data = boost::get<byte_array>(details_data[0]);
            for (auto& player_obj : players_data.array)
            {
                player_t player;
                auto& player_data = boost::get<byte_map>(player_obj);
                auto& bnet_data = boost::get<byte_map>(player_data[1]);
                auto& color_data = boost::get<byte_map>(player_data[3]);

                player.name = boost::get<std::string>(player_data[0]);
                player.bnet_region = boost::get<long>(bnet_data[1]);
                player.bnet_id = boost::get<long>(bnet_data[3]);
                player.race = boost::get<std::string>(player_data[2]);
                player.color.a = boost::get<long>(color_data[0]);
                player.color.r = boost::get<long>(color_data[1]);
                player.color.g = boost::get<long>(color_data[2]);
                player.color.b = boost::get<long>(color_data[3]);
                player.handicap = boost::get<long>(player_data[6]);
                player.result = static_cast<player_t::result_t>(boost::get<long>(player_data[8]));

                players.push_back(player);
                std::cout << player_data << std::endl;
            }

            map = boost::get<std::string>(details_data[1]);
            played_time = boost::posix_time::from_time_t(boost::get<long>(details_data[5]));
        } catch (boost::bad_get const & e) {
            std::cerr << "Invalid get: " << e.what() << "; " << details << std::endl;
            return; // TODO: signal error here
        }

    }

}

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:

