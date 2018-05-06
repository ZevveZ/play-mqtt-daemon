#ifndef CAMERA_H
#define CAMERA_H

#include <unistd.h>
#include <string>

class Camera{
    pid_t pid = 0;
    std::string scriptAddress;
public:
    Camera(std::string sa);
    ~Camera();
    void open();
    void close();
};

#endif