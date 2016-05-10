@echo off

ffmpeg -nostdin ^
    -f rawvideo -pixel_format yuv420p -video_size 640x480 -framerate 25 ^
    -i input.yuv ^
    -y output.mp4

pause
