#include <iostream>
#include <fstream>
#include <memory>
#include <array>

#include <libmpq/mpq.h>

#include <sc2pp/types.hpp>
#include <sc2pp/parsers.hpp>
#include <sc2pp/detail/parsers_common.hpp>
#include <sc2pp/detail/parsers_game_event_fwd.hpp>
#include <sc2pp/detail/parsers_message_fwd.hpp>
#include <sc2pp/detail/parsers_object_fwd.hpp>
#include <sc2pp/detail/parsers_timestamp_fwd.hpp>
#include <sc2pp/detail/utils.hpp>

using namespace boost;
using namespace sc2pp::parsers;

namespace sc2pp {

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

        object_type header = parse_details(begin, end);

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
    
    void
    replay_t::read_details(mpq_archive_s* archive)
    {
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
        details = parse_details(begin, end);
//        if (not parse(begin, end, parsers::object_grammar_t<typeof(begin)>(), details))
//        {
//            std::cerr << "Failed to parse replay.details!" << std::endl;
//            // TODO: signal error here
//            return;
//        }

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

    void
    replay_t::read_messages(mpq_archive_s* archive)
    {
        unsigned int fileno = 0;
        long size = 0, actual_size = 0;
        libmpq__file_number(archive, "replay.message.events", &fileno);
        libmpq__file_unpacked_size(archive, fileno, &size);
        
        std::unique_ptr<unsigned char[]> buf(new unsigned char[size]);
        libmpq__file_read(archive, fileno, buf.get(), size, &actual_size);

        object_type details;
        bitshift_iterator<const unsigned char*> 
            begin(buf.get()),
            end(buf.get()+actual_size);

        messages = parse_messages(buf.get(), buf.get()+actual_size);
//        message_event_ptr event;
//        while (parse(begin, end, parsers::message_grammar_t<typeof(begin)>(), event))
//        {
//            messages.push_back(event);
//        }

        if (begin != end)
        {
            std::cerr << "Junk detected at the end of replay.message.events!" << std::endl;
            // TODO: signal error here
            return;
        }
    }

    void
    replay_t::read_events(mpq_archive_s* archive)
    {
        unsigned int fileno = 0;
        long size = 0, actual_size = 0;
        libmpq__file_number(archive, "replay.game.events", &fileno);
        libmpq__file_unpacked_size(archive, fileno, &size);
        
        std::unique_ptr<unsigned char[]> buf(new unsigned char[size]);
        libmpq__file_read(archive, fileno, buf.get(), size, &actual_size);

        const unsigned char
            *begin = buf.get(),
            *end = buf.get()+actual_size;

        events = parse_events(begin, end);
//        if (not parse(begin, end, *parsers::game_event_grammar_t<typeof(begin)>(), events))
//        {
//            std::cerr << "Error while parsing game events" << std::endl;
//            // TODO: signal error here
//            return;
//        }
    }

    replay_t::replay_t(std::string const& inputFile)
    {
        read_header(inputFile);
        mpq_archive_s* archive;
        int ret = libmpq__archive_open(&archive, inputFile.c_str(), -1);
        if (ret != 0) return; // TODO: signal error here

        read_details(archive);
        read_messages(archive);
        read_events(archive);
    }

    std::string 
    message_event_t::asString() const
    {
        std::stringstream ss;
        ss << timestamp << " - P" << player_id;
        return ss.str();
    }

    std::string
    ping_event_t::asString() const
    {
        std::stringstream ss;
        ss << message_event_t::asString() << ": PING(" << x << ", " << y << ")";
        return ss.str();
    }

    std::string
    message_t::asString() const
    {
        std::stringstream ss;
        ss << message_event_t::asString();
        switch (target)
        {
        case ALL:
            ss << "(To ALL)";
            break;
        case ALLIES:
            ss << "(To Allies)";
            break;
        default:
            ss << "(To Unknown)";
            break;
        }
        ss << ": " << text;
        return ss.str();
    }

    std::string
    unknown_message_t::asString() const
    {
        std::stringstream ss;
        ss << message_event_t::asString() << "(unknown message): " << std::hex << std::right;
        for (unsigned char c : data) ss << static_cast<unsigned int>(c) << " ";
        return ss.str();
    }

    void
    selection_event_t::mask_t::operator()()
    {
    }

    std::string
    selection_event_t::mask_t::asString() const
    {
        return "mask";
    }

    void
    selection_event_t::deselect_t::operator()()
    {
    }

    std::string
    selection_event_t::deselect_t::asString() const
    {
        return "deselect";
    }

    void
    selection_event_t::replace_t::operator()()
    {
    }

    std::string
    selection_event_t::replace_t::asString() const
    {
        return "replace";
    }

    std::string
    game_event_t::asString() const
    {
        std::stringstream ss;
        ss << timestamp << " - P" << player_id;
        return ss.str();
    }



}

namespace std
{

using namespace sc2pp;

std::ostream& operator<<(std::ostream& stream, game_event_ptr const & event)
{
    stream << (event ? event->asString() : "nullptr");
    return stream;
}

std::ostream& operator<<(std::ostream& stream, message_event_ptr const & msg)
{
    stream << (msg ? msg->asString() : "nullptr");
    return stream;
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
    stream << "Messages: \n";
    for ( message_event_ptr const & msg : rep.messages )
    {
        stream << msg << "\n";
    }
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

ostream& operator<<(ostream& stream, vector<pair<int, int> > const & vec)
{
    stream << "[";
    for (const auto & p : vec)
    {
        stream << "(" << p.first << ", " << p.second << "), ";
    }
    stream << "]";
    return stream;
}

ostream& operator<<(ostream& stream, selection_event_t::selection_modifier_ptr const & modifier)
{
    return stream << (modifier ? modifier->asString() : "nullptr");
}
}

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:

