#include <sc2pp/parsers.hpp>
#include <sc2pp/detail/utils.hpp>
#include <sc2pp/detail/parsers_game_event.hpp>
#include <sc2pp/detail/types.hpp>

#include <vector>

using std::vector;
using sc2pp::detail::object_type;

namespace sc2pp
{
    vector<game_event_ptr> parse_events(const unsigned char* begin, const unsigned char* end)
    {
        vector<game_event_ptr> ret;
        bitshift_iterator<const unsigned char*> bitbegin(begin), bitend(end);
        parse(bitbegin, bitend, *sc2pp::parsers::game_event_grammar_t<bitshift_iterator<const unsigned char*>>(), ret);
        return ret;
    }

}
