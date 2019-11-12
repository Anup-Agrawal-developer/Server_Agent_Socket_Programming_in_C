.PHONY : all clean

all:  intermediate_server

intermediate_server : intermediate_server.c	
	gcc -g -Iheader -o intermediate_server intermediate_server.c -lpthread -Liniparser -liniparser

clean : 
	@echo "cleaning data"
	rm intermediate_server
