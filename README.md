# Streamer

This project aims at publishing camera data into RTMP server.

# Releases:
*Streamer v1.0*
- get video data from camera through v4l2 libs
- encode video raw data into h264 format via rk_mpp libs
  - now it can only support video raw data
  - jpeg format data should be decode before sending into encoder
- mux the h264 flow into flv and send it via ffmpeg libs
