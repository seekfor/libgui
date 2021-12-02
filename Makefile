CC := arm-himix200-linux-gcc
AR := arm-himix200-linux-ar
STRIP := arm-himix200-linux-strip

CFLAGS  := -O3 -I./include
LDFLAGS := -lm -lc -lpthread

LIBOBJS := ./source/gui/libgui.o ./source/gui/gui.o ./source/gui/bmp.o
LIBOBJS += ./source/os/os.o ./source/os/thread.o ./source/os/network.o
LIBOBJS += ./source/fb/fb.o ./source/fb/gdi.o
LIBOBJS += ./source/hid/hid.o
LIBOBJS += ./source/utils/utils.o ./source/utils/conf.o



%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

all: ./lib/libgui.a
		
./lib/libgui.a: $(LIBOBJS)
	$(AR) rcu $@ $^

clean:
	rm -rf $(LIBOBJS)








