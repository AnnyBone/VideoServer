@echo off

ffmpeg -nostdin ^
    -f h264 -i output.h264 ^
    -y output.mp4

pause
