#ifndef SERVIDOR_TCP
#define SERVIDOR_TCP

void configurarServidor(unsigned short servidorPorta);
char *TrataClienteTCP(int socketCliente);
char *aguardarMensagem();
void encerrarServidor();

#endif
