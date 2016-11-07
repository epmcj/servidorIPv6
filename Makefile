all: bin/server_FTP.c bin/client_FTP.c bin/server_side.c
		gcc -pthread -o servidorFTP bin/server_FTP.c bin/server_side.c
		gcc -o clienteFTP bin/client_FTP.c

clean: 
		rm -f servidorFTP 
		rm -f clienteFTP
