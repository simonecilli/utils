/*
 * #brief C ECHO client/server example using sockets
 */
#include <stdio.h>	    //printf
#include <stdlib.h>     //atoi
#include <string.h>	    //strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>     //write
#include <pthread.h>    //for threading , link with lpthread

#define SERVER_PORT     12345
#define SERVER_ROLE     0
#define SERVER_MAX_CONN 64
#define CLIENT_ROLE     1

void *connection_handler(void *);

int main(int argc , char *argv[])
{
	int role, port, val;
	int sd, sd2;
	struct sockaddr_in server, client;
	char buff[128];
	pthread_t thread_id;

	// Handle parameters
	switch(argc)
	{
		case 1:
			role = SERVER_ROLE;
			port = SERVER_PORT;
			break;
		case 2:
			role = (argv[1][0] == 'c')? CLIENT_ROLE : SERVER_ROLE;
			port = SERVER_PORT;
			break;
		case 3:
			role = (argv[1][0] == 'c')? CLIENT_ROLE : SERVER_ROLE;
			port = atoi(argv[2]);
			break;
		default:
			return 1;
	}
	
	//Create socket
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd == -1)
	{
		printf("ERROR - Could not create socket\n");
		return 1;
	}
	printf("Socket created (sd:%d)\n", sd);

	// Initialization
	if(role == SERVER_ROLE)
	{
		val = 1;
		setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof val);
		
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		server.sin_addr.s_addr = htonl(INADDR_ANY);
		if(bind(sd, (struct sockaddr*)&server, sizeof(server)) < 0)
		{
			printf("ERROR - Could not bind socket\n");
			close(sd);
			return 1;
		}
		
		if(listen(sd, 128) < 0)
		{
			printf("ERROR - Could not listen on socket\n");
			close(sd);
			return 1;
		}
		printf("Server is listening on port %d\n", port);
	}
	else if(role == CLIENT_ROLE)
	{
		//Connect to remote server	
		server.sin_addr.s_addr = inet_addr("127.0.0.1");
		server.sin_family      = AF_INET;
		server.sin_port        = htons(port);
		if(connect(sd, (struct sockaddr*)&server, sizeof(server)) < 0)
		{
			printf("ERROR - connect failed\n");
			close(sd);
			return 1;
		}
		printf("Client connected to server 127.0.0.1:%d\n", port);
		printf("Type 'q' to exit.\n");
	}
	else
	{
		printf("ERROR - Unknown role\n");
		close(sd);
		return 1;
	}
	
	// Communication loop
	while(1)
	{
		if(role == SERVER_ROLE)
		{
			val = sizeof(struct sockaddr_in);
			sd2 = accept(sd, (struct sockaddr *)&client, (socklen_t*)&val);
			if (sd2 < 0)
			{
				printf("ERROR - Accept failed\n");
				close(sd);
				return 1;
			}
			
			if(pthread_create(&thread_id, NULL, connection_handler, (void*)&sd2) < 0)
			{
				printf("ERROR - could not create thread\n");
				close(sd);
				return 1;
			}
		}
		else if(role == CLIENT_ROLE)
		{
			system ("/bin/stty raw");
			buff[0] = getchar();
			system ("/bin/stty cooked");
			buff[1] = '\0';
			if (buff[0] == 'q')
			{
				break;
			}
			//scanf("%s", buff);
			
			//Send some data
			if((val = send(sd, buff, strlen(buff), 0)) < 0)
			{
				printf("ERROR - send failed\n");
				close(sd);
				return 1;
			}
			//Receive a reply from the server
			if((val = recv(sd, buff, sizeof(buff)-1, 0)) < 0)
			{
				printf("ERROR - recv failed\n");
				close(sd);
				return 1;
			}
			buff[val] = '\0';
			printf("%s", buff);
		}
	}
	
	close(sd);
	return 0;
}

void *connection_handler(void *sd)
{
	int sock = *(int*)sd;
	int read_size;
	char msg[128];
	
    printf("[sd:%d] Thread created waiting for data\n", sock);
	while((read_size = recv(sock, msg, sizeof(msg), 0)) > 0)
    {
		msg[read_size] = '\0'; //end of string marker
        write(sock , msg , strlen(msg)); //Send the message back to client
		printf("[sd:%d] Echo: %s\n", sock, msg);
		memset(msg, 0, sizeof(msg)); //clear the message buffer
    }
    if(read_size == 0)
    {
        printf("[sd:%d] Client disconnected\n", sock);
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        printf("[sd:%d] Recv failed\n", sock);
    }
    printf("[sd:%d] Close comm with client\n", sock);
	close(sock);         
    return 0;
} 