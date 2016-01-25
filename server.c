#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h> 

#define MAX_CONNECTIONS 10
int connections = 0;

void signal_handler(int signal)
{
	connections--;
}

int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[2000];
	
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket\n");
	}
	printf("Socket created\n");
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 3000 );
	
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		printf("Socket binding error\n");
		return 1;
	}

	listen(socket_desc , 3);
	
	//Accept and incoming connection
	printf("Waiting for incoming connections...\n");
	c = sizeof(struct sockaddr_in);

	int parentPid = getpid();
	signal(SIGUSR2, signal_handler);
	
	//accept connection from an incoming client
	while(1)
	{
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		printf("Connection %d accepted\n", connections);

		if (connections + 1 >= MAX_CONNECTIONS)
		{
			printf("Too many connections already, passing\n");
			close(client_sock);
		}

		connections++;
		int pid = fork();

		if (pid != 0)
			continue;

		char receivedMessage[10000];
		
		read_size = recv(client_sock , receivedMessage , 2000 , 0);
		printf("Received %d bytes\n", read_size);

		char* sentMessage = "Hello!";

		char headers[10000];
		sprintf(headers, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: text/plain\r\n\r\n%s", 
			strlen(sentMessage), sentMessage);

		write(client_sock, headers , strlen(headers));
		printf("Written %d bytes.\n", read_size);
		
		close(client_sock);
		kill(parentPid, SIGUSR2);
		return 0;
	}
	
	
	return 0;
}
