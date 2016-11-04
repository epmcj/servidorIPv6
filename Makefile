all: server_TP1.c common.c client_TP1.c
		gcc -pthread -o servidorFTP server_TP1.c common.c
		gcc -o clienteFTP client_TP1.c

clean: 
		rm -f servidorFTP 
		rm -f clienteFTP
