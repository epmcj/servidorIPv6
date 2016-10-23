#include "common.h"

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

void error(int cod){
    printf("Erro: %d - Descrição: %s\n", cod, get_error_msg(cod));
    exit(1);
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
printf("::%d\n", (int)numBytesSent);
        if(numBytesSent <= 0)
            return -1;

        i += numBytesSent;
    }
    return 0;
}

/* Envia uma lista de todos os arquivos contidos no diretorio dir pela rede, em
 * mensagens de tamanho maximo igual a buffer_size. Retorna 0 em caso de 
 * sucesso, -1 caso contrario. */

int send_list(int c_socket, DIR *dir, int buffer_size){
    struct dirent *dir_ent;
    char *buffer;
    ssize_t numBytesSent;
    int i, bytesToSend;

    buffer = (char*)calloc(MSG_SIZE, sizeof(char));
    if(buffer == NULL)
        exit(1);

    buffer[0] = 'O'; // Sinalizacao de mensagem OK.
    // Criando uma string que contem o nome de todos os arquivos no diretorio.
    while((dir_ent = readdir(dir)) != NULL){
        if(dir_ent->d_type == DT_REG){  // Verifica se e' um arquivo.
            strcat(buffer, dir_ent->d_name);
            strcat(buffer,"\n");
        }
    }
printf("> %s\n", buffer);
    i = 0;
    bytesToSend = strlen(buffer)+1; // Lista + '\0'
printf("::%d::\n", bytesToSend);

    // Enviando 
    while(i < bytesToSend){
        // Caso o numero de bytes a se enviar seja menor do que o limite.
        if((bytesToSend - i) < buffer_size)
            buffer_size = (bytesToSend - i);

        numBytesSent = send(c_socket, &buffer[i], buffer_size, 0);
printf("::%d\n", (int)numBytesSent);
        if(numBytesSent <= 0){
            free(buffer);
            return -1;
        }

        i += numBytesSent;
    }
    //send(c_socket, buffer, bytesToSend, buffer_size);

    free(buffer);
    return 0;
}

/* Envia o arquivo com nome fname pela rede, em mensagens de tamanho maximo 
 * igual a buffer_size. Retorna 0 em caso de sucesso e -1 caso contrario. */

int send_file(int c_socket, int buffer_size, char *fname){
    FILE *fp;
    char * buffer;
    ssize_t bytesRead, numBytesSent;
    int end_msg;

    buffer = (char*)calloc(buffer_size, sizeof(char));
    if(buffer == NULL)
        exit(1);

    fp = fopen(fname, "r");
    if(fp == NULL){ // Mandar mensagem de arquivo não encontrado.
        buffer[0] = 'E';
        buffer[1] = E_FNF;
        buffer[2] = '\0';

        sendTo(c_socket, buffer, 3, buffer_size);
        free(buffer);
        return -1;
    }

    // Processo de envio do arquivo:
    buffer[0] = 'O';    // Sinalizacao de mensagem OK.
    bytesRead = fread(&buffer[1], 1, buffer_size - 1, fp);
    if(bytesRead > 0) // Correcao para incluir o 'O'.
        bytesRead++;

    end_msg = 0;
    while(!end_msg){
        // Se leu menos bytes do que deveria, ou aconteceu um problema ou se chegou no fim de arquivo.
        if(bytesRead != buffer_size){
            if(feof(fp)){ // Caso tenha chegado ao fim do arquivo, adicionar o EOF na mensagem.
                buffer[bytesRead] = EOF;
                end_msg = 1;
                buffer_size = bytesRead+1;

            } else{
                fclose(fp);
                free(buffer);
                return -1;
            }
        }

        numBytesSent = send(c_socket, buffer, buffer_size, 0);
printf("::%d\n", (int)numBytesSent);
        if(numBytesSent <= 0){
            fclose(fp);
            free(buffer);
            return -1;
        }

        bytesRead = fread(buffer, 1, buffer_size, fp);
    }

    fclose(fp);
    free(buffer);
    return 0;
}