
//fucn displays current time and date 
char *time_info(char *buffer,int length)
{
   time_t rawtime;
   struct tm *info;

   time( &rawtime );
   info = localtime( &rawtime );
   strftime(buffer,length+1,"[%d-%b-%Y@%X]", info);
   return buffer;
}


//write messages in server.log file
void writelog(int level, char *log_message)
{
	int time_length=1024;
	char time_buffer[time_length];


	FILE *logfile = fopen("server.log","a+");
	bzero(time_buffer,time_length);
	if(Debug_level>=level)
        {
                time_info(time_buffer,time_length);
                fprintf(logfile,"%s",time_buffer);
                fprintf(logfile,"%s",log_message);
        }
	fclose(logfile);
}

