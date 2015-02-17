set(SUBSYS_NAME raspberrypi)
set(SUBSYS_DESC "raspberrypi server")

set(RASPBERRYPI_INCLUDES
    raspberrypi.h
    streamer.h
    huffman.h
    input.h
    jpeg_utils.h
    output.h
    uvc_compat.h
    uvcvideo.h
    v4l2uvc.h
   )

set(RASPBERRYPI_SOURCES
    ../devices/raspberrypi.c
    ../devices/input_uvc.c
    ../devices/jpeg_utils.c
    rpi.c
    ../devices/output_streamer.c
    ../devices/v4l2uvc.c
   )

add_executable(urobot_raspberrypi ${RASPBERRYPI_SOURCES})
target_link_libraries(urobot_raspberrypi ${PROJECT_NAME} ${UROBOT_LINK_WITH})
install(TARGETS urobot_raspberrypi RUNTIME DESTINATION bin COMPONENT main)
