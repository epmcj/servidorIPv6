Trabalho Prático I da Disciplina de Redes de Computadores
===============================================================================
**Alunos: [Alexader Decker](https://github.com/AlexDecker) e [Eduardo Pinto](https://github.com/epmcj)**

 Os códigos fontes dos dois programas estão contidos na pasta bin. Os 
 arquivos server_FTP.c, server_side.c e server_side.h são referentes à 
 aplicação do servidor e o arquivo client_FTP.c é referente à aplicação do 
 cliente. O relatório de desempenho se encontra na pasta doc.


## Instruções de compilação e execução:

Para compilar os programas não é necessário a instalação de nenhuma 
biblioteca adicional, devendo-se apenas executar o comando:
```
 make 
```
Para executar o cliente deve-se utilizar um dos dois comandos a seguir, a 
depender da finalidade desejada.

- Para listar o conteúdo do diretório:
```
./clienteFTP list <nome ou IPv6 do servidor> <porta do servidor> <tam_buffer>
./clienteFTP list ::1 1024 1024
```
- Para realizar a transferência de um arquivo:
```
./clienteFTP get <nome do arquivo> <nome ou IPv6 do servidor> <porta do servidor> <tam_buffer>
./clienteFTP get README ::1 1024 1024
```
- Para executar o servidor:
```
./servidorFTP <porta do servidor> <tam_buffer> <diretório a ser utilizado>
./servidorFTP 1024 1024 .
```
