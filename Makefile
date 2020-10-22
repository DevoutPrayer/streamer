
CC		:= aarch64-linux-gnu-gcc
CFLAGS	:= $(shell pkg-config --cflags rockchip_mpp)
LDFLAGS	:= $(shell pkg-config --libs rockchip_mpp)

streamer : streamer.o
	$(CC) -o streamer streamer.o $(LDFLAGS)
streamer.o : streamer.c
	$(CC) -c $(CFLAGS) streamer.c

clean :
	rm *.o