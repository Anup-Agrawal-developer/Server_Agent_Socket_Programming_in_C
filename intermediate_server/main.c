#include "main.h"
#include "config.c"
#include "general.c"
#include "write_log.c"

#define EVENT_CACHE                     "client_data.cache"
#define PID_FILE			"/var/run/intermediate_server.pid"

#define MAXBUFF 2048
#define MINBUFF 256

pthread_mutex_t mutexLinkedList;

int rear =0;
int front=0;
int Term_exit_flag=0;
char arr_queue[queue_size][1024];

// remaining events are stored into cache from queue
int write_eventinfile()
{
	char queue_nullbuff[MINBUFF]="";

	if(arr_queue[front]==NULL)
		return 0;

	//storing event in file from queue
	FILE *client_data=fopen(EVENT_CACHE,"a+");
	while(rear!= front)
	{	
		fprintf(client_data,"%s",arr_queue[front]);
	
		//deleting event from queue
		bzero(queue_nullbuff,sizeof(queue_nullbuff));
		strcpy(arr_queue[front],queue_nullbuff);
		front++;
	}
	fclose(client_data);
	writelog(4,"events stored successfully\n");
	return 1;
}

//func call if interupt the process
void signal_handler(int signum)
{
	Term_exit_flag =1;
        writelog(3,"term signal detected\n");
}


//set timeout of 30sec for reading from client connections
int setTimeoutOnSocket( int sockDesc )
{
        if( sockDesc < 0 )
        {
                return -1;
        }
        fd_set set;
        struct timeval timeout;

        /* clear the set */
        FD_ZERO(&set); 
        /* add our file descriptor to the set */
        FD_SET(sockDesc, &set);
        timeout.tv_sec=30;
        timeout.tv_usec = 0;

        int rv = select(sockDesc + 1, &set, NULL, NULL, &timeout);
        if(rv == -1)
        {
                /* an error accured */
                writelog(1, "Faliled on select(socket)\n");
                return -1;
        }
        else if(rv == 0)
        {
                /* a timeout occured */
                writelog(1, "Timeout[30-seconds] while reading response from eserv\n");
                return 1;
        }
        else
        {
                /* data is available on server*/
                return 0;
        }


        return -1;
}
            

//calling thread for communicating with multiple clients
void *ThreadHandle(void *TData)
{	
	if( TData == NULL )
                return NULL;

        ThreadData *ThreadHandleData = (ThreadData*)TData;

	static int client_no=1;
	int in=0,new_socket=-1;
	char Welcome_string[MINBUFF]="";
	char printbuff[MAXBUFF]="";
	char sendbuff_client[MAXBUFF]="";
	char recvbuff_client[MAXBUFF]="";
	char queuebuff[MAXBUFF]="";


	if(ThreadHandleData == NULL)
                return NULL;

        ThreadHandleData->success = true;

 


        while(Term_exit_flag==0)
        {
		if((new_socket = ThreadHandleData->sockfd)<0)
                {
                        usleep(200);
                        continue;
                }


		bzero(Welcome_string,sizeof(Welcome_string));
		sprintf(Welcome_string,"welcome(C :: %d , IP :: %s ) server 192.168.0.5\r\n", client_no,inet_ntoa(ThreadHandleData->client->sin_addr));
		client_no++;
		//sending welcome message to client
		if((in=write(new_socket,Welcome_string,sizeof(Welcome_string)))<0)
		{
			
 			writelog(2,"error in sending welcome msg to client\n");
                        close(new_socket);
                        ThreadHandleData->sockfd = -1;
                        client_no--;
                        continue;
		}

		//Set time out for read client response
  	        if(setTimeoutOnSocket(new_socket) != 0)
	        {
                        close(new_socket);
                        client_no--;
                        ThreadHandleData->sockfd = -1;
			new_socket=-1;
			continue;			
                }

		
		//reading event from client
		bzero(recvbuff_client,sizeof(recvbuff_client));
		if((in=read(new_socket,recvbuff_client,sizeof(recvbuff_client)))<0)
		{
			write(new_socket,"250 Line not Accepted.\r\n",25);
			close(new_socket);
                        ThreadHandleData->sockfd = -1;
			new_socket=-1;
			client_no--;
			continue;
		}
		else
		{
			//event got from client	
			bzero(printbuff,sizeof(printbuff));
			snprintf(printbuff,sizeof(printbuff),"msg got from client  %s is :: %s\n",inet_ntoa(ThreadHandleData->client->sin_addr),recvbuff_client);

			writelog(2,printbuff);
			strcpy(queuebuff,printbuff);	

			//sending acknowledgment to client
			if((in=write(new_socket,"250 Line Accepted.\r\n",21))<0)
			{
				writelog(2,"writing to client is failed\n");
			}

			//storing event in queue
			if(rear!= (queue_size-1))
			{
				strcpy(arr_queue[rear],queuebuff);
				rear++;
			}
			else 
			{
				strcpy(arr_queue[rear],queuebuff);
				rear++;
				//storing event in file	
				int retValWrite =  write_eventinfile();
				if(retValWrite < 0)
        			{
                			writelog(2,"Error opening event cache file. Failed to save received events.\n");
        			}
        			else if(retValWrite == 0)
        			{
                			writelog(2,"Event list is empty. No need to save cache.\n");
        			}
	
				rear=0;	
				front=0;
			}
		}
		
		close(new_socket);
		client_no--;
		ThreadHandleData->sockfd = -1;
		new_socket = -1;

	}
	return ThreadHandleData;
	
}


//connect to primary server
int connect_to_PrimaryServer()
{
	config_data conf_data;
	char config_error_data[1024]="";

	//reading config.ini file	
 	if(read_config_file(&conf_data,config_error_data) < 0)
        {
                writelog(1,config_error_data);
                remove(PID_FILE);
                exit(1);
        }

	struct sockaddr_in server1;
	bzero((char *)&server1, sizeof(server1));

	//Create Socket for primary_server
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket<0)
	{
		writelog(2,"Socket for main server not created succesfully\n");
		close(server_socket);
		return -1;
	}

	server1.sin_family = AF_INET;
	server1.sin_addr.s_addr=inet_addr(conf_data.primaryServerIP);
	server1.sin_port=htons(conf_data.primaryServerPort);

	//connecting to the primary_server
	int in1=connect(server_socket, (struct sockaddr *)&server1, sizeof(server1));
	if(in1<0)
	{
		if(errno == EAGAIN || errno == ECONNREFUSED)
		{
			close(server_socket);
			sleep(2);
			return -2;
		}
		writelog(1,"connect error to eserv \n");
		close(server_socket);
		return -1;
	}

	return server_socket;

}

//sending event from queue
int sendEvent_fromqueue()
{
	char sendbuff_server[MAXBUFF]= "";
	char recvbuff_server[MAXBUFF]= "";
	char queue_nullbuff[MINBUFF]="";	
	int server_socket=0;


        while(Term_exit_flag==0)
        {
                server_socket=connect_to_PrimaryServer();
                if(server_socket==-1)
                        return -1;
		
                else if(server_socket==-2)
			continue;
			
		else
			break;
        }
	pthread_mutex_lock(&mutexLinkedList);
	if(front!=rear)
	{
		//if queue data is null then quit
		if(arr_queue[front]==NULL)
		{
			writelog(2,"error in reading from queue\n");
			pthread_mutex_unlock(&mutexLinkedList);
			close(server_socket);
			return -1;
		}

		//getting banner from primary_server
		bzero(recvbuff_server,sizeof(recvbuff_server));
		if(read(server_socket,recvbuff_server,sizeof(recvbuff_server))<0)
		{
			writelog(2,"error in reading from primary_server\n");
			pthread_mutex_unlock(&mutexLinkedList);
			close(server_socket);
			return -1;
		}
		writelog(4,recvbuff_server);
	
		//sending data to primary_server
		bzero(sendbuff_server,sizeof(sendbuff_server));
		strcpy(sendbuff_server,arr_queue[front]);	
		

		if(write(server_socket,sendbuff_server,sizeof(sendbuff_server))<0)
		{
			writelog(1,"error in sending event from queue to primary_server\n");
			pthread_mutex_unlock(&mutexLinkedList);
			close(server_socket);			
			return -1;
		}
	
		if(write(server_socket,"\r\n",2)<0)
		{
			writelog(2,"error in writing to primary_server\n");
			pthread_mutex_unlock(&mutexLinkedList);
			close(server_socket);		
			return -1;	
		}				
		
		//reading from primary_server
		bzero(recvbuff_server,sizeof(recvbuff_server));
		if(read(server_socket,recvbuff_server,sizeof(recvbuff_server))<0)
		{
			writelog(2,"error in reading 250 line accepted from primary_server\n");
			pthread_mutex_unlock(&mutexLinkedList);
			close(server_socket);		
			return -1;
		}
						
		writelog(4,recvbuff_server);
		
		//deleting event from queue	
		bzero(queue_nullbuff,sizeof(queue_nullbuff));
		strcpy(arr_queue[front],queue_nullbuff);
	
	
		front++;
	}
	pthread_mutex_unlock(&mutexLinkedList);
	close(server_socket);
        return 0;
}

//sending event from cache
int sendEvent_fromCache()
{
	char sendbuff_server[MAXBUFF]= "";
	char recvbuff_server[MAXBUFF]= "";
	char data_buff[MAXBUFF]= "";
	int server_socket=0;
	char *getfile = malloc(sizeof(getfile));

	FILE *client_data=fopen(EVENT_CACHE,"a+");
	if(client_data==NULL)
	{
		writelog(2,"cache failed to open\n");
		return -1;
	}


	while(Term_exit_flag==0)
	{
		pthread_mutex_lock(&mutexLinkedList);
		server_socket=connect_to_PrimaryServer();
		if(server_socket==-1)
		{
			pthread_mutex_unlock(&mutexLinkedList);
			return -1;
		}

		else if(server_socket==-2)
		{
			pthread_mutex_unlock(&mutexLinkedList);
			continue;
		}
		
		if((getfile=fgets(data_buff,sizeof(data_buff),client_data))!=NULL)
		{
			
			//getting banner from primary_server
			bzero(recvbuff_server,sizeof(recvbuff_server));
			if(read(server_socket,recvbuff_server,sizeof(recvbuff_server))<0)
			{
				writelog(2,"error in reading welcome from primary_server\n");	
				close(server_socket);
				pthread_mutex_unlock(&mutexLinkedList);
				return -1;
			}
			writelog(4,recvbuff_server);
			
			//sending stored data to primary_server
			if(write(server_socket,data_buff,sizeof(data_buff))<0)
			{
				writelog(1,"error in sending cache to primary_server\n");
				close(server_socket);	
				pthread_mutex_unlock(&mutexLinkedList);
				return -1;
			}
			if(write(server_socket,"\r\n",2)<0)
			{
				writelog(2,"error in writing to primary_server\n");
				close(server_socket);
				pthread_mutex_unlock(&mutexLinkedList);
				return -1;	
			}
			
			//reading from primary server
			bzero(recvbuff_server,sizeof(recvbuff_server));
			if(read(server_socket,recvbuff_server,sizeof(recvbuff_server))<0)
			{
				writelog(2,"error in reading 250 line accepted from primary_server\n");
				close(server_socket);
				pthread_mutex_unlock(&mutexLinkedList);
				return -1;
			}                                   
	  
			writelog(4,recvbuff_server);
			
		}
		if(getfile == NULL)
		{
			remove(EVENT_CACHE);
			fclose(client_data);
        		pthread_mutex_unlock(&mutexLinkedList);
			close(server_socket);
			break;
		}
		close(server_socket);
        	pthread_mutex_unlock(&mutexLinkedList);
		usleep(100);
	}

	return 0;
}





//calling this thread for sending events to primary server
void *HandleQueue(void *cdata)
{
 
        while(Term_exit_flag==0)
        {
       		if(access(EVENT_CACHE, R_OK|F_OK) == 0)
		{	
			int retVal= sendEvent_fromCache();
			if(retVal <0)
			{
				writelog(4,"error in sending event from cache\n");
				continue;		
			}
		}	
		int retVal = sendEvent_fromqueue();
		if(retVal<0)
		{
	                writelog(4,"error in sending event from queue\n");
			continue;
		}
 		else if(retVal == 1)
                        usleep(500);
                else if(retVal == 0)
                        usleep(500);

        }

        return 0;
}



