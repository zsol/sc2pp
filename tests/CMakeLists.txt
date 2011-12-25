ADD_EXECUTABLE( test_parsers test_parsers.cpp )

TARGET_LINK_LIBRARIES( test_parsers SC2pp )

IF (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-std=gnu++0x")
ENDIF()

