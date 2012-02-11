############################## Minimum CMake version to run ###################
CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

############################## Checks #########################################
IF ( NOT CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE Release CACHE STRING "Available build types: Debug | Release" FORCE )
ENDIF ( NOT CMAKE_BUILD_TYPE )

############################## Project name ###################################
PROJECT( SC2pp )

SET( SC2pp_MAJOR_VERSION 0 )
SET( SC2pp_MINOR_VERSION 0 )
SET( SC2pp_PATCH_LEVEL   0 )

############################## Modules configuration ##########################
OPTION( BUILD_DOC    "Build the doxygen documentation"         ON  )
OPTION( BUILD_STATIC "Build as a static library"               OFF  )
OPTION( BUILD_TESTS  "Build the tests"                         ON  )

MESSAGE( STATUS "Building documentation:   " ${BUILD_DOC}    )
MESSAGE( STATUS "Building static:          " ${BUILD_STATIC} )
MESSAGE( STATUS "Building tests:           " ${BUILD_TESTS}  )

############################## Sources ########################################
FILE( GLOB_RECURSE Sources        sc2pp/*.cpp sc2pp/detail/*.cpp sc2pp/*.hpp sc2pp/*.h sc2pp/detail/*.hpp )

# Group files in virtual folders under Visual Studio
SOURCE_GROUP( "Sources"           FILES ${Sources} )

############################## External libraries #############################
#
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG SC2PP_DEBUG)

IF (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-g0 -std=c++0x -Wall -Wextra -Werror")
ENDIF()
INCLUDE( FindBoost )
SET( Boost_USE_MULTITHREADED ON )
SET( Boost_USE_STATIC_LIBS   ON )
FIND_PACKAGE( Boost REQUIRED COMPONENTS iostreams program_options)

FIND_LIBRARY( MPQLib NAMES mpq )
FIND_LIBRARY( GMP NAMES gmp libgmp )
FIND_LIBRARY( GMPXX NAMES gmpxx libgmpxx )

############################## Includes #######################################
INCLUDE_DIRECTORIES( . ) # Headers

############################## Project generation #############################
IF ( BUILD_STATIC )
  ADD_LIBRARY( SC2pp STATIC ${Sources} ${PublicHeaders} ${PrivateHeaders} )
ELSE ( BUILD_STATIC )
  ADD_LIBRARY( SC2pp SHARED ${Sources} ${PublicHeaders} ${PrivateHeaders} )
ENDIF ( BUILD_STATIC )

TARGET_LINK_LIBRARIES( SC2pp ${Boost_LIBRARIES} ${MPQLib} ${GMP} ${GMPXX} )


################################### Options ###################################
IF ( BUILD_STATIC )
  ADD_DEFINITIONS( -DBUILD_STATIC )
ENDIF( BUILD_STATIC )

IF ( BUILD_DOC )
  #TODO build documentation using Doxygen
ENDIF( BUILD_DOC )

IF ( BUILD_TESTS )
  ADD_SUBDIRECTORY( tests )
#  FIND_PACKAGE( Boost REQUIRED COMPONENTS program_options )
ENDIF( BUILD_TESTS )

ADD_SUBDIRECTORY( tools )

############################## Target Properties ##############################
SET_TARGET_PROPERTIES( SC2pp PROPERTIES LINKER_LANGUAGE CXX ) # Language forced to C++

SET_TARGET_PROPERTIES( SC2pp PROPERTIES DEBUG_POSTFIX "d" ) # Debug mode: a trailing "d"

SET_TARGET_PROPERTIES( SC2pp PROPERTIES RELEASE_POSTFIX "" ) # Release mode: no postfix

SET_TARGET_PROPERTIES( SC2pp PROPERTIES PREFIX  ""  ) # Remove the "lib" prefix

SET_TARGET_PROPERTIES( SC2pp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ./${CMAKE_BUILD_TYPE} ) # Path for shared libraries
SET_TARGET_PROPERTIES( SC2pp PROPERTIES LIBRARY_OUTPUT_DIRECTORY ./${CMAKE_BUILD_TYPE} ) # Path for module libraries
SET_TARGET_PROPERTIES( SC2pp PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ./${CMAKE_BUILD_TYPE} ) # Path for import libraries

SET_TARGET_PROPERTIES( SC2pp PROPERTIES OUTPUT_NAME "sc2pp" ) # Output name

