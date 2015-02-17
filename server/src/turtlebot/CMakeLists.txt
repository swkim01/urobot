set(SUBSYS_NAME turtlebot)
set(SUBSYS_DESC "turtlebot create")

set(TURTLEBOT_INCLUDES
    streamer.h
    streamer.h
    jpeg_utils.h
    openni_kinect.h
    roomba.h
   )

set(TURTLEBOT_SOURCES
    ../devices/input_kinect.cpp
    ../devices/jpeg_utils.c
    ../devices/roomba.c
    turtlebot.c
    ../devices/output_streamer.c
   )

add_executable(urobot_turtlebot ${TURTLEBOT_SOURCES})
target_link_libraries(urobot_turtlebot ${PROJECT_NAME} ${UROBOT_LINK_WITH} ${OPENNI_LIBRARY})
install(TARGETS urobot_turtlebot RUNTIME DESTINATION bin COMPONENT main)
