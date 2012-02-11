%module sc2pp

%include "std_string.i"
%include "std_vector.i"
%include "std_pair.i"
%include "std_shared_ptr.i"

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

%{
#define SWIG_FILE_WITH_INIT
#include <sc2pp/types.hpp>

using std::ostream;
using std::vector;
using std::pair;
%}

namespace std
{
    %template(game_event_vector) vector<sc2pp::game_event_ptr>;
    %template(message_vector) vector<sc2pp::message_event_ptr>;
}

%include "../sc2pp/types.hpp"
