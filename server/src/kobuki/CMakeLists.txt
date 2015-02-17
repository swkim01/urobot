set(SUBSYS_NAME kobuki)
set(SUBSYS_DESC "kobuki turtlebot")

set(KOBUKI_INCLUDES
    streamer.h
    jpeg_utils.h
    openni_kinect.h
    kobuki.h
   )

set(KOBUKI_SOURCES
    ../devices/input_kinect.cpp
    ../devices/jpeg_utils.c
    ../devices/kobuki.cpp
    ../devices/output_streamer.c
    turtlebot2.cpp
   )

include_directories(${KOBUKI_DRIVER_INCLUDE_DIR})

add_executable(urobot_kobuki ${KOBUKI_SOURCES})
target_link_libraries(urobot_kobuki ${PROJECT_NAME} ${UROBOT_LINK_WITH} ${OPENNI_LIBRARY} ${KOBUKI_DRIVER_LIBRARY})
install(TARGETS urobot_kobuki RUNTIME DESTINATION bin COMPONENT main)
