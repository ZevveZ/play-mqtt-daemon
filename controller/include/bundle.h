#ifndef BUNDLE_H
#define BUNDLE_H

#include <mqtt/client.h>
#include<thread>
#include<termios.h>
#include<string>

class Bundle{
    mqtt::client &cli;
    std::string topic_sonar;
    std::string topic_laser;
    std::string topic_gesture;
    std::thread *t = nullptr;
    struct termios saved_term;
    int ttyfd;

    void run();
    std::string getts();
    std::string genjson(std::string &&val, std::string &&ts);
public:
    Bundle(mqtt::client &cliref, std::string t_sonar, std::string t_laser, std::string t_gesture);
    ~Bundle();
    void open();
    void close();
};

#endif