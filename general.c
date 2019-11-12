

//convert running process into daemon
int intermediate_server_daemon(void)
{

        FILE *fp1=fopen("/var/log/intermediate_server.pid","a+");
        pid_t pid;



        pid=fork();

        if(pid<0)
                exit(0);

        if(pid>0)
        {
                printf("process id of child process :- %d\n",pid);
                exit(0);
        }


        if(setsid()<0)
                printf("setsid");


        pid=getpid();
        umask(0);

        fprintf(fp1,"%d",pid);
        close(0);
        close(1);
        close(2);

        fclose(fp1);


}

//release dynamically allocated memory
void releaseDynamicAllocatedMemory(void * mptr)
{
        if(mptr)
        {
                free(mptr);
                mptr = NULL;
        }
}
