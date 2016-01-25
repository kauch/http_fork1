#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_CONNECTIONS 10
int connections = 0;

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
		
		
		//Receive a message from client
		while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
		{
			printf("Read: %s\n, length: %d\n", client_message, read_size);
			//Send the message back to client
			write(client_sock , client_message , strlen(client_message));
		}
		
		if(read_size == 0)
		{
			printf("Client disconnected\n");
			fflush(stdout);
		}
		else if(read_size == -1)
			printf("recv failed\n");
	}
	
	
	return 0;
}
