#include <iostream>
#include <sc2pp/types.h>
#include <sc2pp/parsers.h>

#include <boost/program_options.hpp>

#include <libmpq/mpq.h>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char** argv)
{
    po::options_description desc("Possible options");
    po::positional_options_description posdesc;
    po::variables_map vm;

    desc.add_options()
        ("help", "display this help message")
        ("mpq", "treat input file as an MPQ archive (SC2Replay file), not as the replay.details file")
        ;
    posdesc.add("input", -1);
    po::store(po::command_line_parser(argc, argv)
              .options(desc)
              .positional(posdesc)
              .run(),
              vm);
    po::notify(vm);

    if (vm.count("input") == 0)
    {
        const size_t BUFSIZE = 1 << 20; // 1MB
        typedef array<unsigned char, BUFSIZE> buffer_type;
        buffer_type buf;
        auto tobegin = begin(buf), toend = end(buf);
        cin >> noskipws;
        istream_iterator<char> frombegin(cin), fromend;
        size_t actual_size = 0;
        while (frombegin != fromend && tobegin != toend)
            *tobegin++ = *frombegin++, ++actual_size;

        sc2pp::object_type details;
        parse(buf.cbegin(), buf.cbegin()+actual_size, sc2pp::parsers::object, details);
        std::cout << details << std::endl;
    }


    return 0;
}
