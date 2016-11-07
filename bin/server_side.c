#include "server_side.h"

/* Retorna a mensagem de erro equivalente ao codigo passado. */

char* get_error_msg(int cod){
    switch(cod){
        case E_BADARGS:
            return "Erros nos argumentos de entrada";
        case E_SOCKET:
            return "Erro de criação de socket";
        case E_BIND:
            return "Erro de bind";
        case E_LISTEN:
            return "Erro de listen";
        case E_ACCEPT:
            return "Erro de accept";
        case E_CONNECT:
            return "Erro de connect";
        case E_COMMUNICATE:
            return "Erro de comunicação com servidor/cliente";
        case E_FNF:
            return "Arquivo solicitado não encontrado";
        case E_POINTER:
            return "Erro em ponteiro";
        case E_CMD:
            return "Comando de clienteFTP não existente";
        default:
            return "Outros erros (não listados)";
    }
}

/* Exibe mensagem de erro na saida padrao e termina o programa */

void print_error(int cod){
    printf("Erro: %d - Descrição: %s\n", cod, get_error_msg(cod));
}

/* Envia uma mensagem de erro para o cliente com o codigo especificado. Retorna 
 * 0 em caso de sucesso, -1 caso contrario. */

int send_error(int cod, int socket, int buffer_size){
    char msg[3];

    msg[0] = ERROR;
    if(cod == E_BASIC)  // Tratamento de overflow.
        msg[1] = 11;
    else
        msg[1] = -cod;
    msg[2] = '\0';

    return sendTo(socket, msg, 3, buffer_size);
}

/* Recebe mensagens (de tamanho maximo igual a buffer_size) enviadas pelo 
 * cliente e as unifica em uma unica mensagem, que e' armazenada no buffer.   
 * Retorna 0 em caso de sucesso, -1 caso contrario. */ 

int get_client_msg(int c_socket, char* buffer, int buffer_size){
    int i = 0;
    int nBytesRcvd;

    do{
        // Recebendo dados do cliente.
        nBytesRcvd = recv(c_socket, &buffer[i], buffer_size, 0);
        if(nBytesRcvd <= 0)
            return -1;

        i += nBytesRcvd;

    } while(buffer[i-1] != '\0'); // Fim da mensagem e' marcada por '\0'

    return 0;
}

/* Envia uma string atraves da rede, dividindo-a em pedaços com tamanho maximo
 * definido por portion. Retorna 0 em caso de sucesso, -1 caso contrario. */

int sendTo(int c_socket, char *buffer, int buffer_size, int portion){
    int i = 0;
    ssize_t numBytesSent;

    while(i < buffer_size){
        // Caso o numero de bytes a se enviar seja menor do que o limite.
        if((buffer_size - i) < portion)
            portion = (buffer_size - i);

        numBytesSent = send(c_socket, &buffer[i], portion, 0);
        if(numBytesSent <= 0)
            return -1;

        i += numBytesSent;
    }
    return 0;
}

/* Envia uma lista de todos os arquivos contidos no diretorio dir pela rede, em
 * mensagens de tamanho maximo igual a buffer_size. Retorna 0 em caso de 
 * sucesso, E_BASIC caso o diretorio esteja vazio, E_POINTER em caso de erro de
 * alocacao do buffer ou E_COMMUNICATE caso aconteca erro na comunicacao. */

int send_list(int c_socket, DIR *dir, int buffer_size){
    struct dirent *dir_ent;
    char *buffer;
    ssize_t numBytesSent;
    int i, bytesToSend;

    buffer = (char*)calloc(MSG_SIZE, sizeof(char));
    if(buffer == NULL)
        return E_POINTER;

    buffer[0] = OK;
    // Criando uma string que contem o nome de todos os arquivos no diretorio.
    while((dir_ent = readdir(dir)) != NULL){
        if(dir_ent->d_type == DT_REG){  // Verifica se e' um arquivo.
            strcat(buffer, dir_ent->d_name);
            strcat(buffer,"\n");
        }
    }

    // Se o diretorio estiver vazio, deve sinalizar erro.
    if(strlen(buffer) == 1){
        free(buffer);
        return E_BASIC;

    } else{
        buffer[strlen(buffer)-1] = '\0';    // Retirando ultimo '\n' colocado.
    }

    i = 0;
    bytesToSend = strlen(buffer)+1; // Lista + '\0'

    // Enviando 
    while(i < bytesToSend){
        // Caso o numero de bytes a se enviar seja menor do que o limite.
        if((bytesToSend - i) < buffer_size)
            buffer_size = (bytesToSend - i);

        numBytesSent = send(c_socket, &buffer[i], buffer_size, 0);
        if(numBytesSent <= 0){
            free(buffer);
            return E_COMMUNICATE;
        }

        i += numBytesSent;
    }

    free(buffer);
    return 0;
}

/* Envia o arquivo com nome fname pela rede, em mensagens de tamanho maximo 
 * igual a buffer_size. Retorna 0 em caso de sucesso, E_FNF caso o arquivo nao 
 * seja encontrado, E_COMMUNICATE em caso de erro de comunicacao, ou E_POINTER
 * em caso de erro de alocacao de memoria. */

int send_file(int c_socket, int buffer_size, char *fname){
    FILE *fp;
    char * buffer;
    ssize_t bytesRead, numBytesSent;
    int end_msg;

    buffer = (char*)calloc(buffer_size, sizeof(char));
    if(buffer == NULL)
        return E_POINTER;

    fp = fopen(fname, "rb");
    if(fp == NULL){ // Mandar mensagem de arquivo não encontrado.
        free(buffer);
        return E_FNF;
    }

    // Processo de envio do arquivo:
    bytesRead = fread(&buffer[1], 1, buffer_size-1, fp);

    end_msg = 0;
    while(!end_msg){
        // Se leu menos bytes do que deveria, ou aconteceu um problema ou se chegou no fim de arquivo.
      
        if(bytesRead != buffer_size-1){
            if(feof(fp)){ // Caso tenha chegado ao fim do arquivo, adicionar o EOF na mensagem.
                buffer[0] = END;
                end_msg = 1;

                buffer_size = bytesRead+1;

            } else{
                fclose(fp);
                free(buffer);
                return E_COMMUNICATE;
            }
        } else {
	    buffer[0] = OK;	
	}

        numBytesSent = send(c_socket, buffer, buffer_size, 0);
        if(numBytesSent <= 0){
            fclose(fp);
            free(buffer);           
            return E_COMMUNICATE;
        }

        bytesRead = fread(&buffer[1], 1, buffer_size-1, fp);
    }

    fclose(fp);
    free(buffer);
    return 0;
}
