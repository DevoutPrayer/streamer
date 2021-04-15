#
#
ifdef cross-compile
	CC	:= aarch64-none-linux-gnu-master/bin/aarch64-none-linux-gnu-gcc
	CFLAGS	:= -I rkmpp/inc -I ffmpeg/inc -w 
	LDFLAGS	:= -L rkmpp/lib -L ffmpeg/lib  -lrockchip_mpp -lavformat -lavcodec -lavutil -lpthread -Wl,-rpath ffmpeg/lib 
else
	CC	:= aarch64-linux-gnu-gcc
	CFLAGS	:= $(shell pkg-config --cflags rockchip_mpp) -w
	LDFLAGS	:= $(shell pkg-config --libs rockchip_mpp) -lavformat -lavcodec -lavutil
endif

m_streamer : streamer.o rtmp.o v4l2.o mpp.o 
	$(CC) -o streamer streamer.o rtmp.o v4l2.o mpp.o  $(LDFLAGS)  
streamer.o : streamer.c
	$(CC) -c $(CFLAGS) streamer.c
rtmp.o : rtmp.c
	$(CC) -c $(CFLAGS) rtmp.c
v4l2.o : v4l2.c
	$(CC) -c $(CFLAGS) v4l2.c
mpp.o : mpp.c
	$(CC) -c $(CFLAGS) mpp.c


all : m_streamer 
clean :
	rm *.o
