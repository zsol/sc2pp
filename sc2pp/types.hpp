#ifndef SC2PP_TYPES_H
#define SC2PP_TYPES_H

#include <vector>
#include <array>
#include <iosfwd>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/dynamic_bitset.hpp>

#include <libmpq/mpq.h>

namespace sc2pp {

    typedef long num_t;

    typedef std::domain_error parse_error;

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

    struct game_event_t
    {
        game_event_t() = default;
        game_event_t(num_t const & ts, int pid) : timestamp(ts), player_id(pid) {}
        num_t timestamp;
        int player_id;

        virtual std::string asString() const;

        virtual ~game_event_t() {}
    };
    typedef std::shared_ptr<game_event_t> game_event_ptr;

    struct unknown_event_t : public game_event_t
    {
        unknown_event_t() = default;
        unknown_event_t(num_t const & ts, int pid, int ty, int c) : game_event_t(ts, pid), type(ty), code(c) {}

        static game_event_ptr make(num_t const & ts, int pid, int ty, int c) { return std::make_shared<unknown_event_t>(ts, pid, ty, c); }

        int type;
        int code;
    };
    typedef std::shared_ptr<unknown_event_t> unknown_event_ptr;

    struct player_joined_event_t : public game_event_t
    {
        player_joined_event_t() {}
        player_joined_event_t(num_t const & ts, int pid) : game_event_t(ts, pid) {}

        static game_event_ptr make(num_t const & ts, int pid) { return std::make_shared<player_joined_event_t>(ts, pid); }
    };
    typedef std::shared_ptr<player_joined_event_t> player_joined_event_ptr;

    struct player_left_event_t : public game_event_t
    {
        player_left_event_t() {}
        player_left_event_t(num_t const & ts, int pid) : game_event_t(ts, pid) {}

        static game_event_ptr make(num_t const & ts, int pid) { return std::make_shared<player_left_event_t>(ts, pid); }
    };
    typedef std::shared_ptr<player_left_event_t> player_left_event_ptr;

    struct game_started_event_t : public game_event_t
    {
        game_started_event_t() {}
        game_started_event_t(num_t const & ts, int pid) : game_event_t(ts, pid) {}

        static game_event_ptr make(num_t const & ts, int pid) { return std::make_shared<game_started_event_t>(ts, pid); }
    };
    typedef std::shared_ptr<game_started_event_t> game_started_event_ptr;

    /**
      \todo Flesh out camera movement details
      */
    struct camera_movement_event_t : public game_event_t
    {
        camera_movement_event_t() {}
        camera_movement_event_t(num_t const & ts, int pid) : game_event_t(ts, pid) {}
        static game_event_ptr make(num_t const& ts, int pid) { return std::make_shared<camera_movement_event_t>(ts, pid); }
    };
    typedef std::shared_ptr<camera_movement_event_t> camera_movement_event_ptr;

    struct resource_transfer_event_t : public game_event_t
    {
        typedef std::vector<num_t> resources_t;

        resource_transfer_event_t() = default;
        resource_transfer_event_t(num_t const & ts, int pid, int tgt, resources_t const & res) : game_event_t(ts, pid), target(tgt), resources(res) {}

        static game_event_ptr make(num_t const & ts, int pid, int tgt, resources_t const & res) { return std::make_shared<resource_transfer_event_t>(ts, pid, tgt, res); }

        int target;
        resources_t resources;
    };
    typedef std::shared_ptr<resource_transfer_event_t> resource_transfer_event_ptr;

    struct hotkey_event_t : public game_event_t
    {
    };
    typedef std::shared_ptr<hotkey_event_t> hotkey_event_ptr;

    struct selection_event_t : public game_event_t
    {
        typedef std::vector<std::pair<int /*type*/, int/*id*/> > objects_t;

