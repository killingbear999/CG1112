#include <pthread.h>
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
#include "tls_client_lib.h"
#include "tls_pthread.h"

volatile int exitFlag=0;

SSL_CTX *ctx;
SSL *ssl;
int sockfd;

void closeAndExit(int exitCode)
{
	close(sockfd);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	thread_cleanup();
	cleanup_openssl();
}

void intHandler(int dummy)
{
	printf("\n\nWHOA MAMA!!!! Why did you press CTRL-C? XD\n\n");
	exitFlag=1;
}


// Cert verification callback. Implement this in the client
int verify_callback(int preverify, X509_STORE_CTX *x509_ctx)
{
	return preverify;
}

void *kbThread(void *p)
{
	SSL *ssl = (SSL *) p;
	while(!exitFlag)
	{
		int len;
		char buffer[1024];
		printf("Say something: ");
		fgets(buffer, sizeof(buffer), stdin);

		len = SSL_write(ssl, buffer, strlen(buffer)+1);
		printf("Wrote %d bytes to the server\n", len);
	}
	
	printf("Keyboard thread exiting\n");
	pthread_exit(NULL);
}

void *readThread(void *p)
{
	SSL *ssl = (SSL *) p;
	while(!exitFlag)
	{
			int len;
			char buffer[1024];
			len = SSL_read(ssl, buffer, sizeof(buffer));
			
			if(len > 0)
				printf("Received %d bytes from server: %s\n", len, buffer);

			// Exit if we read zero bytes. This means the server closed the
			// connection
			exitFlag=(len == 0);
	}

	printf("Read thread exiting\n");
	pthread_exit(NULL);
}

int main(int ac, char **av)
{
	if(ac != 3)
	{
		fprintf(stderr, "\nUSAGE: %s <host name> <port number>\n\n", av[0]);
		exit(-1);
	}

	// Create a structure to store the server address
	int sockfd;
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

	// NEW: Initialize SSL
	init_openssl();

	// NEW: Create context
	ctx = create_context("signing.pem");


	// NEW: Enable multithreading in openSSL
	CRYPTO_thread_setup();

	// Now let's connect!
	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Error connecting: ");
		exit(-1);
	}

	ssl = connectSSL(ctx, sockfd, "192.168.1.16");

	if(ssl == NULL)
		closeAndExit(-1);

	printf("CERTIFICATE DATA:\n");
	printCertificate(ssl);

	if(!verifyCertificate(ssl))
	{
		printf("SSL Certicate validation failed.\n");
		closeAndExit(-1);
	}
	else
		printf("SSL CERTIFICATE IS VALID\n");

	printf("Now we write to the server and get its response. To exit press CTRL-C\n");

	// Install handler to catch CTRL-C (SIGINT)
	signal(SIGINT, intHandler);

	pthread_t _kb, _rd;

	pthread_create(&_kb, NULL, kbThread, (void *) ssl);
	pthread_detach(_kb);
	pthread_create(&_rd, NULL, readThread, (void *) ssl);
	pthread_detach(_rd);

	// Keep looping until we exit
	while(!exitFlag);

	// Now join the threads
	printf("Closing socket and exiting.\n");
	closeAndExit(0);
}

