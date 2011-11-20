#include <iostream>
#include <fstream>
#include <array>
#include <sc2pp/detail/types.hpp>
#include <sc2pp/detail/parsers.hpp>

using namespace sc2pp::parsers;
using namespace sc2pp;
using namespace boost;

int main(int argc, char** argv)
{
    if (argc < 2) return 1;
    std::basic_ifstream<char> file(argv[1], std::ios::binary); //, ios::binary | ios::in);
    
    if (not file.good())
    {
        std::cerr << "error opening file" << std::endl;
    }

    const int BUFSIZE = 1024 * 8;
    std::array<char, BUFSIZE> buf;
    file.seekg(16, std::ios::beg);
    file.read(buf.data(), BUFSIZE);
    const unsigned char 
        *begin = reinterpret_cast<std::array<unsigned char, BUFSIZE>::iterator>(buf.begin()), 
        *end = reinterpret_cast<std::array<unsigned char, BUFSIZE>::iterator>(buf.begin()) + file.tellg();
    file.close();

    object_type header;
    parse(begin, end, object, header);

    try {
        byte_map& header_map = get<byte_map>(header);
        byte_map& version_map = get<byte_map>(header_map.map[1]);

        std::cout << "Identifier:\t" << header_map[0] << "\n"
                  << "Version:\t" << version_map[1] << "." << version_map[2] << "." << version_map[3] << "\n"
                  << "Build Num:\t" << version_map[4] << "\n"
                  << "Num Frames:\t" << header_map.map[3] << std::endl;

    } catch (bad_get const & e)
    {
        std::cerr << "Error (" << e.what() << ") while parsing invalid header: " << header << std::endl;
        return 1;
    }

    return 0;
}
