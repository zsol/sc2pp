############################## Minimum CMake version to run ###################
CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
cmake_policy(SET CMP0007 NEW)

############################## Checks #########################################
IF ( NOT CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE Release CACHE STRING "Available build types: Debug | Release" FORCE )
ENDIF ( NOT CMAKE_BUILD_TYPE )

############################## Project name ###################################
PROJECT( SC2pp )

SET( SC2pp_VERSION 0.1.0 )

############################## Modules configuration ##########################
OPTION( BUILD_DOC    "Build the doxygen documentation"         ON  )
OPTION( BUILD_STATIC "Build as a static library"               OFF  )
OPTION( BUILD_TESTS  "Build the tests"                         ON  )
OPTION( BUILD_TOOLS  "Build the tools"                         ON )
OPTION( BUILD_PYTHON "Build python wrapper library" OFF )

MESSAGE( STATUS "Building documentation:   " ${BUILD_DOC}    )
MESSAGE( STATUS "Building static:          " ${BUILD_STATIC} )
MESSAGE( STATUS "Building tests:           " ${BUILD_TESTS}  )
MESSAGE( STATUS "Building tools:           " ${BUILD_TOOLS}  )
MESSAGE( STATUS "Building python wrapper:  " ${BUILD_PYTHON} )

############################## Sources ########################################
FILE( GLOB_RECURSE Sources        sc2pp/*.cpp sc2pp/detail/*.cpp sc2pp/*.hpp sc2pp/*.h sc2pp/detail/*.hpp )

# Group files in virtual folders under Visual Studio
SOURCE_GROUP( "Sources"           FILES ${Sources} )

############################## External libraries #############################
#
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG SC2PP_DEBUG)

IF (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-g0 -std=c++0x -Wall -Wextra -Werror -fPIC")
ENDIF()
INCLUDE( FindBoost )
SET( Boost_USE_MULTITHREADED ON )
SET( Boost_USE_STATIC_LIBS   OFF )
FIND_PACKAGE( Boost REQUIRED COMPONENTS program_options)

FIND_LIBRARY( MPQLib NAMES mpq )
FIND_LIBRARY( GMP NAMES gmp libgmp )
FIND_LIBRARY( GMPXX NAMES gmpxx libgmpxx )
FIND_LIBRARY( zlib NAMES z )
FIND_LIBRARY( bz2 NAMES bz2 )

IF (NOT MPQLib)
  MESSAGE(STATUS "libmpq not found, using bundled version" )
  include(ExternalProject)
  ExternalProject_Add( MPQLib
					   SOURCE_DIR ${PROJECT_SOURCE_DIR}/deps/libmpq
					   CONFIGURE_COMMAND CFLAGS=-fPIC ./configure --prefix=<INSTALL_DIR> --disable-shared
					   BUILD_IN_SOURCE 1
					   BUILD_COMMAND "make")

  ExternalProject_Add_Step( MPQLib autogen
							WORKING_DIRECTORY <SOURCE_DIR>
							COMMAND ./autogen.sh
							DEPENDEES update
							DEPENDERS configure
							ALWAYS 0)

  ExternalProject_Get_Property(MPQLib install_dir)
  set(MPQLib "${install_dir}/lib/libmpq.a")
  INCLUDE_DIRECTORIES( ${install_dir}/include )
ENDIF()
############################## Includes #######################################
SET( CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON )
INCLUDE_DIRECTORIES( . ) # Headers

FIND_PATH( MPQInclude mpq.h
           PATH_SUFFIXES libmpq )
FIND_PATH( GMPXXInclude gmpxx.h )

INCLUDE_DIRECTORIES( BEFORE ${MPQInclude}/.. ${GMPXXInclude} )

SET(CMAKE_INSTALL_RPATH "\\\$ORIGIN/../lib")

############################## Uninstall target ###############################
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

add_custom_target(uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

############################## Project generation #############################
IF ( BUILD_STATIC )
  ADD_LIBRARY( SC2pp STATIC ${Sources} ${PublicHeaders} ${PrivateHeaders} )
ELSE ( BUILD_STATIC )
  ADD_LIBRARY( SC2pp SHARED ${Sources} ${PublicHeaders} ${PrivateHeaders} )
ENDIF ( BUILD_STATIC )

TARGET_LINK_LIBRARIES( SC2pp ${MPQLib} ${GMP} ${GMPXX} ${zlib} ${bz2} )

############################## Target Properties ##############################
SET_TARGET_PROPERTIES( SC2pp PROPERTIES LINKER_LANGUAGE CXX ) # Language forced to C++

SET_TARGET_PROPERTIES( SC2pp PROPERTIES DEBUG_POSTFIX "d" ) # Debug mode: a trailing "d"

SET_TARGET_PROPERTIES( SC2pp PROPERTIES RELEASE_POSTFIX "" ) # Release mode: no postfix

SET_TARGET_PROPERTIES( SC2pp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE} ) # Path for shared libraries
SET_TARGET_PROPERTIES( SC2pp PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE} ) # Path for module libraries
SET_TARGET_PROPERTIES( SC2pp PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE} ) # Path for import libraries

SET_TARGET_PROPERTIES( SC2pp PROPERTIES OUTPUT_NAME "sc2pp" ) # Output name

INSTALL( DIRECTORY sc2pp/
         DESTINATION include/sc2pp
         FILES_MATCHING
         PATTERN "*.hpp" )
INSTALL( TARGETS SC2pp 
         RUNTIME DESTINATION bin 
         LIBRARY DESTINATION lib 
         ARCHIVE DESTINATION lib )

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

IF ( BUILD_TOOLS )
  ADD_SUBDIRECTORY( tools )
ENDIF ( BUILD_TOOLS )

IF (BUILD_PYTHON)
  ADD_SUBDIRECTORY( python )
ENDIF (BUILD_PYTHON)

