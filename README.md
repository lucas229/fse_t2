# Trabalho 2 - FSE 2021-2
Repositório referente ao Trabalho 2 da disciplina de Fundamentos de Sistemas Embarcados. A descrição do trabalho pode ser encontrada em: https://gitlab.com/fse_fga/trabalhos-2021_2/trabalho-2-2021-2.

## Compilação
### Servidor Central
Após clonar o repositório, para compilar e executar o servidor central, utilize os seguintes comandos a partir da pasta raíz:
```
cd centralServer
make
make run
```

### Servidor Distribuído
Para iniciar uma instância de um servidor distribuído é necessário incluir o nome do arquivo json com as configurações desejadas que deve estar na pasta distributedServer ao iniciar o programa.
Se o nome do arquivo desejado for, por exemplo, "configuracao_andar_terreo.json", os comandos são os seguintes:
```
cd distributedServer
make
make run JSON=configuracao_andar_terreo.json
```

## Uso
- Ao iniciar o servidor central será apresentado um menu para interação com o sistema. Para acionar um comando do menu é necessário digitar o número associado a opção
desejada, que se encontra entre colchetes antes da descrição do comando.

- A partir do menu inicial é possível acessar o menu específico de um servidor distribuído que esteja conectado, além de ligar ou desligar o alarme de segurança.
Abaixo desses comandos podem ser visualizadas a contagem de pessoas por andar e o estado atual dos alarmes.

- Sempre que um alarme ou sensor estiver ligado o nome aparecerá na cor verde, e se estiver desligado aparecerá na cor vermelha.

- A partir do menu principal, ao acessar o menu específico de um servidor distribuído a interação com os comandos se dá da mesma maneira, com os comandos possíveis aparecendo do lado direito abaixo de Acionamento
e os estados atuais dos sensores, dispositivos e alarmes do lado esquerdo.

- Para retornar ao menu anterior ou finalizar o programa (caso esteja no menu principal) é necessário pressionar a tecla 'q'.

- Se um servidor distribuído aparecer como desconectado não será mais possível acessar o seu menu correspondente no servidor central.

- O arquivo de log será salvo na pasta centralServer/Logs.

- O servidor central por padrão aguarda mensagens TCP na porta 10052. Ao iniciar uma instância de um servidor distribuído, ele se conectará automaticamente ao
servidor central caso esteja disponível.

- O estado do alarme de segurança só poderá ser alterado para ligado se todos os sensores de porta, janela e presença estiverem desligados.
