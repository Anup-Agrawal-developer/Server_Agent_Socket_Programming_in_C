# Intermediate_Server_Socket_Programming_in_C

What is Intermediate_Server?
This is useful for server which having huge number of clients and when server don't have time to handle it . So , then agent does
his work send it to server whenever server wants.

This is one type of module which will accept multiple clients using threadpool and also keeps saving all client data into 
Client_data.cache. Simultaneously, trying to connecting to main_server, whenever server got connnected it will pass all data to that server . 
One special thread created for always sending connection signal to the main_server. 
    
    
 Threadpool is just a concept which is used to creating infinite threads. This concept having a simple logic , if you understand 
 this concept properly. 
 How it works?
 Whenever, you create thread for executing your operation. What we do, we create thread,use that thread and exit from that thread.
 Again create new thread. This will end at sometime , because every term has a limit. So, for that threadpool concept is built.
 the concept is when you complete your thread work then you should reuse that thread instead of exiting from that.Because of this 
 you can use some 10 threads infinite time .
 
 
For Executing this code you should have "pthread" library 

First go in the intermediate server

BEFORE RUNNING ALL PROGRAMS , WRITE YOUR SERVER IP  in client.c program for intermediate_server.where i mentioned
and in config.ini file for main_server's IP.



and do make 
and make will create binary file "intermediate_server" and after that run your main_server and client

for runnning main_server:

    gcc -o main_server main_server.c -lpthread
    
 for running client : 
 
    gcc -o client client.c -lpthread

