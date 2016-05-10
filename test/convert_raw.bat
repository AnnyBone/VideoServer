@echo off

ffmpeg -nostdin ^
    -f rawvideo -pixel_format rgb32 -video_size 640x480 -framerate 25 ^
    -i input.raw ^
    -y output.mp4

pause
