%module sc2pp
%naturalvar;

%{
#define SWIG_FILE_WITH_INIT
#include <sc2pp/types.hpp>
#include <sc2pp/parsers.hpp>

using std::ostream;
using std::vector;
using std::pair;
namespace detail = sc2pp::detail;
%}

%include "std_string.i"
%include "std_vector.i"
%include "std_pair.i"
%include "std_shared_ptr.i"
%include "pybuffer.i"

%shared_ptr(sc2pp::game_event_t)
%shared_ptr(sc2pp::unknown_event_t)
%shared_ptr(sc2pp::player_joined_event_t)
%shared_ptr(sc2pp::player_left_event_t)
%shared_ptr(sc2pp::game_started_event_t)
%shared_ptr(sc2pp::camera_movement_event_t)
%shared_ptr(sc2pp::resource_transfer_event_t)
%shared_ptr(sc2pp::hotkey_event_t)
%shared_ptr(sc2pp::selection_event_t)
%shared_ptr(sc2pp::ability_event_t)
%shared_ptr(sc2pp::message_event_t)
%shared_ptr(sc2pp::ping_event_t)
%shared_ptr(sc2pp::message_t)
%shared_ptr(sc2pp::unknown_message_t)

namespace std
{
    %template(game_event_vector) vector<sc2pp::game_event_ptr>;
    %template(message_vector) vector<sc2pp::message_event_ptr>;
}

%pybuffer_binary(unsigned char *buffer, size_t s);
%inline %{
std::vector<sc2pp::game_event_ptr> parse_events(unsigned char* buffer, size_t s)
{
    return sc2pp::parse_events(buffer, buffer+s);
}
std::vector<sc2pp::message_event_ptr> parse_messages(unsigned char* buffer, size_t s)
{
    return sc2pp::parse_messages(buffer, buffer+s);
}
%}

%ignore parse_events;
%ignore parse_messages;

%include "../sc2pp/types.hpp"
%include "../sc2pp/parsers.hpp"

