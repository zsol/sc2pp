#include <sc2pp/parsers.hpp>
#include <sc2pp/detail/utils.hpp>
#include <sc2pp/detail/parsers_message_fwd.hpp>
#include <sc2pp/detail/types.hpp>

#include <vector>

using std::vector;
using sc2pp::detail::object_type;

namespace sc2pp
{
    vector<message_event_ptr> unsafe_parse_messages(const unsigned char* begin,
                                                    const unsigned char* end)
    {
        vector<message_event_ptr> ret;
        bitshift_iterator<const unsigned char*> bitbegin(begin), bitend(end);
        parse(bitbegin, bitend,
              *parsers::message_grammar_t<bitshift_iterator<const unsigned char*>>(),
              ret);
        return ret;
    }

    vector<message_event_ptr> parse_messages(const unsigned char *begin,
                                             const unsigned char *end)
    {
        try {
            return unsafe_parse_messages(begin, end);
        } catch (parse_error const & err)
        {
            std::cerr << "Error while parsing messages: "
                      << err.what() << std::endl;
            return vector<message_event_ptr>();
        }
    }
}
