ADD_EXECUTABLE( mpqinfo mpqinfo.cpp )
TARGET_LINK_LIBRARIES( mpqinfo SC2pp )

ADD_EXECUTABLE( detailsinfo detailsinfo.cpp )
TARGET_LINK_LIBRARIES( detailsinfo SC2pp )

ADD_EXECUTABLE( unpack unpack.cpp )
TARGET_LINK_LIBRARIES( unpack SC2pp )

ADD_EXECUTABLE( messages messages.cpp )
TARGET_LINK_LIBRARIES( messages SC2pp )