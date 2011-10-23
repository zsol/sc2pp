#include <iostream>
#include <sc2pp/types.h>

using namespace std;

int main(int argc, char** argv)
{
    if (argc < 2) return 1;

    sc2pp::replay_t rep(argv[1]);

    std::cout << rep << std::endl;

    return 0;
}
