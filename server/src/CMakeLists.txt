set(SERVER_INCLUDES
    buffer.h
    cam.h
    cfgparser.h
    cms.h
    css.h
    device.h
    http.h
    gettext.h
    mcs.h
    osdep.h
    presentation.h
    services.h
    sms.h
    sts.h
    target.h
    trace.h
    urobot.h
    utils.h
   )

set(SERVER_SOURCES
    buffer.c
    cam.c
    cfgparser.c
    cms.c
    css.c
    devices.c
    http.c
    mcs.c
    osdep.c
    presentation.c
    template.cpp
    services.c
    sms.c
    sts.c
    trace.c
    urobot.c
   )

add_library( ${PROJECT_NAME}
    buffer.c
    cam.c
    cfgparser.c
    cms.c
    css.c
    devices.c
    http.c
    mcs.c
    osdep.c
    presentation.c
    template.cpp
    services.c
    sms.c
    sts.c
    trace.c
    urobot.c
   )

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/include/config.h)

set(UROBOT_LINK_WITH)
list(APPEND UROBOT_LINK_WITH ${CMAKE_THREAD_LIBS_INIT} ${UPNP_LIBRARY} ${JPEG_LIBRARIES} ${CTEMPLATE_LIBRARY_RELEASE})

add_subdirectory(linux)
if (OPENNI_FOUND)
    add_subdirectory(turtlebot)
    if (KOBUKI_DRIVER_FOUND)
        add_subdirectory(kobuki)
    endif (KOBUKI_DRIVER_FOUND)
endif (OPENNI_FOUND)
if (RPI_FOUND)
    add_subdirectory(raspberrypi)
endif (RPI_FOUND)
