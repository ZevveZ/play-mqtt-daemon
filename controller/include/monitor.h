#ifndef MONITOR_H
#define MONITOR_H

#include <mqtt/client.h>
#include <string>
#include <vector>
#include <utility>
#include <thread>
#include <unistd.h>

class Monitor
{
    mqtt::client &cli;
    bool isclose = true;
    std::string host;
    int port;
    pid_t pid = 0;
    std::string scriptAddress;

    std::thread *t=nullptr;
    std::vector<std::pair<std::string, std::string>> cmdvec = {
        {"{\"method\":\"GetCmdMpstat\"}", "/1/monitor/cpu_stat/raw"},
        {"{\"method\":\"GetCmdMpstat-I\"}", "/1/monitor/cpu_softirq/raw"},
        {"{\"method\":\"GetProcLoadavg\"}", "/1/monitor/cpu_avgload/raw"},
        {"{\"method\":\"GetCmdProcrank\"}", "/1/monitor/memory_procrank/raw"},
        {"{\"method\":\"GetCmdIostat\"}", "/1/monitor/io_status/raw"}};

    // static void *run(void *argptr);
    void run();

  public:
    Monitor(mqtt::client &cliref, std::string host, int port, std::string sa);
    ~Monitor();
    void open();
    void close();
};

#endif