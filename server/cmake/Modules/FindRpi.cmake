# - Try to find Rpi BCM includes and library
# copied from http:/github.com/Wicker25/Rpi-hw/cmake/FindRpiHw.cmake
#
# BCM_FOUND - TRUE if all components are found.
# BCM_INCLUDE_DIRS - Full paths to all include dirs.
# BCM_LIBRARIES - Full paths to all libraries.
#

# Find the header files
find_path(RPI_INCLUDE_DIR bcm_host.h
    /usr/include /usr/local/include /opt/vc/include )

# Find the raspberrypi from bcm header files
if (RPI_INCLUDE_DIR)
    set (RPI_FOUND TRUE)
endif (RPI_INCLUDE_DIR)

MARK_AS_ADVANCED(
    RPI_INCLUDE_DIR
)
