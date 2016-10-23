#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>

#define E_BADARGS 		-1
#define E_SOCKET 		-2
#define E_BIND 			-3
#define E_LISTEN 		-4
#define E_ACCEPT 		-5
#define E_CONNECT 		-6
#define E_COMMUNICATE 	-7
#define E_FNF 			-8
#define E_POINTER 		-9
#define E_CMD 			-10
#define E_BASIC 		-999

#define MSG_SIZE		1000
#define PATH_SIZE		1024
#define MAX_THREADS		1000
#define MAX_CON 		5    // Numero maximo de pedidos de conexao em espera. 



/* Retorna a mensagem de erro equivalente ao codigo passado. */
char* get_error_msg(int cod);

/* Exibe mensagem de erro na saida padrao e termina o programa */
void print_error(int cod);

/* Recebe mensagens (de tamanho maximo igual a buffer_size) enviadas pelo 
 * cliente e as unifica em uma unica mensagem, que e' armazenada no buffer.   
 * Retorna 0 em caso de sucesso, -1 caso contrario. */ 
int get_client_msg(int c_socket, char* buffer, int buffer_size);

/* Envia uma string atraves da rede, dividindo-a em pedaços com tamanho maximo
 * definido por portion. Retorna 0 em caso de sucesso, -1 caso contrario. */
int sendTo(int c_socket, char *buffer, int buffer_size, int portion);

/* Envia uma lista de todos os arquivos contidos no diretorio dir pela rede, em
 * mensagens de tamanho maximo igual a buffer_size. Retorna 0 em caso de 
 * sucesso, -1 caso contrario. */
int send_list(int c_socket, DIR *dir, int buffer_size);

/* Envia o arquivo com nome fname pela rede, em mensagens de tamanho maximo 
 * igual a buffer_size. Retorna 0 em caso de sucesso e -1 caso contrario. */
int send_file(int c_socket, int buffer_size, char *fname);