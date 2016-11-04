//Falta a marcação de tempo
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//Constantes para os modos
#define LIST 0
#define GET  1

//Definicao dos codigos de erro
char* Erros[11];

void iniciarECods();
void errorMsg(int codErro);

int main(int argc, char** argv){
	char modo;
	char* nomeArquivo = NULL;
	char* IP_servidor = NULL;
	in_port_t porta   = -1;
	int tam_buffer 	  = 0;
	char* buffer      = NULL;
	iniciarECods();//Inicia o controle de erros
	if(argc<5){//formato errado
		errorMsg(-1);
		exit(0);
	}
	
	if(strcmp(argv[1],"list") == 0){
		modo = LIST;
		IP_servidor = argv[2];
		porta		= atoi(argv[3]);
		tam_buffer  = atoi(argv[4]);
	}else if(strcmp(argv[1],"get") == 0){
		if(argc!=6){//erro no numero de parametros
			errorMsg(-1);
			exit(0);
		}
		modo = GET;
		nomeArquivo = argv[2];
		IP_servidor = argv[3];
		porta	    = atoi(argv[4]);
		tam_buffer  = atoi(argv[5]);
		if((porta<1024)||(tam_buffer<=0)){
			errorMsg(-1);
			exit(0);
		}
	}else{//Comando cliente n existe
		errorMsg(-10);
		exit(0);
	}
	
	buffer = (char*)malloc(tam_buffer+1);
	if(buffer == NULL){
		errorMsg(-9);
		exit(0);	
	}	

	int sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);//Criando um socket ipv6
	if(sock<0){
		errorMsg(-2);
		exit(0);
	}
	
	struct sockaddr_in6 servAddr;
	memset(&servAddr, 0, sizeof(servAddr));//completando a estrutura com 0's
	servAddr.sin6_family = AF_INET6;
	inet_pton(AF_INET6, IP_servidor, &servAddr.sin6_addr.s6_addr);
	//servAddr.sin6_addr = in6addr_any;
	servAddr.sin6_port = htons(porta);
	
	if(connect(sock, (struct sockaddr*) &servAddr, sizeof(servAddr))<0){
		errorMsg(-6);
		exit(0);
	}
	
	//Envia a requisicao
	if(modo == LIST){
		//Mensagem única apenas com um caractere "L"
		buffer[0] = 'L';
		buffer[1] = '\0';
		if(send(sock, buffer, (size_t) 2, 0)!=2){
			errorMsg(-7);
			exit(0);
		}
	
	}else{//modo GET
		int qtd = strlen(nomeArquivo)+1;//comprimento do nome do arquivo (incuindo o \0)
		int i = 0;//atual indice do buffer
		int j = 0;//quantos caracteres do arquivo ja foram enviados
		buffer[i++] = 'G';//tipo de comando:get
		while(j<qtd){
			while((i<tam_buffer)&&(j<qtd)){//completa o buffer com o que tem que ser enviado e atualiza os marcadores
				buffer[i++] = nomeArquivo[j++];
			}
			if(send(sock, buffer, (size_t) i, 0)!=i){
				errorMsg(-7);
				exit(0);
			}
			if(i==tam_buffer)i=0;
		}
	}

	char keep = 1;//flag que termina o recebimento quando encontra um \0 ou eof
	int i,n;//para o percorrimento de vetores
	if((n=recv(sock, buffer, tam_buffer, 0))<0){//recebe a primeira msg
		//falha
		errorMsg(-7);
		exit(0);
	}

	if(buffer[0]=='E'){//o servidor teve um erro
		errorMsg(-(*(buffer+1)));
		exit(0);
	}
	
	//se não há erro, um tratamento diferente para cada modo
	if(modo == LIST){
		//trata o resto da primeira mensagem
		for(i=1;i<tam_buffer;i++){//percorre o buffer
			if(buffer[i]=='\0'){
				keep=0;
				printf("\n");
				break;
			}else{
				printf("%c",buffer[i]);//suponho que a lista venha formatada
			}
		}
		while(keep){//trata outras mensagens
			if(recv(sock, buffer, tam_buffer, 0)<0){
				//falha
				errorMsg(-7);
				exit(0);
			}
			for(i=0;i<tam_buffer;i++){//percorre o buffer
				if(buffer[i]=='\0'){
					keep=0;
					printf("\n");
					break;
				}else{
					printf("%c",buffer[i]);//suponho que a lista venha formatada
				}
			}
		}
	}else{//modo = get
		FILE* arq = fopen(nomeArquivo,"wb");
		if(arq==NULL){
			errorMsg(-9);
			exit(0);
		}

		//trata o resto da primeira mensagem
		for(i=1;i<n;i++){//percorre o buffer
			fwrite(&(buffer[i]),1,1,arq);//suponho que a lista venha formatada
		}
		while(buffer[0]!='F'){//trata outras mensagens

			if((n=recv(sock, buffer, tam_buffer, 0))<0){
				//falha
				errorMsg(-7);
				exit(0);
			}
//printf("::%d",n);
			for(i=1;i<n;i++){//percorre o buffer
				fwrite(&(buffer[i]),1,1,arq);//suponho que a lista venha formatada
			}
		}
		fclose(arq);
	}
	close(sock);
	free(buffer);
	
	return 0;
}

void iniciarECods(){
	Erros[0] = "Erros nos argumentos de entrada";
	Erros[1] = "Erro de criacao de socket";
	Erros[2] = "Erro de bind";
	Erros[3] = "Erro de listen";
	Erros[4] = "Erro de accept";
	Erros[5] = "Erro de connect";
	Erros[6] = "Erro de comunicacao com servidor/cliente";
	Erros[7] = "Arquivo solicitado nao encontrado";
	Erros[8] = "Erro em ponteiro";
	Erros[9] = "Comando de clienteFTP nao existente";
	Erros[10] = "Outros erros (nao listados)";
}

void errorMsg(int codErro){
	if(codErro!=-999)
		printf("Erro %d - Descricao: %s\n",codErro, Erros[(-codErro)-1]);
	else
		printf("Erro %d - Descricao: %s\n",codErro, Erros[10]);
}
