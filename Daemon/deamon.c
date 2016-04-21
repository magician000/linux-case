/************************************
守护进程案例

编译：gcc -Wall -g -o deamon deamon.c 
查看进程：ps ux | grep -v grep | grep deamon 
查看日志：tail -f outfile 
杀死进程：kill -s SIGTERM PID 
************************************/  

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>

//程序主循环运行标记
volatile sig_atomic_t _running = 1;  

/**************
signal handler
接收到信号准备退出
***************/
void sigterm_handler(int arg)  
{
    _running = 0;
    printf("\nWill Stop...\n"); //死前之呐喊
}

/***********
初始化守护进程
************/
void init_daemon(void) 
{
    pid_t pid;
      
    /* 屏蔽一些有关控制终端操作的信号 
     * 防止在守护进程没有正常运转起来时，
     * 因控制终端受到干扰退出或挂起.
     * */  
    signal(SIGINT,  SIG_IGN);   // 终端中断  
    signal(SIGHUP,  SIG_IGN);   // 连接挂断  
    signal(SIGQUIT, SIG_IGN);   // 终端退出  
    signal(SIGPIPE, SIG_IGN);   // 向无读进程的管道写数据  
    signal(SIGTTOU, SIG_IGN);   // 后台程序尝试写操作  
    signal(SIGTTIN, SIG_IGN);   // 后台程序尝试读操作  
    signal(SIGTERM, SIG_IGN);   // 终止  
  
    // [1] fork child process and exit father process  
    pid = fork();  
    if(pid < 0)  
    {  
        perror("fork error!");  
        exit(1);  
    }  
    else if(pid > 0)  
    {  
        exit(0);  
    }  
  
    // [2] create a new session  
    setsid();  
  
    // [3] set current path  
    char szPath[1024];  
    if(getcwd(szPath, sizeof(szPath)) == NULL)  
    {  
        perror("getcwd");  
        exit(1);  
    }  
    else  
    {  
        chdir(szPath);  
        printf("set current path succ [%s]\n", szPath);  
    }  
  
    // [4] umask 0  
    umask(0);  
  
    // [5] close useless fd  
    int i;  
    int fdTableSize = getdtablesize();
    //for (i = 0; i < fdTableSize; ++i) /* 为了保证在当前终端输出一些调试信息，我们使用下面一行代码 */
    for (i = 3; i < fdTableSize; ++i)  
    {  
        close(i);  
    }  
  
    // [6] set termianl signal  
    signal(SIGTERM, sigterm_handler);
    printf("\nDaemon begin to work..., PID is %d, and use kill -9 PID to terminate\n", getpid()); 
    return;
}

/****************
守护进程启动后的工作
*****************/
void write_log()
{
    int fd;
    char *buf = "This is a Daemon...\n";
    // open file and set rw limit  
    if((fd = open("outfile", O_CREAT|O_WRONLY|O_APPEND, 0600)) < 0)  
    {  
        perror("open");  
        exit(1);  
    }  
  
    // do something in loop  
    while(_running)  
    {  
        if (write(fd, buf, strlen(buf)) != strlen(buf))  
        {  
            perror("write");  
            close(fd);  
            exit(1);  
        }  
  
        sleep(1);   //sleep 1 s 
    }
    close(fd);
}

int main(int argv, char** argc) 
{
    //初始化守护进程
    init_daemon();
    //每秒钟向日志文件输出一次
    write_log();
    //临终遗言
    printf("\nStopped!\n");

    return 0;
}