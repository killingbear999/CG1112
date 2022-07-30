#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>

// We create our file descriptor for the socket as a global so that our CTRL-C handler
// can close the socket.
int sockfd;

void intHandler(int dummy)
{
	printf("\n\nWHOA MAMA!!!! Why did you press CTRL-C? XD\n\n");
	printf("Closing the socket.\n");
	close(sockfd);
	exit(0);
}

int main(int ac, char **av)
{
	if(ac != 3)
	{
		fprintf(stderr, "\nUSAGE: %s <host name> <port number>\n\n", av[0]);
		exit(-1);
	}

	// Create a structure to store the server address
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));

	// Create a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0)
	{
		perror("Cannot create socket: ");
		exit(-1);
	}

	// Get host IP address
	char hostIP[32];
	struct hostent *he;

	he = gethostbyname(av[1]);

	if(he == NULL)
	{
		herror("Unable to get host IP address: ");
		exit(-1);
	}

	struct in_addr **addr_list = (struct in_addr **) he->h_addr_list;

	strncpy(hostIP, inet_ntoa(*addr_list[0]), sizeof(hostIP));
	printf("Host %s IP address is %s\n", av[1], hostIP);

	// Now form the server address
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(av[2]));
	inet_pton(AF_INET, hostIP, &serv_addr.sin_addr);

	// Now let's connect!
	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Error connecting: ");
		exit(-1);
	}

	printf("Now we write to the server and get its response. To exit press CTRL-C\n");

	// Install handler to catch CTRL-C (SIGINT)
	signal(SIGINT, intHandler);

	int len;

	// Now we read from the keyboard, send to the server, and print the server's response.
	do
	{
		char buffer[1024], c;
		printf("Say something: ");
		fgets(buffer, sizeof(buffer), stdin);

		len = write(sockfd, buffer, strlen(buffer)+1);

		if(len > 0)
		{
			printf("Wrote %d bytes to the server\n", len);
			
			// Now read from the server
			len = read(sockfd, buffer, sizeof(buffer));
			
			if(len > 0)
				printf("Received %d bytes from server: %s\n", len, buffer);
		}

	} while(len > 0);

	printf("Server closed connection\n");
	close(sockfd);
}

