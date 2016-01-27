#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h> 
#include <sys/stat.h>

#define MAX_CONNECTIONS 10
int connections = 0;

void signal_handler(int signal)
{
	connections--;
	printf("Client finished, %d available\n", connections);
}

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
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
	c = sizeof(struct sockaddr_in);

	int parentPid = getpid();
	signal(SIGUSR2, signal_handler);
	
	while(1)
	{
		printf("Waiting for incoming connections...\n");
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		printf("Connection %d accepted\n", connections);

		if (connections + 1 >= MAX_CONNECTIONS)
		{
			printf("Too many connections already, passing\n");
			close(client_sock);
			continue;
		}

		connections++;
		int pid = fork();

		if (pid == 0)
		{
			char receivedMessage[10000];
		
			read_size = recv(client_sock , receivedMessage , 10000 , 0);
			printf("[%d] Received %d bytes\n", getpid(), read_size);
			//printf("Headers: %s", receivedMessage);

			char* filename = strtok(receivedMessage, " ");
			filename = strtok(NULL, " ");
			printf("[%d] Filename: %s\n", getpid(), filename);

			char sentMessage[10000];
			char headers[10000];
			if (strcmp(filename, "/") == 0)
			{
				strcpy(sentMessage, "Root directory.");
				sprintf(headers, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: text/plain\r\n\r\n%s", 
					strlen(sentMessage), sentMessage);
			}
			else if (access( filename, F_OK ) != -1 && is_regular_file(filename))
			{
				FILE * f = fopen (filename, "r");
				char buffer[1000];
				while (fgets(buffer, sizeof(buffer), f))
				{
					strcat(sentMessage, buffer);
				}
				sprintf(headers, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: text/plain\r\n\r\n%s", 
					strlen(sentMessage), sentMessage);
			}
			else
			{
				sprintf(sentMessage, "\"%s\" not found", filename);
				sprintf(headers, "HTTP/1.1 404 Not found\r\nContent-Length: %d\r\nContent-Type: text/plain\r\n\r\n%s", 
					strlen(sentMessage), sentMessage);
			}

			write(client_sock, headers , strlen(headers));
			printf("[%d] Written %d bytes.\n", getpid(), read_size);
			
			close(client_sock);
			kill(parentPid, SIGUSR2);
			return 0;
		}
	}
	
	
	return 0;
}
