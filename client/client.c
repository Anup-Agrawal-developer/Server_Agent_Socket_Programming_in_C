#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define n_th 1

int i=1;
pthread_t tid[n_th];
pthread_mutex_t lock;


void* func(void *args)
{	
	while(1)
	{
		int th,fd = 0,in=0,flag=0;
		char buff[2000];
		printf("p   == %d \n",p);
		p++;
	

		//Setup Buffer Array
		bzero(buff,sizeof(buff));	

		//Create Socket
		fd = socket(AF_INET, SOCK_STREAM, 0);
		if(fd<0)
		{
			perror("Client Error: Socket not created succesfully");
			return 0;
		}

		struct sockaddr_in server; 
		bzero((char *)&server, sizeof(server)); 

		server.sin_family = AF_INET;
		server.sin_addr.s_addr=inet_addr("192.168.2.197");
		server.sin_port=htons(8801);


				
		in = connect(fd, (struct sockaddr *)&server, sizeof(server));
		if(in<0)
		{
				perror("Client Error: Connection Failed.");
				close(fd);
				return 0;
		}

		in=recv(fd,buff,2000,0);
		if(in<0)
			printf("error in receive");

		printf("\n%s\n",buff);
		bzero(buff,2000);	
		snprintf(buff,2000, "i am client%d", i);
			

		printf("\nSending to SERVER: %s \n",buff);

		in = write(fd,buff,2000);
		if (in < 0) 
		{
			perror("\nClient Error: Writing to Server");
			close(fd);
			exit(1);
		}

		bzero(buff,2000);   
		in = read(fd,buff,2000);
		if (in < 0) 
		{
			perror("\nClient Error: Reading from Server");
			close(fd);
			exit(1);
		}
		printf("\nReceived FROM SERVER: %s \n",buff);
		i++;	
		close(fd);
	}
}



int main()
{
        int flag=0,th,i;	
	
	for(i=1;i<=n_th;i++)
	{

		th=pthread_create(&tid[i],NULL,func,NULL);
		if(th!=0)
		{
			printf("thread creation failed");
			break;
		}   	
		
	}
	for(i=1;i<=n_th;i++)
	{	
		pthread_join(tid[i],NULL);
	}
	
	return 0;
}

