@echo off

ffmpeg -nostdin ^
    -f gdigrab -video_size 640x480 -offset_x 100 -offset_y 100 -framerate 25 ^
    -i desktop ^
    -y output.mp4

pause