        struct selection_modifier_t : std::function<void ()>
        {
            virtual std::string asString() const = 0;
            virtual ~selection_modifier_t() {}
        };
        typedef std::shared_ptr<selection_modifier_t> selection_modifier_ptr;

        struct mask_t : public selection_modifier_t
        {
            typedef boost::dynamic_bitset<unsigned char> bitmask_t;
            mask_t(bitmask_t mask) : mask(mask) {}
            void operator()();

            static selection_modifier_ptr make(bitmask_t mask) { return std::make_shared<mask_t>(mask); }
            std::string asString() const;
            virtual ~mask_t() {}
            bitmask_t mask;
        };

        struct deselect_t : public selection_modifier_t
        {
            typedef std::vector<int> indices_t;

            deselect_t(indices_t const & ind) : indices(ind) {}
            void operator()();
            
            static selection_modifier_ptr make(indices_t const & ind) { return std::make_shared<deselect_t>(ind); }
            std::string asString() const;
            virtual ~deselect_t() {}
            indices_t indices;
        };

        struct replace_t : public selection_modifier_t
        {
            typedef std::vector<int> indices_t;

            replace_t(indices_t const & ind) : indices(ind) {}
            void operator()();

            static selection_modifier_ptr make(indices_t const & ind) { return std::make_shared<replace_t>(ind); }
            std::string asString() const;
            virtual ~replace_t() {}
            indices_t indices;
        };

        selection_event_t() = default;
        selection_event_t(num_t const & ts, int pid, int s, objects_t const & objs, selection_modifier_ptr const & mod) : game_event_t(ts, pid), slot(s), objects(objs), modifier(mod) {}

        static game_event_ptr make(num_t const & ts, int pid, int s, objects_t const & objs, selection_modifier_ptr const & mod) { return std::make_shared<selection_event_t>(ts, pid, s, objs, mod); }

        int slot;
        objects_t objects;
        selection_modifier_ptr modifier; // can be nullptr
    };
    typedef std::shared_ptr<selection_event_t> selection_event_ptr;

    struct ability_event_t : public game_event_t
    {
    };
    typedef std::shared_ptr<ability_event_t> ability_event_ptr;

    struct message_event_t
    {
        message_event_t() = default;
        message_event_t(num_t const & ts, int pid) : timestamp(ts), player_id(pid) {}
        num_t timestamp;
        int player_id;

        virtual std::string asString() const;
        virtual ~message_event_t() {}
    };
    typedef std::shared_ptr<message_event_t> message_event_ptr;

    struct ping_event_t : public message_event_t
    {
        ping_event_t() = default;
        ping_event_t(num_t const & ts, int pid, int x_, int y_) : message_event_t(ts, pid), x(x_), y(y_) {}
        static std::shared_ptr<message_event_t> make(num_t const & ts, int pid, int x_, int y_) { return std::make_shared<ping_event_t>(ts, pid, x_, y_); }
        virtual std::string asString() const;

        int x;
        int y;
    };

    struct message_t : public message_event_t
    {
        enum target_t { ALL = 0, ALLIES = 2 };

        message_t() = default;
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
        std::vector<game_event_ptr> events;
    private:
        void read_header(std::string const&);
        void read_details(mpq_archive_s* archive);
        void read_messages(mpq_archive_s* archive);
        void read_events(mpq_archive_s* archive);
    };
    typedef std::shared_ptr<replay_t> replay_ptr;

}

namespace std
{
ostream& operator<<(ostream& stream, sc2pp::replay_t const & rep);
ostream& operator<<(ostream& stream, sc2pp::player_t const & rep);
ostream& operator<<(ostream& stream, sc2pp::message_event_ptr const & msg);
ostream& operator<<(ostream& stream, sc2pp::game_event_ptr const & event);
ostream& operator<<(ostream& stream, sc2pp::selection_event_t::selection_modifier_ptr const & modifier);
ostream& operator<<(ostream& stream, vector<pair<int, int> > const & vec);
}

#endif

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:
