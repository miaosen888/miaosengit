#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include <arpa/inet.h>
#include<netdb.h>
#include <pthread.h>
#include </test/zigbeeSerial.h> 

 
#define PORT 4156
#define PORT1 1234
#define MAXDATASIZE 256
#define BACKLOG 5               //listen队列中等待的连接数  

void *thrd_func1(void *arg);
void *thrd_func2(void *arg);
pthread_t tid1,tid2,tid3;
int cfd;
struct sockaddr_in s_add,c_add;

typedef struct _CLIENT  
{  
    int fd;                     //客户端socket描述符   
    char name[20];              //客户端名称   
    struct sockaddr_in addr;    //客户端地址信息结构体   
    char data[MAXDATASIZE];     //客户端私有数据指针   
} CLIENT;
/**************************************************************************



****************************************************************************/



int tcpClient()
{
     
     int recbytes;
     int sin_size;

     char buffer[1024]={0}; 
     char buf[80];
     char ip[30];
 
     unsigned char buf1[4]={0xFe,0x00,0x03,0x04} ;
     
     unsigned short portnumClient=9250; 


     cfd = socket(AF_INET, SOCK_STREAM, 0);
     if(-1 == cfd)
     {
          printf("socket fail ! \r\n");
          return -1;
     } 
     printf("create clint socket ok !\r\n");

     bzero(&s_add,sizeof(struct sockaddr_in));
     s_add.sin_family=AF_INET;
     sprintf(ip,"%d.%d.%d.%d",192,168,1,104);
     s_add.sin_addr.s_addr= inet_addr(ip);//("192.168.1.109");
     s_add.sin_port=htons(portnumClient);
     printf("s_addr = %#x ,port : %#x\r\n",s_add.sin_addr.s_addr,s_add.sin_port);

    if(-1 == connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
    {
        printf("connect fail !\r\n");
        return -1;
    }
    printf("connect ok !\r\n");

    sendto(cfd,buf1,4,0,(struct sockaddr *)&s_add,sizeof(s_add)); 
    sleep(1);

   // close(cfd);
    return 0;
} 
void *tcp_server()
{
  int i, maxi, maxfd, sockfd;  
    int nready;  
    ssize_t n;  
    fd_set rset, allset;        //select所需的文件描述符集合   
    int listenfd, connectfd;    //socket文件描述符   
    struct sockaddr_in server;  //服务器地址信息结构体   
  
    CLIENT client[FD_SETSIZE];  //FD_SETSIZE为select函数支持的最大描述符个数   
    unsigned char message[16];  //缓冲区   
    int sin_size;               //地址信息结构体大小  

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction( SIGPIPE, &sa, 0 );

 
  printf("satrt working !FD_SETSIZE=%d \r\n",FD_SETSIZE);
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)  
    {                           //调用socket创建用于监听客户端的socket   
        printf("Creating socket failed.");  
        exit(1);  
    }
    else  
       printf("socket success ! \r\n");
    int opt = SO_REUSEADDR;  
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));  //设置socket属性   
  
    bzero(&server, sizeof(server));  
    server.sin_family = AF_INET;  
    server.sin_port = htons(PORT1);  
    server.sin_addr.s_addr = htonl(INADDR_ANY);  
  
    if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)  
    {                           //调用bind绑定地址   
        printf("Bind error.");  
        exit(1);  
    }  
    else
    {
        printf("bind success ! \r\n");
    }
  
    if (listen(listenfd, BACKLOG) == -1)  
    {                           //调用listen开始监听   
        printf("listen() error\n");  
        exit(1);  
    }  
    else
    {
        printf("listen now!  ! \r\n");
    }
    //初始化select   
    maxfd = listenfd;  
    maxi = -1;  
    for (i = 0; i < FD_SETSIZE; i++)  
    {  
        client[i].fd = -1;  
    }  
    FD_ZERO(&allset);           //清空   
    FD_SET(listenfd, &allset);  //将监听socket加入select检测的描述符集合   
  
     

    while (1)  
    {  
        struct sockaddr_in addr;  
        rset = allset;  
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);    //调用select   
        printf("Select() break and the return num is %d. \n", nready);  
  
        if (FD_ISSET(listenfd, &rset))  
        {                       //检测是否有新客户端请求   
            printf("Accept a connection.\n");  
            //调用accept，返回服务器与客户端连接的socket描述符   
            sin_size = sizeof(struct sockaddr_in);  
            if ((connectfd =  
                 accept(listenfd, (struct sockaddr *)&addr, (socklen_t *) & sin_size)) == -1)  
            {  
                printf("Accept() error\n");  
                continue;  
            }  
  
            //将新客户端的加入数组   
            for (i = 0; i < FD_SETSIZE; i++)  
            {  
                if (client[i].fd < 0)  
                {  
                    char buffer[20];  
                    client[i].fd = connectfd;   //保存客户端描述符   
                    memset(buffer, '0', sizeof(buffer));  
                    sprintf(buffer, "Client[%.2d]", i);  
                    memcpy(client[i].name, buffer, strlen(buffer));  
                    client[i].addr = addr;  
                    memset(buffer, '0', sizeof(buffer));  
                    sprintf(buffer, "Only For Test!");  
                    memcpy(client[i].data, buffer, strlen(buffer));  
                    printf("You got a connection from %s:%d.\n", inet_ntoa(client[i].addr.sin_addr),ntohs(client[i].addr.sin_port));  
                    printf("Add a new connection:%s\n",client[i].name);  
                    break;  
                }  
            }  
              
            if (i == FD_SETSIZE)  
                printf("Too many clients\n");  
            FD_SET(connectfd, &allset); //将新socket连接放入select监听集合   
            if (connectfd > maxfd)  
                maxfd = connectfd;  //确认maxfd是最大描述符   
            if (i > maxi)       //数组最大元素值   
                maxi = i;  
            if (--nready <= 0)  
                continue;       //如果没有新客户端连接，继续循环   
        }  
  
        for (i = 0; i <= maxi; i++)  
        {  
            if ((sockfd = client[i].fd) < 0)    //如果客户端描述符小于0，则没有客户端连接，检测下一个   
                continue;  
            // 有客户连接，检测是否有数据   
            if (FD_ISSET(sockfd, &rset))  
            {  
                printf("Receive from connect fd[%d].\n", i);  
                if ((n = recv(sockfd, message,sizeof(message), 0)) == 0)  
                {               //从客户端socket读数据，等于0表示网络中断   
                    close(sockfd);  //关闭socket连接   
                    printf("%s closed. User's data: %s\n", client[i].name, client[i].data);  
                    FD_CLR(sockfd, &allset);    //从监听集合中删除此socket连接   
                    client[i].fd = -1;  //数组元素设初始值，表示没客户端连接   
                }  
                else 
                { 
                   if(sendto(cfd,"test ok",7,0,(struct sockaddr *)&s_add,sizeof(s_add))<0)
                    {
			printf("connect is out, re-connect now!");
			tcpClient();
                     } 
                    
                 } 
                if (--nready <= 0)  
                    break;      //如果没有新客户端有数据，跳出for循环回到while循环   
            }  
        }  
    }  

    pthread_exit((void *)0);
}
void *udp_server()
{
    int socket_fd;
    struct sockaddr_in my_addr,user_addr;
    char buf[MAXDATASIZE]={0xFE,0x00,0x01,0x00,0x01,0x03,0x00,0xff};
    int so_broadcast=1;
    socklen_t size;
    char my_ip[12];

    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(PORT);
    my_addr.sin_addr.s_addr=inet_addr("255.255.255.255");
    bzero(&(my_addr.sin_zero),8);
    
    user_addr.sin_family=AF_INET;
    user_addr.sin_port=htons(PORT);
    user_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    bzero(&(user_addr.sin_zero),8);
    if((socket_fd=(socket(AF_INET,SOCK_DGRAM,0)))==-1) {
        perror("socket");
        exit(1);
    }
   setsockopt(socket_fd,SOL_SOCKET,SO_BROADCAST,&so_broadcast,sizeof(so_broadcast));
    if((bind(socket_fd,(struct sockaddr *)&user_addr,
                        sizeof(struct sockaddr)))==-1) {
        perror("bind");
        exit(1);
    }
    //strcpy(buf,"Hello,I'm on line!");
    sendto(socket_fd,buf,8,0,(struct sockaddr *)&my_addr,sizeof(my_addr));
printf("send ok!\n");
    size=sizeof(user_addr);
    
    while(1) {
sendto(socket_fd,buf,8,0,(struct sockaddr *)&my_addr,sizeof(my_addr));
       sleep(1); 
       
    }
    pthread_exit((void *)0);
}
int main()
{
    tcpClient();
    if (pthread_create(&tid1,NULL,udp_server,NULL)!=0) {
         printf("Create thread 1 error!\n");
         exit(1);
     }
     
     if (pthread_create(&tid2,NULL,tcp_server,NULL)!=0) {
         printf("Create thread 2 error!\n");
         exit(1);
     }
if (pthread_create(&tid3,NULL,zigbeeSerial,NULL)!=0) {
         printf("Create thread zigbeeSerial error!\n");
         exit(1);
     }
    if (pthread_join(tid2,NULL)!=0){
        printf("Join thread 1 error!\n");
        exit(1);
    }

}
