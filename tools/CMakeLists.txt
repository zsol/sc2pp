ADD_EXECUTABLE( mpqinfo mpqinfo.cpp )
TARGET_LINK_LIBRARIES( mpqinfo SC2pp )

ADD_EXECUTABLE( sc2details detailsinfo.cpp )
TARGET_LINK_LIBRARIES( sc2details SC2pp ${Boost_LIBRARIES} )

ADD_EXECUTABLE( mpqunpack unpack.cpp )
TARGET_LINK_LIBRARIES( mpqunpack SC2pp ${Boost_LIBRARIES} )

ADD_EXECUTABLE( sc2messages messages.cpp )
TARGET_LINK_LIBRARIES( sc2messages SC2pp ${Boost_LIBRARIES} )

ADD_EXECUTABLE( sc2replay replay.cpp )
TARGET_LINK_LIBRARIES( sc2replay SC2pp ${Boost_LIBRARIES} )

ADD_EXECUTABLE( sc2events events.cpp )
TARGET_LINK_LIBRARIES( sc2events SC2pp ${Boost_LIBRARIES} )

INSTALL ( TARGETS mpqinfo sc2details mpqunpack sc2messages sc2replay sc2events
          RUNTIME DESTINATION bin )

