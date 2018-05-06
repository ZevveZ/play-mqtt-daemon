#include "mjpegcamera.h"
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <functional>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>

using std::string;
using std::thread;
using std::bind;

MjpegCamera::MjpegCamera(mqtt::client &cliref, string host, int port, string topic, string sa):cli(cliref), host(host), port(port), topic(topic), scriptAddress(sa){
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

MjpegCamera::~MjpegCamera(){
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

void MjpegCamera::open(){
    if(t == nullptr){
        t = new thread(bind(&MjpegCamera::run, this));
    }
}

void MjpegCamera::close(){
    if(t){
        pthread_cancel(t->native_handle());
        t->join();
        delete t;
        t = nullptr;
    }
}

void MjpegCamera::run(){
    int sockfd;
    struct sockaddr_in sin;
    string cmd ="GET /?action=stream\n\n";
    string imagebuf;
    imagebuf.reserve(20*1024);
    imagebuf.push_back('\xff');
    imagebuf.push_back('\xd8');
    char recvbuf[16*1024];
    ssize_t nr;
    char lastbyte = 0;
    bool enRecv = false;
    int fps = 2;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &sin.sin_addr);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){
        perror("socket");
    }

    if(connect(sockfd, (struct sockaddr *)&sin, sizeof(sin))<0){
        perror("connect");
        goto end;
    }
    //发送请求
    if(write(sockfd, cmd.c_str(), cmd.size())!=cmd.size()){
        perror("write");
        goto end;
    }

    while(true){
        //接收图片
        while((nr=read(sockfd, recvbuf, sizeof(recvbuf)))>0){
            for(int i=0;i<nr;++i){
                if(enRecv) imagebuf.push_back(recvbuf[i]);
                if(lastbyte==(char)0xFF){
                    if(recvbuf[i]==(char)0xD8){
                        enRecv = true;
                    }else if(recvbuf[i]==(char)0xD9){
                        if(fps==5){
                            fps=0;
                            cli.publish(topic.c_str(), imagebuf.c_str(), imagebuf.length());
                        }
                        ++fps;
                        enRecv = false;
                        imagebuf.resize(2);
                    }
                }
                lastbyte = recvbuf[i];
            }
        }
        if(nr<0){
            perror("read");
            goto end;
        }
    }
end:
    ::close(sockfd);
}

// void MjpegCamera::run(){
//     int sockfd;
//     struct sockaddr_in sin;
//     string cmd = "GET /?action=stream\n\n";
//     string imagebuf;
//     imagebuf.reserve(20*1024);
//     imagebuf.push_back('\xff');
//     imagebuf.push_back('\xd8');
//     char recvbuf[16*1024];
//     ssize_t nr;
//     char lastbyte = 0;
//     bool enRecv = false;

//     memset(&sin, 0, sizeof(sin));
//     sin.sin_family = AF_INET;
//     sin.sin_port = htons(port);
//     inet_pton(AF_INET, host.c_str(), &sin.sin_addr);

//     if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){
//         perror("socket");
//     }

//     if(connect(sockfd, (struct sockaddr *)&sin, sizeof(sin))<0){
//         perror("connect");
//         goto end;
//     }

//     //发送请求
//     if(write(sockfd, cmd.c_str(), cmd.size())!=cmd.size()){
//         perror("write");
//         goto end;
//     }
//     while((nr=read(sockfd, recvbuf, sizeof(recvbuf)))>0){
//         for(int i=0;i<nr;++i){
//             if(enRecv) imagebuf.push_back(recvbuf[i]);
//             if(lastbyte==(char)0xFF){
//                 if(recvbuf[i]==(char)0xD8){
//                     enRecv = true;
//                 }else if(recvbuf[i]==(char)0xD9){
//                     // cli.publish(topic.c_str(), imagebuf.c_str(), imagebuf.length(), 0, false);
//                     cli.publish(topic.c_str(), imagebuf.c_str(), imagebuf.length());
//                     enRecv = false;
//                     imagebuf.resize(2);
//                 }
//             }
//             lastbyte = recvbuf[i];
//         }
//     }
//     if(nr<0){
//         perror("read");
//         goto end;
//     }

// end:
//     ::close(sockfd);
// }