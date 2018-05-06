#include "camera.h"
#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>

using std::string;

Camera::Camera(string sa) : scriptAddress(sa)
{
}

void Camera::open()
{
    if (pid != 0)
        return;

    if ((pid = fork()) == 0)
    {
        //child process
        setpgrp();
        execl("/bin/sh", "sh", scriptAddress.c_str(), (char *)nullptr);
    }
    else if (pid < 0)
    {
        perror("fork");
    }
}

void Camera::close()
{
    if (pid != 0)
    {
        kill(-pid, SIGTERM); //向整个子进程组发送信号
        if (waitpid(pid, nullptr, 0) == -1)
        {
            perror("waitpid");
        }
        pid = 0;
    }
}

Camera::~Camera()
{
    close();
}