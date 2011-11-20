#ifndef SC2PP_TYPES_H
#define SC2PP_TYPES_H

#include <vector>
#include <array>
#include <iosfwd>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace sc2pp {

    typedef long num_t;

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
    typedef std::shared_ptr<player_t> player_ptr;

    struct message_event_t
    {
        message_event_t() {}
        message_event_t(num_t const & ts, int pid) : timestamp(ts), player_id(pid) {}
        num_t timestamp;
        int player_id;

        virtual std::string asString() const;
        virtual ~message_event_t() {}
    };
    typedef std::shared_ptr<message_event_t> message_event_ptr;

    struct ping_event_t : public message_event_t
    {
        ping_event_t() {}
        ping_event_t(num_t const & ts, int pid, int x_, int y_) : message_event_t(ts, pid), x(x_), y(y_) {}
        static std::shared_ptr<message_event_t> make(num_t const & ts, int pid, int x_, int y_) { return std::make_shared<ping_event_t>(ts, pid, x_, y_); }
        virtual std::string asString() const;

        int x;
        int y;
    };

    struct message_t : public message_event_t
    {
        enum target_t { ALL = 0, ALLIES = 2 };

        message_t() {}
        message_t(num_t const & ts, int pid, target_t tgt, const std::string& str) : message_event_t(ts, pid),
                                                                                     target(tgt), text(str) {}
        static std::shared_ptr<message_event_t> make(num_t const & ts, int pid, target_t tgt, const std::string& str) { return std::make_shared<message_t>(ts, pid, tgt, str); }
        virtual std::string asString() const;

        target_t target;
        std::string text;
    };
    
    struct unknown_message_t : public message_event_t
    {
        unknown_message_t() {}
        unknown_message_t(num_t const & ts, int pid, std::array<unsigned char, 4> const & d) 
            : message_event_t(ts, pid), data(d) {}
        static std::shared_ptr<message_event_t> make(num_t const & ts, int pid, std::array<unsigned char, 4> const & d) { return std::make_shared<unknown_message_t>(ts, pid, d); }
        virtual std::string asString() const;

        std::array<unsigned char, 4> data;
    };

    struct replay_t
    {
        replay_t(std::string const& file);
        enum speed_t { SLOWER = 1, SLOW, NORMAL, FAST, FASTER };
        std::string version;
        num_t build;
        num_t frames;
        typedef std::vector<player_t> players_t;
        players_t players;
        std::string map;
        boost::posix_time::ptime played_time;
        speed_t speed;
        std::vector<message_event_ptr> messages;
    private:
        void read_header(std::string const&);
        void read_details(mpq_archive_s* archive);
        void read_messages(mpq_archive_s* archive);
    };
    typedef std::shared_ptr<replay_t> replay_ptr;

    std::ostream& operator<<(std::ostream& stream, replay_t const & rep);
    std::ostream& operator<<(std::ostream& stream, player_t const & rep);
    std::ostream& operator<<(std::ostream& stream, message_event_ptr const & msg);

}

#endif

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:
