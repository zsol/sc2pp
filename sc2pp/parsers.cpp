#include <sc2pp/parsers.hpp>
#include <sc2pp/detail/utils.hpp>
#include <sc2pp/detail/parsers.hpp>
#include <sc2pp/detail/types.hpp>

#include <vector>

using std::vector;
using sc2pp::detail::object_type;

namespace sc2pp
{
    vector<message_event_ptr> parse_messages(const unsigned char* begin, const unsigned char* end)
    {
        vector<message_event_ptr> ret;
        bitshift_iterator<const unsigned char*> bitbegin(begin), bitend(end);
        parse(bitbegin, bitend, *sc2pp::parsers::message_grammar_t<bitshift_iterator<const unsigned char*>>(), ret);
        return ret;
    }

    object_type parse_details(const unsigned char* begin, const unsigned char* end)
    {
        object_type ret;
        bitshift_iterator<const unsigned char*> bitbegin(begin), bitend(end);
        parse(bitbegin, bitend, sc2pp::parsers::object_grammar_t<bitshift_iterator<const unsigned char*>>(), ret);
        return ret;
    }

    vector<game_event_ptr> parse_events(const unsigned char* begin, const unsigned char* end)
    {
        vector<game_event_ptr> ret;
        bitshift_iterator<const unsigned char*> bitbegin(begin), bitend(end);
        parse(bitbegin, bitend, *sc2pp::parsers::game_event_grammar_t<bitshift_iterator<const unsigned char*>>(), ret);
        return ret;
    }

}
