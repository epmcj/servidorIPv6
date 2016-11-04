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

// ENVIAR ERRO COM VALOR ABSOLUTO ERRO = -E_POINRTER;

typedef struct{
    int c_socket;       // Socket do cliente
    char* dir_name;     // Nome do diretorio a ser utilizado
    int buffer_size;    // Tamanho do buffer de envio
} thread_arg, *ptr_thread_arg;

void* HandleCall(void* arg);

int main(int argc, char *argv[]){
    // Atributos principais que serao passados por parametro:
    in_port_t s_port;
    int buffer_size;
    char *dir_name;

    DIR *dir;
    int s_socket, c_socket;
    struct sockaddr_in6 s_addr;
    struct sockaddr_storage c_addr;
    socklen_t addr_size;

    // Para threads:
    int sig;
    int i;
    thread_arg argument;
    pthread_t* thread;//[MAX_THREADS];
    //thread_arg args[MAX_THREADS]; 

    // Verificacao do numero de argumentos: PORTA | TAM_BUFFER | DIRETORIO
    if(argc < 4){
        print_error(E_BADARGS);
        return 1;
    }

    s_port      = atoi(argv[1]);
    buffer_size  = atoi(argv[2]);
    dir_name    = argv[3];

    // Verifica se o diretorio existe
    dir = opendir(dir_name);
    if(dir == NULL){
        print_error(E_BADARGS);
        return 1;

    } else
        closedir(dir);

    // Criando o socket
    if((s_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) < 0){
        print_error(E_SOCKET);
        return 1;
    }

    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin6_family = AF_INET6;
    s_addr.sin6_addr   = in6addr_any;
    s_addr.sin6_port   = htons(s_port);
    

    // Realiza o bind
    if(bind(s_socket, (struct sockaddr *) &s_addr, sizeof(s_addr)) < 0){
        print_error(E_BIND);
        return 1;
    }

    // Marcando o socket para escutar por requisicoes de conexoes.
    if(listen(s_socket, MAX_CON) < 0){
        print_error(E_LISTEN);
        return 1;
    }

    //i = 0; <- # limitados de threads
    while(1){
        addr_size = sizeof(c_addr);
        // Espera ate que um cliente conecte.
        c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &addr_size);
        if(c_socket < 0){
            print_error(E_ACCEPT);
            return 1;
        }

	printf("Connected\n");

        // Criando thread para atender a um cliente.
        thread = (pthread_t*)malloc(sizeof(pthread_t));
        if(thread == NULL){
            print_error(E_BASIC);
            return 1;
        }

        argument.c_socket = c_socket;
        argument.dir_name = dir_name;
        argument.buffer_size = buffer_size;

        sig = pthread_create(thread, NULL, HandleCall, &argument);
        if(sig != 0){
            print_error(E_BASIC);
            return 1;
        }

        /*if(i < MAX_THREADS){
            args[i].c_socket = c_socket;
            args[i].dir_name = dir_name;
            args[i].buffer_size = buffer_size;

            sig = pthread_create(&(thread[i]), NULL, HandleCall, &(args[i]));
            if(sig != 0)
                error(E_BASIC);

            i++;
        }*/
    }

    closedir(dir);
    return 0;
}


void* HandleCall(void* arg){
    ptr_thread_arg argm = (ptr_thread_arg) arg;
    char msg[MSG_SIZE];
    DIR *dir;
    char pathname[PATH_SIZE]; 
    int error;

    if(get_client_msg(argm->c_socket, &msg[0], argm->buffer_size) < 0){
        print_error(E_COMMUNICATE);
        close(argm->c_socket);
        pthread_exit(NULL);
    }

    switch(msg[0]){
        case 'L':   // Comando 'list'
            dir = opendir(argm->dir_name);
            if(dir == NULL){
                if(send_error(E_POINTER, argm->c_socket, argm->buffer_size) < 0)
                    print_error(E_COMMUNICATE);

            } else{
printf("LIST COMMAND.\n");
                if(send_list(argm->c_socket, dir, argm->buffer_size) < 0)
                    print_error(E_COMMUNICATE);

                close(dir);
            }
            break;

        case 'G':   // Comando 'get'
            sprintf(pathname, "%s%s", argm->dir_name, &msg[1]);
printf("GET COMMAND. %s\n", pathname);
            error = send_file(argm->c_socket, argm->buffer_size, pathname);
            if(error < 0 && error != E_COMMUNICATE){
                if(send_error(error, argm->c_socket, argm->buffer_size) < 0)
                    error = E_COMMUNICATE;
            }

            // Se não conseguir comunicar o erro, deve mostrar.
            if(error == E_COMMUNICATE)
                print_error(E_COMMUNICATE);

            break;

        default:
            if(send_error(E_CMD, argm->c_socket, argm->buffer_size) < 0)
                print_error(E_COMMUNICATE);

    }

    close(argm->c_socket);
    pthread_exit(NULL);
}
