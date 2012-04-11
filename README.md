What is sc2pp?
--------------

sc2pp is a tool and programming library to extract and analyze replay files made by [Blizzard Entertainment][blz]'s [Starcraft II][sc2] game. Inspired by awesome projects such as [phpsc2replay][], [sc2reader][] and [sc2gears][], the main focus of sc2pp is for users to be able to parse thousands of replays in about a second, i.e. performance. This is why it is implemented in C++ with the help of [libmpq][] (a C library for parsing MPQ files) and [Boost.Spirit][spirit] (a C++ framework for writing compile time parsers). Currently, sc2pp is available as a standalone set of command line tools, a library that can be linked against, or a python module. A PHP module is also planned in the near future, so stay tuned.

  [blz]: http://blizzard.com
  [sc2]: http://starcraft2.com
  [libmpq]: https://github.com/ge0rg/libmpq
  [spirit]: http://boost-spirit.com
  [phpsc2replay]: http://code.google.com/p/phpsc2replay/
  [sc2reader]: https://github.com/GraylinKim/sc2reader
  [sc2gears]: http://code.google.com/p/phpsc2replay/

How to install?
---------------

The source code is (and will always be) available to anyone from [the repository][repo]. I plan to release binaries for various linux distros and windows in the future, although this will require a significant extra effort, so it is not yet done. The preferred way to install is through the [zero-install feed][0ifeed] (if you haven't heard of zero-install, I encourage you to visit http://0install.net - it is an awesome concept). Failing that, your only option is to download and compile sc2pp on your own.

  [repo]: https://github.com/zsol/sc2pp
  [0ifeed]: http://snowglo.be/~zsol/zero-install/interfaces/SC2pp.xml

Okay, so how to compile?
------------------------

Assuming you are on a modern Linux distribution, (apart from the essential tools like a C++ compiler) you will need CMake, the Boost libraries (and header files), GMP (and header files), zlib, bz2 and optionally libmpq (although you can opt use the version bundled with sc2pp). On a recent Ubuntu distro, this means these packages:

  cmake libboost-dev libboost-date-time-dev libboost-program-options-dev libgmp10 libgmp-dev zlib1g libbz2-1.0 build-essential

Once you have the dependencies, building and installing sc2pp should be a simple process:

1. Clone the git repository from github:

  git clone git://github.com/zsol/sc2pp.git

2. If you wish to use the bundled libmpq, make sure to pull that down, too:

  cd sc2pp; git submodule init; git submodule update

3. Create a temporary build directory somewhere:

  mkdir build; cd build

4. Create the build infrastructure:

  cmake [path_to_sc2pp_source]

5. Start the build:

  make

6. If everything went well, you should be able to install the generated binaries and header files:

  sudo make install

The tools
---------

Building the tools can be switched off by adding "-DBUILD_TOOLS=OFF" to the cmake command in the build instructions. There are several tools built by default:

+ *mpqinfo* is able to tell you diagnostics about the MPQ file itself; it is useful for normal MPQ files as well as SC2Replay files.

+ *mpqunpack* can extract the contents of an MPQ file. As with "mpqinfo", this can also be used on any MPQ file, including SC2Replay files.

+ *sc2details* extracts some general information from a replay file, like the players' names, the map they played on, or when the game took place.

+ *sc2messages* extracts the chat messages from a replay file.

+ *sc2events* parses the game events (actions, screen movement, selections, etc.).

+ *sc2replay* does everything the other "sc2" prefixed tools do.

The library
-----------

By default a shared library called "libsc2pp.so" (on linux) is generated for you. *TODO* header files, linkage, api documentation

Using it from Python
--------------------

Add "-DBUILD_PYTHON=ON" to the cmake command arguments. *TODO* python versions, how to use, distutils...

Using it from PHP
-----------------

Not yet implemented, stay tuned :)
