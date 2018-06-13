#include "monitor.h"
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <signal.h>
#include <functional>
#include <sys/wait.h>

using std::string;
using std::vector;
using std::pair;
using std::thread;
using std::bind;
using std::cerr;
using std::endl;

Monitor::Monitor(mqtt::client &cliref, string host, int port, string sa) : cli(cliref), host(host), port(port), scriptAddress(sa)
{
    if(pid==0){
        if((pid=fork())==-1){
            perror("fork");
        }else if(pid==0){
            //child process
            setpgrp();
            if(execl("/bin/sh", "sh", scriptAddress.c_str(), (char *)0)==-1){
                perror("execl");
            }
        }
    }
}

Monitor::~Monitor()
{
    close();
    if(pid){
        kill(-pid, SIGTERM);
        if (waitpid(pid, nullptr, 0) == -1)
        {
            perror("waitpid");
        }
        pid=0;
    }
}

void Monitor::open()
{   
    if(t==nullptr){
        t = new thread(bind(&Monitor::run, this));
    }
}

void Monitor::close()
{
    if(t){
        pthread_cancel(t->native_handle());
        t->join();
        delete t;
        t = nullptr;
    }
}

void Monitor::run()
{
    struct sockaddr_in servaddr;
    int sockfd;
    char rbuf[1024];
    string msg;
    ssize_t nr;
    string endstr = "lepdendstring";

    msg.reserve(20*1024);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &servaddr.sin_addr);
    try{
        while(true)
        {
            for(auto &cmd : cmdvec){
                // 使用socket与LEPD的JSON RPC服务器建立连接
                sockfd = socket(AF_INET, SOCK_STREAM, 0);

                if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
                {
                    perror("connect");
                    goto end;
                }
                // 发送JSON RPC控制指令
                if (write(sockfd, cmd.first.c_str(), cmd.first.length() + 1) != cmd.first.length()+1)
                {
                    perror("write");
                    goto end;
                }
                // 获取返回数据
                while(true){
                    if((nr = read(sockfd, rbuf, sizeof(rbuf)))==-1){
                        perror("read");
                        goto end;
                    }else{
                        msg.append(rbuf, nr);
                        if(nr<sizeof(rbuf)) break;  //last read
                        // 检查分隔符，判断数据是否接收完成
                        for(int i=0; i<nr-endstr.length(); ++i){
                            if(strcmp(rbuf+i, endstr.c_str())==0) break;
                        }
                    }
                }
                ::close(sockfd);
                // 将数据发布到MQTT话题上
                cli.publish(cmd.second, msg.c_str(), msg.length());
                msg.erase();
            }
            sleep(10);
        }
    }catch (const mqtt::exception& exc) {
		cerr << exc.what() << endl;
	}   
end:
    ::close(sockfd);
}