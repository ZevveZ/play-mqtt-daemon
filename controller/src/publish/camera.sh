#! /bin/sh
# 使用硬件加速编码H.264，并将视频推送到远程SRS服务器，已经被弃用了，转而使用mcamera
gst-launch-1.0 -e v4l2src device=/dev/video0 ! video/x-raw,format=I420,framerate=30/1,width=320,height=240 ! queue ! nxvideoenc codec=video/x-h264  ! flvmux ! rtmpsink location="rtmp://222.201.144.236/live/livestream"