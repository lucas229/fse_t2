#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "servidor_tcp.h"

int servidorSocket;
int socketCliente;
struct sockaddr_in servidorAddr;
struct sockaddr_in clienteAddr;
unsigned int clienteLength;

void configurarServidor(unsigned short servidorPorta) {
	// Abrir Socket
	if((servidorSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		printf("falha no socker do Servidor\n");

	// Montar a estrutura sockaddr_in
	memset(&servidorAddr, 0, sizeof(servidorAddr)); // Zerando a estrutura de dados
	servidorAddr.sin_family = AF_INET;
	servidorAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servidorAddr.sin_port = htons(servidorPorta);

	// Bind
	if(bind(servidorSocket, (struct sockaddr *) &servidorAddr, sizeof(servidorAddr)) < 0)
		printf("Falha no Bind\n");

	// Listen
	if(listen(servidorSocket, 10) < 0)
		printf("Falha no Listen\n");	
}

char *TrataClienteTCP(int socketCliente) {
	char buffer[16], *text = calloc(10000, sizeof(char));
	int tamanhoRecebido;

	if((tamanhoRecebido = recv(socketCliente, buffer, 16, 0)) < 0)
		printf("Erro no recv()\n");

	int tamanho = tamanhoRecebido;
	while (tamanhoRecebido > 0) {
		memcpy(&text[tamanho - tamanhoRecebido], buffer, tamanhoRecebido);

		if(send(socketCliente, buffer, tamanhoRecebido, 0) != tamanhoRecebido)
			printf("Erro no envio - send()\n");
		
		if((tamanhoRecebido = recv(socketCliente, buffer, 16, 0)) < 0)
			printf("Erro no recv()\n");
		
		tamanho += tamanhoRecebido;
	}

	return text;
}

char *aguardarMensagem() {	
	clienteLength = sizeof(clienteAddr);
	if((socketCliente = accept(servidorSocket, 
								(struct sockaddr *) &clienteAddr, 
								&clienteLength)) < 0)
		printf("Falha no Accept\n");
	
	printf("Conexão do Cliente %s\n", inet_ntoa(clienteAddr.sin_addr));
	
	char *text = TrataClienteTCP(socketCliente);

	return text;
}

void encerrarServidor() {
	close(socketCliente);
	close(servidorSocket);
}
