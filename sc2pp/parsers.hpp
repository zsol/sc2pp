#ifndef SC2PP_PARSERS_H
#define SC2PP_PARSERS_H

#include <sc2pp/types.hpp>
#include <sc2pp/detail/types.hpp>

namespace sc2pp {

    std::vector<message_event_ptr> parse_messages(const unsigned char* begin, const unsigned char* end);
    detail::object_type parse_details(const unsigned char* begin, const unsigned char* end);
    std::vector<game_event_ptr> parse_events(const unsigned char* begin, const unsigned char* end);
    
}

#endif

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:

