
//func calls for reading config file
int read_config_file(config_data *conf, char *buff)
{
        unsigned int eservclnport,eservport,debug_level;
        char *IP = malloc(sizeof(IP));
        int max_inst=50,min_inst=5,def_inst=10,instances=0;
        dictionary * ini ;
 

        ini = iniparser_load("config.ini");
        if(ini==NULL)
	{
		strcpy(buff,"can't open config file\n");
                return -1;
	}

        debug_level=iniparser_getint(ini,"Debug:Debug_level",0);
        if(debug_level==0)
                debug_level=1;
	
	conf->debug_level = debug_level;
	
        eservclnport = iniparser_getint(ini,"eservcln:eservclnport",0);
        if(eservclnport==0)
	{
		strcpy(buff,"can't find eservclnport\n");	
                return -1; 
        }
       
        conf->eservclnport = eservclnport;

        instances = iniparser_getint(ini,"eservcln:instances",0);
        if(instances>max_inst||instances<min_inst)
                conf->instances=def_inst;
        else    
                conf->instances=instances;
                
        eservport =iniparser_getint(ini,"eserv:eservport",0);
        if(eservport==0)
	{	
		strcpy(buff,"can't find eservpport\n");
                return -1;
        }
       
        conf->primaryServerPort = eservport;
        IP = iniparser_getstring(ini,"eserv:eservip",NULL);
	if(IP==NULL)
	{ 
		strcpy(buff,"can't find eservip\n");
       		return -1; 
	}       
        strncpy(conf->primaryServerIP,IP,sizeof(conf->primaryServerIP));

        return 0;
}
