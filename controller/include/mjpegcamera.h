#ifndef MJPEGCAMERA_H
#define MJPEGCAMERA_H

#include <mqtt/client.h>
#include <string>
#include <thread>
#include <unistd.h>

class MjpegCamera{
    mqtt::client &cli;
    std::string host;
    int port;
    std::string topic;
    std::thread *t = nullptr;
    std::string scriptAddress;
    pid_t pid=0;

    void run();
public:
    MjpegCamera(mqtt::client &cliref, std::string host, int port, std::string topic, std::string sa);
    ~MjpegCamera();
    void open();
    void close();
};

#endif