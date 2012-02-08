#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <libmpq/mpq.h>
#include <sc2pp/types.hpp>

namespace po = boost::program_options;
using namespace std;

int main(int argc, char** argv)
{
    po::options_description desc("Possible options");
    po::positional_options_description posdesc;
    po::variables_map vm;

    desc.add_options()
        ("help", "display this help message")
        ("input", po::value<vector<string>>(), "input file(s)")
        ("no-output,n", "do not display any output (useful for profiling)")
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
        cerr << "No input files specified." << endl;
        return 1;
    }

    const auto& inputs = vm["input"].as<vector<string>>();
    for (const auto& input : inputs)
    {
        sc2pp::replay_t rep(input);
        if (vm.count("no-output") == 0)
            cout << "---- " << input << " ----\n"
                 << rep << endl;
    }

    return 0;
}
