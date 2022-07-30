#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include "tls_server_lib.h"
#include "tls_pthread.h"

void *workerThread(void *p)
{
	// NEW: We pass in ssl instead of connfd
	SSL *ssl = (SSL *) p;

	char buffer[1024];

	int len=0;
	do
	{
		// NEW: We use SSL_read and SSL_write instead of read/write. Also
		// we use ssl rather than the file descriptor for the socket.
		// Read in data
		 len = SSL_read(ssl, buffer, sizeof(buffer));

		 // Echo it back. Note if len=0 means other side closed the connection.
		 if(len > 0)
			 SSL_write(ssl, buffer, strlen(buffer)+1);

	} while(len > 0);

	printf("Client closed connection.\n");
	SSL_free(ssl);
	pthread_exit(NULL);
}

int main()
{
	// Declare two integer variables that will
	// point to sockets.
	int listenfd, connfd;

	// serv_addr will be used to configure the port number
	// of our server.
	struct sockaddr_in serv_addr;

	// Set every element in serv_addr to 0.
	memset(&serv_addr, 0, sizeof(serv_addr));

	// Open up a TCP/IP (AF_INET) socket, using the reliable TCP
	// protocol (SOCK_STREAM). You can also create a best effort UDP socket
	// by specifying SOCK_DGRAM instead of SOCK_STREAM. The "0" means
	// use the first protocol in the AF_INET family. The AF_INET family
	// has only one protocol so this is always 0.
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	// We use perror to print out error messages
	if(listenfd < 0)
	{
		perror("Unable to create socket: ");
		exit(-1);
	}

	// Configure our server to bind to all interfaces (INADDR_ANY)
	// including all network cards and WiFi ports. We also
	// set our port number to 5000.
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(5000);

	// Now actually bind our socket to port 5000
	if(bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Unable to bind: ");
		exit(-1);
	}

	// And start listening for connections. We maintain a FIFO
	// queue of 10 entries of "unaccepted" connections. I.e.
	// connections pending acceptance using the accept() function below.

	printf("Now listening..\n");
	if(listen(listenfd, 10) < 0)
	{
		perror("Unable to listen to port: ");
		exit(-1);
	}

	int c = sizeof(struct sockaddr_in);

	// NEW: Initialize the SSL library
	init_openssl();

	// NEW: Create SSL Context
	SSL_CTX *ctx = create_context();

	// NEW: Configure context to load certificate and private keys
	configure_context(ctx, "alex.crt", "alex.key");

	// NEW: Configure multithreading in OpenSSL
	CRYPTO_thread_setup();

	while(1)
	{
		// Accept a new connection from the queue in listen. We will
		// build an echo server
		struct sockaddr_in client;
		connfd = accept(listenfd, (struct sockaddr *) &client, (socklen_t *) &c);

		char clientAddress[32];

		// Use inet_ntop to extract client's IP address.
		inet_ntop(AF_INET, &client.sin_addr, clientAddress, 32);

		printf("Received connection from %s\n", clientAddress);

		SSL *ssl = connectSSL(ctx, connfd);

		if(ssl != NULL)
		{
			pthread_t worker;

			// NEW: We pass in ssl instead of connfd
			pthread_create(&worker, NULL, workerThread, (void *) ssl);
			pthread_detach(worker);
		}
	}

	// We never reach here but good to know how to properly shut down openSSL
	close(listenfd);
	SSL_CTX_free(ctx);
	thread_cleanup();
	cleanup_openssl();
}
