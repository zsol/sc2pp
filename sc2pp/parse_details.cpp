#include <sc2pp/parsers.hpp>
#include <sc2pp/detail/utils.hpp>
#include <sc2pp/detail/parsers_object.hpp>
#include <sc2pp/detail/types.hpp>

using sc2pp::detail::object_type;

namespace sc2pp
{
    object_type unsafe_parse_details(const unsigned char* begin,
                                     const unsigned char* end)
    {
        object_type ret;
        bitshift_iterator<const unsigned char*> bitbegin(begin), bitend(end);
        parse(bitbegin, bitend, sc2pp::parsers::object_grammar_t<bitshift_iterator<const unsigned char*>>(), ret);
        return ret;
    }

    object_type parse_details(const unsigned char *begin,
                              const unsigned char *end)
    {
        try {
            return unsafe_parse_details(begin, end);
        } catch (parse_error const & err)
        {
            std::cerr << "Error while parsing details: "
                      << err.what() << std::endl;
            return object_type();
        }
    }
}
