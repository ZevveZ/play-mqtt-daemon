#! /bin/bash
EXECPATH=$PWD
sudo apt update && sudo apt-get install build-essential gcc make cmake git libssl-dev libjpeg8-dev libev-dev linux-tools-common linux-tools-generic libncurses5-dev -y

# build paho.mqtt.c
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c/
mkdir build && cd build
cmake -DPAHO_WITH_SSL=TRUE .. 
make && sudo make install
 
# build paho.mqtt.cpp
cd $EXECPATH
git clone https://github.com/eclipse/paho.mqtt.cpp.git
cd paho.mqtt.cpp/
mkdir build && cd build
cmake ..
make && sudo make install
sudo sh -c 'echo "/usr/local/lib" >> /etc/ld.so.conf'
sudo ldconfig -v
 
# build mjpg-streamer
cd $EXECPATH
git clone https://github.com/jacksonliam/mjpg-streamer.git
cd mjpg-streamer/mjpg-streamer-experimental/
make

# build lepd
cd $EXECPATH
git clone https://github.com/ZevveZ/lepd.git
cd lepd/
make

# build controller
cd $EXECPATH/controller/
make

# configure systemd
cd $EXECPATH
sed -i "6s#path#$EXECPATH#" play-mqtt-daemon.service
sudo cp play-mqtt-daemon.service /lib/systemd/system/
sudo systemctl enable play-mqtt-daemon
sudo systemctl start play-mqtt-daemon
