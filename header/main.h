#ifndef HEADERFILE_H
#define HEADERFILE_H

#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <poll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#define queue_size 50

unsigned int Debug_level=1;

typedef struct ThreadData
{
        int                     sockfd;
        bool                    success;
        struct sockaddr_in      *client;
}ThreadData;


typedef struct config_datafile
{
        unsigned int    eservclnport;
        int             instances;
        char            primaryServerIP[256];
        unsigned int    primaryServerPort;
	unsigned int 	debug_level; 
}config_data;







#endif

#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

typedef struct _dictionary_ {
        int                             n ;             /** Number of entries in dictionary */
        int                             size ;  /** Storage size */
        char            **      val ;   /** List of string values */
        char            **  key ;       /** List of string keys */
        unsigned         *      hash ;  /** List of hash values for keys */
} dictionary ;

unsigned dictionary_hash(char * key);
dictionary * dictionary_new(int size);

char * dictionary_get(dictionary * d, char * key, char * def);
char dictionary_getchar(dictionary * d, char * key, char def) ;

int dictionary_getint(dictionary * d, char * key, int def);
double dictionary_getdouble(dictionary * d, char * key, double def);

void dictionary_del(dictionary * vd);
void dictionary_set(dictionary * vd, char * key, char * val);
void dictionary_unset(dictionary * d, char * key);
void dictionary_setint(dictionary * d, char * key, int val);
void dictionary_setdouble(dictionary * d, char * key, double val);
void dictionary_dump(dictionary * d, FILE * out);

#endif

#ifndef _INIPARSER_H_ 
#define _INIPARSER_H_ 

void iniparser_dump_ini(dictionary * d, FILE * f);
void iniparser_dump(dictionary * d, FILE * f);
void iniparser_unset(dictionary * ini, char * entry);
void iniparser_freedict(dictionary * d);

int iniparser_getnsec(dictionary * d);
int iniparser_getboolean(dictionary * d, const char * key, int notfound);
int iniparser_setstr(dictionary * ini, char * entry, char * val);
int iniparser_getint(dictionary * d, const char * key, int notfound);
int iniparser_find_entry(dictionary * ini, char * entry) ;

char * iniparser_getsecname(dictionary * d, int n);
char * iniparser_getstr(dictionary * d, const char * key);
char * iniparser_getstring(dictionary * d, const char * key, char * def);

double iniparser_getdouble(dictionary * d, char * key, double notfound);
dictionary * iniparser_load(const char * ininame);

#endif                         
