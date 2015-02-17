set(SUBSYS_NAME linux)
set(SUBSYS_DESC "linux server")

set(LINUX_INCLUDES
    car.h
    streamer.h
    huffman.h
    input.h
    jpeg_utils.h
    linuxuvc.h
    output.h
    uvc_compat.h
    uvcvideo.h
    v4l2uvc.h
   )

set(LINUX_SOURCES
    ../devices/virtual_car.c
    ../devices/input_uvc.c
    ../devices/jpeg_utils.c
    linuxuvc.c
    ../devices/output_streamer.c
    ../devices/v4l2uvc.c
   )

add_executable(urobot_linux ${LINUX_SOURCES})
target_link_libraries(urobot_linux ${PROJECT_NAME} ${UROBOT_LINK_WITH})
install(TARGETS urobot_linux RUNTIME DESTINATION bin COMPONENT main)
