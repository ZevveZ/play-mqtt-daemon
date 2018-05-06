#! /bin/sh
cd ../mjpg-streamer/mjpg-streamer-experimental/
# export LD_LIBRARY_PATH="$(pwd)"
./mjpg_streamer -i "./input_uvc.so" -o "./output_http.so -w ./www"