int main()
{
	
	struct sockaddr_in serv_addr, cli_addr;
	int sockfd=0,server_sock;
	char config_error_data[1024]="";
	int ac = 0, ind, errno,opt=1;
        pthread_t queue_thread;
        config_data conf_data;
 	ThreadData *threaddata = NULL;
        pthread_t *threadid =NULL ;


	//reading config.ini file	
 	if(read_config_file(&conf_data,config_error_data) < 0)
        {
                writelog(1,config_error_data);
                exit(1);
        }
	Debug_level = conf_data.debug_level;

	//converting running process into daemon
 	if(intermediate_server_daemon()<0)
	{
		writelog(1,"can't daemonize the process\n");
                exit(1);
	}

	//create socket
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(server_sock == -1)
	{
		writelog(1,"Error in open eservcln socket\n");
		remove(PID_FILE);
		exit(1);
	}

	fcntl(server_sock, F_SETFD, FD_CLOEXEC);

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(conf_data.eservclnport);

	//set for reusing port
	if ( setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 )
	{
		writelog(1,"ERROR setting reuse option\n");
		remove(PID_FILE);
		exit(1);
	}

	//Bind socket
	if(bind( server_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		writelog(1,"Can not bind eservcln socket\n");
		remove(PID_FILE);
		exit(1);
	}

	//listen for incoming connection and keep in network Queue
	if( listen(server_sock,500) == -1)
	{
		writelog(1,"Listen error on eservcln\n");
		remove(PID_FILE);
		exit(1);
	}
	fcntl(server_sock, F_SETFL, O_NONBLOCK);
 
	//Allocate memory for thread handle data        
        threaddata = (ThreadData*) malloc(sizeof(ThreadData) * conf_data.instances);
        if(threaddata == NULL)
        {
		writelog(1,"memory not sufficient\n");
		remove(PID_FILE);
                exit(1);
        }

        //Allocate memory for storing Threadid
        threadid = (pthread_t *) malloc(sizeof(pthread_t) *conf_data.instances);
        if(threadid == NULL)
        {
		writelog(1,"memory not sufficient\n");	
		remove(PID_FILE);
                exit(1);
        }

	//set for initializing mutex
	if (pthread_mutex_init(&mutexLinkedList, NULL) != 0)
	{
		writelog(1,"event list locking initilization error\n");
		exit(1);
	}


	signal(SIGTERM,signal_handler);	
	signal(SIGINT, signal_handler);
 	signal(SIGPIPE, SIG_IGN);

	//create threads for process the client request
	for(ind = 0; ind < conf_data.instances ; ind++)
	{
		//Initialize Thread Data
		threaddata[ind].sockfd = -1;
		threaddata[ind].success = false;
		threaddata[ind].client = NULL;
		if(pthread_create(&threadid[ind], NULL, ThreadHandle,(void *)&threaddata[ind]) < 0)
		{
			Term_exit_flag = 1;
			break;
		}
	}
	sleep(1);

	//create thread for reading Events from event queue
        if(pthread_create(&queue_thread, NULL, HandleQueue,NULL) < 0)
        {
		Term_exit_flag = 1;
        }

	int t = sizeof(cli_addr);
	while(Term_exit_flag==0)
	{
		//Accept request from network stack
		if ((sockfd = accept(server_sock, (struct sockaddr *)&cli_addr, &t)) < 0)
		{
			// accept will return EAGAIN if there is no connection to accept
			if(errno == EAGAIN)
			{
				usleep(500);
				continue;
			}
			writelog(2,"Request accept error\n");
			break;
		}
		//Assign accepted request to free thread        
		while(1)
		{
			for(ind = 0; ind < conf_data.instances; ind++)
			{
//				writelog(3,"Matching thread instance %d (sockFd = %d, success = %d).\n", ind, threaddata[ind].sockfd, threaddata[ind].success);
				if(threaddata[ind].sockfd < 0 && threaddata[ind].success)
				{
//					writelog(3,"Thread instance %d free to process.\n", ind);
					threaddata[ind].sockfd = sockfd;
					threaddata[ind].client = &cli_addr;
					break;
				}
			}
			if(ind < conf_data.instances)
				break;
			writelog(4,"No thread instance free to process, waiting to release one\n");
			usleep(200);
		}
	}


	for(ind = 0; ind < conf_data.instances; ind++)
		pthread_join(threadid[ind],NULL);

	pthread_join(queue_thread,NULL);

	writelog(3,"Threads finish ok\n");

	//storing remaining event in a cache
	int retValFileWrite = write_eventinfile();
        if(retValFileWrite < 0)
        {
                writelog(2,"Error opening event cache file. Failed to save received events.\n");
        }
        else if(retValFileWrite == 0)
        {
                writelog(2,"Event list is empty. No need to save cache.\n");
        }

	releaseDynamicAllocatedMemory((void*)threaddata);
        releaseDynamicAllocatedMemory((void*)threadid);

        pthread_mutex_unlock(&mutexLinkedList);
        remove(PID_FILE);
        close(server_sock);
        close(sockfd);

	return 0;
}

