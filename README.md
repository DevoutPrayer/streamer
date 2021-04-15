# Streamer

This project aims at publishing camera data into RTMP server.

# Releases:
*Streamer v1.0*
![GPL License](https://img.shields.io/github/license/DevoutPrayer/streamer) ![size](https://img.shields.io/github/repo-size/DevoutPrayer/streamer) 

Develop on firefly RK3399 board and publish camera data into RTMP server.
# Dependents
1. ffmpeg(avformat avcodec avutil)
2. rkmpp
3. nginx rtmp server(optional) [[install reference]](https://blog.csdn.net/qq_32739503/article/details/83064132)
# How to use
*on-board compile*
```
git clone git@github.com:DevoutPrayer/streamer
cd streamer
make
./streamer /dev/video0 rtmp://rtmp_server_addr/live/room
```
*cross compile*
```
git clone git@github.com:DevoutPrayer/streamer
cd streamer 
wget https://github.com/UWVG/aarch64-none-linux-gnu/archive/refs/heads/master.zip
unzip master.zip
make cross-compile="1"
```
clone this project,checkout into one branch,make and run
# Releases:
*Streamer v1.0(@a59906c)*
- get video data from camera through v4l2 libs
- encode video raw data into h264 format via rk_mpp libs
  - now it can only support video raw data
  - jpeg format data should be decode before sending into encoder
- mux the h264 flow into flv and send it via ffmpeg libs

# How to contribute
1. Fork this branch.
2. Commit your changes.
3. Send pull requests.

