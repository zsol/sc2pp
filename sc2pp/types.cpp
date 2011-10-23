#include <iostream>
#include <fstream>
#include <memory>
#include <array>

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

        template <typename Arg>
        std::ostream& operator()(Arg const & arg) const { return os << arg; }
    private:
        std::ostream& os;
    };
}

namespace sc2pp {
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

    void
    replay_t::read_header(std::string const& fileName)
    {
        std::ifstream file(fileName, std::ios::binary);
        if (not file.good())
        {
            return; // TODO: signal error here
        }

        static const int BUFSIZE = 1024;
        std::array<char, BUFSIZE> buf;
        file.seekg(16, std::ios::beg); // TODO: check mpq magic bytes
        file.read(buf.data(), BUFSIZE);

        const unsigned char 
            *begin = reinterpret_cast<std::array<unsigned char, BUFSIZE>::iterator>(buf.begin()), 
            *end = reinterpret_cast<std::array<unsigned char, BUFSIZE>::iterator>(buf.begin()) + file.tellg();
        file.close();

        object_type header;
        parse(begin, end, parsers::object, header);

        try {
            byte_map& header_map = get<byte_map>(header);
            byte_map& version_map = get<byte_map>(header_map.map[1]);
            
            std::stringstream ss;
            ss << version_map[1] << "." << version_map[2] << "." << version_map[3];
            version = ss.str();
            
            build = get_num(version_map[4]);
            frames = get_num(header_map[3]);

        } catch (bad_get const & e)
        {
            std::cerr << "Error (" << e.what() << ") while parsing invalid header: " << header << std::endl;
            return; // TODO: signal proper error here
        }

    }

    replay_t::replay_t(std::string const& inputFile)
    {
        read_header(inputFile);
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
                player.bnet_region = get_num(bnet_data[2]);
                player.bnet_id = get_num(bnet_data[4]);
                player.race = boost::get<std::string>(player_data[2]);
                player.color.a = get_num(color_data[0]);
                player.color.r = get_num(color_data[1]);
                player.color.g = get_num(color_data[2]);
                player.color.b = get_num(color_data[3]);
                player.handicap = get_num(player_data[6]);
                player.result = static_cast<player_t::result_t>(get_num(player_data[8]));

                players.push_back(player);
            }

            map = boost::get<std::string>(details_data[1]);
            hugenum_t win_stamp = boost::get<hugenum_t>(details_data[5]);

            static const hugenum_t win_unix_offset = (hugenum_t(369) * 365 + 89) * 24 * 3600 * 10000000;
            win_stamp = (win_stamp - win_unix_offset) / 10000000;
            played_time = boost::posix_time::from_time_t(win_stamp.get_si());
        } catch (boost::bad_get const & e) {
            std::cerr << "Invalid get: " << e.what() << "; " << details << std::endl;
            return; // TODO: signal error here
        }

    }

    std::ostream& operator<<(std::ostream& stream, replay_t const & rep)
    {
        stream << "Replay version " << rep.version << ", build " 
               << rep.build << ". Number of frames: " << rep.frames
               << ", Speed: " << rep.speed << ", Played at: " << rep.played_time
               << "\nMap: " << rep.map
               << "\nPlayers: ";
        for (player_t const & player : rep.players)
        {
            stream << player << "; ";
        }
        stream << "\n";
        return stream;
    }

    std::ostream& operator<<(std::ostream& stream, player_t const & p)
    {
        stream << "Player name: " << p.name << ", Race: " << p.race
               << ", Color: (" << p.color.a << ", " << p.color.r << ", "
               << p.color.g << ", " << p.color.b << ") "
               << "BNet region: " << p.bnet_region << ", ID: " 
               << p.bnet_id << ", Handicap: " << p.handicap
               << ", Result: " << p.result;
        return stream;
    }


}

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:

