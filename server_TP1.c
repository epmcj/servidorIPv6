#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "common.h"

typedef struct{
    int c_socket;       // Socket do cliente
    char* dir_name;     // Nome do diretorio a ser utilizado
    int buffer_size;    // Tamanho do buffer de envio
} thread_arg, *ptr_thread_arg;

void* HandleCall(void* arg);

void HandleClient(int c_socket, char* dir_name, int buffer_size);


int main(int argc, char *argv[]){
    // Atributos principais que serao passados por parametro:
    in_port_t s_port;
    int buffer_size;
    char *dir_name;

    DIR *dir;
    int s_socket, c_socket;
    struct sockaddr_in s_addr; // IPv4
    //struct sockaddr_in6 s_addr; IPv6 <----------
    struct sockaddr_storage c_addr;
    socklen_t addr_size;

    // Para threads:
    int sig;
    int i;
    thread_arg argument;
    pthread_t* thread;//[MAX_THREADS];
    //thread_arg args[MAX_THREADS]; 

    // Verificacao do numero de argumentos: PORTA | TAM_BUFFER | DIRETORIO
    if(argc < 4)
        error(E_BADARGS);

    s_port      = atoi(argv[1]);
    buffer_size  = atoi(argv[2]);
    dir_name    = argv[3];

    // Verifica se o diretorio existe
    dir = opendir(dir_name);
    if(dir == NULL)
        error(E_BADARGS);
    else
        closedir(dir);

    // Criando o socket
    if((s_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) //---ALTERAR PARA AF_INET6
        error(E_SOCKET);

    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET; 
    s_addr.sin_addr.s_addr = INADDR_ANY;
    s_addr.sin_port = htons(s_port);
    /*
    s_addr.sin6_family = AF_INET6
    s_addr.sin6_addr   = in6addr_any;
    s_addr.sin6_port   = htons(s_port);
    */

    // Realiza o bind
    if(bind(s_socket, (struct sockaddr *) &s_addr, sizeof(s_addr)) < 0)
        error(E_BIND);

    // Marcando o socket para escutar por requisicoes de conexoes.
    if(listen(s_socket, MAX_CON) < 0)
        error(E_LISTEN);


    i = 0;
    while(1){
        addr_size = sizeof(c_addr);
        // Espera ate que um cliente conecte.
        c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &addr_size);
        if(c_socket < 0)
            error(E_ACCEPT);


        // Criando thread para atender a um cliente.
        thread = (pthread_t*)malloc(sizeof(pthread_t));
        if(thread == NULL)
            error(E_BASIC);

        argument.c_socket = c_socket;
        argument.dir_name = dir_name;
        argument.buffer_size = buffer_size;

        sig = pthread_create(thread, NULL, HandleCall, &argument);
        if(sig != 0)
            error(E_BASIC);

        /*if(i < MAX_THREADS){
            args[i].c_socket = c_socket;
            args[i].dir_name = dir_name;
            args[i].buffer_size = buffer_size;

            sig = pthread_create(&(thread[i]), NULL, HandleCall, &(args[i]));
            if(sig != 0)
                error(E_BASIC);

            i++;
        }*/
        //HandleClient(c_socket, dir_name, buffer_size);
    }

    closedir(dir);
    return 0;
}

void HandleClient(int c_socket, char* dir_name, int buffer_size){
    char msg[MSG_SIZE];
    ssize_t nBytesRcvd, nBytesSent; 
    DIR *dir;
    char pathname[PATH_SIZE]; 


    if(get_client_msg(c_socket, &msg[0], buffer_size) < 0)
        error(E_COMMUNICATE); // OLHAR ESSE ERRO <<<<<<<<<<<<<

    switch(msg[0]){
        case 'L':   // Comando 'list'
            dir = opendir(dir_name);
            if(dir == NULL){
                msg[0] = 'E';    // Mensagem de erro.
                msg[1] = E_POINTER;
                msg[2] = '\0';
                if(sendTo(c_socket, msg, 3, buffer_size) < 0)
                    error(E_COMMUNICATE);
            } else{
                printf("LIST COMMAND.\n");
                if(send_list(c_socket, dir, buffer_size) < 0)
                    error(E_COMMUNICATE);
            }
            break;

        case 'G':   // Comando 'get'
            sprintf(pathname, "%s/%s", dir_name, &msg[1]);
            printf("GET COMMAND. %s\n", pathname);
            send_file(c_socket, buffer_size, pathname);
            break;

        default:
            error(E_CMD);
    }

    close(dir);
    close(c_socket);
}


void* HandleCall(void* arg){
    ptr_thread_arg argm = (ptr_thread_arg) arg;
    char msg[MSG_SIZE];
    ssize_t nBytesRcvd, nBytesSent; 
    DIR *dir;
    char pathname[PATH_SIZE]; 


    if(get_client_msg(argm->c_socket, &msg[0], argm->buffer_size) < 0)
        error(E_COMMUNICATE); // OLHAR ESSE ERRO <<<<<<<<<<<<<

    switch(msg[0]){
        case 'L':   // Comando 'list'
            dir = opendir(argm->dir_name);
            if(dir == NULL){
                msg[0] = 'E';    // Mensagem de erro.
                msg[1] = E_POINTER;
                msg[2] = '\0';
                if(sendTo(argm->c_socket, msg, 3, argm->buffer_size) < 0)
                    error(E_COMMUNICATE);
            } else{
                printf("LIST COMMAND.\n");
                if(send_list(argm->c_socket, dir, argm->buffer_size) < 0)
                    error(E_COMMUNICATE);

                close(dir);
            }

            break;

        case 'G':   // Comando 'get'
            sprintf(pathname, "%s/%s", argm->dir_name, &msg[1]);
            printf("GET COMMAND. %s\n", pathname);
            send_file(argm->c_socket, argm->buffer_size, pathname);
            break;

        default:
            error(E_CMD);
    }

    close(argm->c_socket);
    pthread_exit(NULL);
}