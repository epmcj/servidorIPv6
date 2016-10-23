all: server_TP1.c common.c
		gcc -pthread -o servidorFTP server_TP1.c common.c

clean: 
		rm -f servidorFTP 