#include "bundle.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdio.h>
#include<unistd.h>
#include<ctime>
#include<cstdlib>

using std::string;
using std::thread;
using std::bind;

Bundle::Bundle(mqtt::client &cliref, std::string t_sonar, std::string t_laser, std::string t_gesture):cli(cliref), topic_sonar(t_sonar), topic_laser(t_laser), topic_gesture(t_gesture){
    // set timezone
    putenv("TZ=Asia/Shanghai");
    tzset();
    struct termios term;
    // ttyUSB0 exists?
    if((ttyfd=::open("/dev/ttyUSB0", O_RDONLY))==-1){
        perror("Bundle");
        return;
    }
    // set ttyUSB0's baundrate to 9600
    if(tcgetattr(ttyfd, &term)==-1){
        perror("Bundle tcgetattr");
        return;
    }
    saved_term = term;
    cfsetispeed(&term, B9600);
    if(tcsetattr(ttyfd, TCSANOW, &term)==-1){
        perror("Bundle tcsetattr");
        return;
    }
}

Bundle::~Bundle(){
    // reset ttyUSB0
    tcsetattr(ttyfd, TCSAFLUSH, &saved_term);
    ::close(ttyfd);
    // close thread
    close();
}

void Bundle::open(){
    if(t==nullptr){
        t = new thread(bind(&Bundle::run, this));
    }
}

void Bundle::close(){
    if(t){
        pthread_cancel(t->native_handle());
        t->join();
        delete t;
        t = nullptr;
    }
}

string Bundle::genjson(string &&val, string &&ts){
    string res("{\"val\":");
    res.append(val);
    res.append(",\"ts\":\"");
    res.append(ts);
    res.append("\"}");
    return res;
}

string Bundle::getts(){
    string res;
    time_t t;
    struct tm *ptm;
    char buf[64];
    size_t bufsz;

    time(&t);
    ptm = localtime(&t);
    bufsz = strftime(buf, 64, "%Y/%m/%d %T", ptm);
    return string(buf, bufsz);
}

void Bundle::run(){
    char buf[1024];
    int nr;
    string val;
    string prefix = "{\"val\":";
    string midfix = ",\"ts\":\"";
    string postfix = "\"}";
    
    string msg;

    while((nr=read(ttyfd, buf, sizeof(buf)))>0){
        for(int i=0;i<nr;++i){
            if(buf[i]=='\n'){
                string &&msg = genjson(string(val.begin()+1, val.end()), getts());
                // send to different topic
                switch (val[0])
                {
                case 's':
                    cli.publish(topic_sonar.c_str(), msg.c_str(), msg.length());
                    break;
                case 'l':
                    cli.publish(topic_laser.c_str(), msg.c_str(), msg.length());
                    break;
                case 'g':
                    cli.publish(topic_gesture.c_str(), msg.c_str(), msg.length());
                    break;
                }
                msg.clear();
                val.clear();
                continue;
            }
            val += buf[i];
        }
    }
}
