//点对点，客户端
//一个客户端和一个服务器端通信
//父进程发送数据，子进程接收数据

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>
#include <signal.h>

using namespace std;

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

void handler(int sig) {
    cout << "recvied a sig = " << sig;
    exit(EXIT_SUCCESS);
}

int main() {
    // 1. 创建套接字
    //PF_INET表示ipv4协议,SOCK_STREAM表示流式套接字
    int sockfd;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        ERR_EXIT("socket");

    // 2. 连接服务器端地址（分配套接字地址）
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));//初始化地址
    servaddr.sin_family = AF_INET;//地址族
    servaddr.sin_port = htons(5188);//端口号;转换为网络字节序
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 3. 请求连接
    //主动模式，通过connect发起连接
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("connect");
    }


    // 4. 数据交换


    pid_t pid;
    pid = fork();
    if (pid == -1)
        ERR_EXIT("fork");
    if (pid == 0) {
        char recvbuf[1024];
        while (1) {
            memset(recvbuf, 0, sizeof(recvbuf));
            int ret = read(sockfd, recvbuf, sizeof(recvbuf));
            if (ret == -1)ERR_EXIT("read");
            if (ret == 0) {
                cout << "peer close" << endl;
                break;
            }
            cout << recvbuf << endl;
        }
        close(sockfd);
        //当子进程退出时，通知父进程也退出
        kill(getppid(), SIGUSR1);
    } else {
        //接收来自子进程的信号量
        signal(SIGUSR1, handler);

        char sendbuf[1024] = {0};
        // 键盘输入获取
        while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
            // 写入服务器
            write(sockfd, sendbuf, strlen(sendbuf));
            // 服务器读取
            memset(sendbuf, 0, sizeof(sendbuf));
        }
        close(sockfd);
    }
    //关闭套接口
    close(sockfd);

    return 0;
}