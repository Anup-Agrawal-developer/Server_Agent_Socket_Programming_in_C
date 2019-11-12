#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
#include <stdbool.h>
#include <sys/resource.h>

#define MINBUFF                         256
#define MAXBUFF                         4096

bool            TermExitSignal          = false;
pthread_mutex_t mutexLinkedList ;

typedef struct ThreadData
{
        pthread_t               threadid;
        int                     sockfd;
        bool                    success;
        struct sockaddr_in      *client;
}ThreadData;

void signal_handler(int signum)
{
	printf("terminate\n");
	 TermExitSignal = true;

}

int setTimeoutOnSocket( int sockDesc )
{
        if( sockDesc < 0 )
	{
                return -1;
	}
        fd_set set;
        struct timeval timeout;

        // clear the set 
        FD_ZERO(&set); 
        // add our file descriptor to the set 
        FD_SET(sockDesc, &set);
        timeout.tv_sec = 30;
        timeout.tv_usec = 0;

        int rv = select(sockDesc + 1, &set, NULL, NULL, &timeout);
        if(rv == -1)
        {
                // an error accured 
                printf( "Faliled on select(socket). ErrorNo= %d Error= %s\n", errno, strerror(errno) );
                return -1;
        }
        else if(rv == 0)
        {
                // a timeout occured 
                printf( "Timeout[30-seconds] while reading response from eserv.\n");
                return 1;
        }
        else
        {
                // data is available on server
                return 0;
        }


        return -1;
}

void* ThreadHandle(void *TData)
{
        if( TData == NULL )
	{
                return NULL;
	}
        ThreadData *ThreadHandleData = (ThreadData*)TData;
        int sockfd = -1;
        char Hellostring[MINBUFF] = "";
        char hostname[MINBUFF]="";
        char socketData[MAXBUFF]="";
        int ret, writeFileFlag = 0;

        if(ThreadHandleData == NULL)
	{
                return NULL;
	}
        ThreadHandleData->success = true;


        while(!TermExitSignal)
        {
                if((sockfd = ThreadHandleData->sockfd) < 0)
                {
                        usleep(200);
                        continue;
                }
                sprintf(Hellostring, "Welcome [IP : %s] Server 1.0.0.4\r\n", inet_ntoa(ThreadHandleData->client->sin_addr));


                if((ret = write(sockfd, Hellostring, strlen(Hellostring))) < 0)
                {
                        printf( "connect to eserv is not successful\n");
			printf("%d\n",sockfd);
                        close(sockfd);
                        ThreadHandleData->sockfd = -1;
                        sockfd = -1;
                }
                //Set time out for read client response
                if(setTimeoutOnSocket(sockfd) != 0)
                {
                        close(sockfd);
  			ThreadHandleData->sockfd = -1;
                }

                bzero(socketData, MAXBUFF);
                int n = read(sockfd, socketData, MAXBUFF);
                if(n > 0)
                {
                        if(strlen(socketData) <= 0 )
                        {
                                write(sockfd, "250 Line Not Accepted.\r\n", 25) ;
                                close(sockfd);
                                ThreadHandleData->sockfd = -1;
                                continue;
                        }

                        if((ret = write(sockfd, "250 Line Accepted.\r\n", 21)) < 0)
                        {
                                printf( "Failed to send Acknowledge event accept\n");
                        }
                }

                close(sockfd);
                ThreadHandleData->sockfd = -1;
                sockfd = -1;

        }

        return ThreadHandleData;
}

int main ()
{
        struct sockaddr_in serv_addr, cli_addr;
        int server_sock, portno, sockfd;
        int ac = 0, ind, errno;
        ThreadData *threaddata = NULL;
        pthread_t *threadid =NULL ;
 
	//create socket
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
        if(server_sock == -1)
        {
                printf("Error in open eservcln socket%s\n",strerror(errno));
                exit(1);
        }

        fcntl(server_sock, F_SETFD, FD_CLOEXEC);

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(2225);

        int turniton = 1;
        if ( setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &turniton, sizeof(turniton)) < 0 )
        {
                printf("ERROR setting reuse option:%s\n", strerror(errno));
                exit(1);
        }

        //Bind eservcln server socket to tcp connection
        if(bind( server_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        {
                printf("Can not bind eservcln socket%s\n",strerror(errno));
                exit(1);
        }

        //listen for incoming connection and keep in network Queue
        if( listen(server_sock,500) == -1)
        {
                printf("Listen error on eservcln: %s.\n", strerror(errno));
                exit(1);
        }
        fcntl(server_sock, F_SETFL, O_NONBLOCK);




        if (pthread_mutex_init(&mutexLinkedList, NULL) != 0)
        {
                printf("event list locking initilization error: %s.\n", strerror(errno));
                exit(1);
        }



 	//Allocate memory for thread handle data        
        threaddata = (ThreadData*) malloc(sizeof(ThreadData) * 10);
        if(threaddata == NULL)
        {
                printf("MemAllocErr(malloc). Memory not sufficient.\n");
                exit(1);
        }

        //Allocate memory for storing Threadid
        threadid = (pthread_t *) malloc(sizeof(pthread_t) *10);
        if(threadid == NULL)
        {
                printf("MemAllocErr(malloc). Memory not sufficient.\n");
                exit(1);
        }


	signal(SIGINT,signal_handler);
 //create threads for process the client request
        for(ind = 0; ind < 10 ; ind++)
        {
                //Initialize Thread Data
                threaddata[ind].threadid = ac;
                threaddata[ind].sockfd = -1;
                threaddata[ind].success = false;
                threaddata[ind].client = NULL;
                if(pthread_create(&threadid[ind], NULL, ThreadHandle, &threaddata[ind]) < 0)
                {
                        TermExitSignal = true;
                        break;
                }
        }
	int t = sizeof(cli_addr);
        while(!TermExitSignal)
        {
                //Accept request from network stack
                if ((sockfd = accept(server_sock, (struct sockaddr *)&cli_addr, (socklen_t *)&t)) < 0)
                {
                        // accept will return EAGAIN if there is no connection to accept
                        if(errno == EAGAIN)
                        {
                                usleep(500);
                                continue;
                        }
                        printf("Request accept error..: %s.\n", strerror(errno));
                        break;
                }
                //Assign accepted request to free thread        
                while(1)
                {
                        for(ind = 0; ind < 10; ind++)
                        {
                                printf("Matching thread instance %d (sockFd = %d, success = %d).\n", ind, threaddata[ind].sockfd, threaddata[ind].success);
                                if(threaddata[ind].sockfd < 0 && threaddata[ind].success)
                                {
                                        printf("Thread instance %d free to process.\n", ind);
                                        ac++;
                                        threaddata[ind].sockfd = sockfd;
                                        threaddata[ind].client = &cli_addr;
                                        break;
                                }
                        }
                        if(ind < 10)
                                break;
                        printf("No thread instance free to process, waiting to release one.\n");
                        usleep(200);
                }
        }

        for(ind = 0; ind < 10; ind++)
                pthread_join(threadid[ind],NULL);


        close(server_sock);
        close(sockfd);


	return 0;
}
