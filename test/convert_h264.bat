@echo off

ffmpeg -nostdin ^
    -f h264 -i input.h264 ^
    -y output.mp4

pause
