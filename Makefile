#
#
CC	:= aarch64-linux-gnu-gcc
CFLAGS	:= $(shell pkg-config --cflags rockchip_mpp) -w
LDFLAGS	:= $(shell pkg-config --libs rockchip_mpp) -lavformat -lavcodec -lavutil



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

streamer.so :  rtmp.so v4l2.so mpp.so
	$(CC) -shared -o libstream.so rtmp.so v4l2.so mpp.so $(LDFLAGS)
v4l2.so : v4l2.c
	$(CC) -fPIC -c $(CFLAGS) v4l2.c -o v4l2.so
mpp.so : mpp.c
	$(CC) -fPIC -c $(CFLAGS) mpp.c -o mpp.so
rtmp.so : rtmp.c
	$(CC) -fPIC -c $(CFLAGS) rtmp.c -o rtmp.so     



all : m_streamer streamer.so
clean :
	rm *.o
	rm *.so
