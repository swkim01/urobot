#
# Makefile
#
# Makefile for hybus
include ../../config.mak

CC	= /usr/local/arm-linux-4.1.1/bin/arm-linux-gcc
CFLAGS  += -Wall -D_GNU_SOURCE -mcpu=iwmmxt -mtune=iwmmxt -mabi=aapcs-linux -I/usr/local/arm-linux-4.1.1/arm-iwmmxt-linux-gnueabi/include
#CFLAGS += -I.
CFLAGS += -I. -DDEBUG_BUILD

LFLAGS  += -Wall -shared -fPIC

#SOURCES	= MF_And_Server.c camera.c overlay2.c yuv2jpg.c 
SOURCES	= hybus.c input_pxa.c v4l2pxa.c jpeg_utils.c output_http.c httpd.c
OBJECTS	= ${SOURCES:.c=.o}

#OUT	= hybus.so
OUT	= target.a
#LIBS = -ljpeg -L/usr/local/arm-linux-4.1.1/arm-iwmmxt-linux-gnueabi/lib -lpthread
LIBS = -ljpeg -lpthread

all: $(OUT)
	@echo Build DONE.

.c.o:
	$(CC) -c $(CFLAGS) $(OPTFLAGS) -o $@ $<

$(OUT): $(OBJECTS)
	$(AR) rcs $(OUT) $(OBJECTS)
#	$(CC) $(LFLAGS) -o $(OUT) $(OBJECTS) $(LIBS)

clean:
	rm -f $(OBJECTS) $(OUT)

distclean:
	rm -f $(OBJECTS) $(OUT)
