/**
* CONTEST: A Very Simple TCP CONnection TESTer
* 	by Sang-Kil Park
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CHAR 		1024
#define DEFAULT_PORT	11212
#define LISTEN_LIMIT	128

void *handle_client(void *params);
void error_handling(char * msg);

// The total count of connected clients
int client_count = 0;
// The latest number of connected clients
int client_latest = 0;
// binding port
int port = DEFAULT_PORT;
// MUTEX: protecting shared data structures from concurrent modifications.
pthread_mutex_t mutex;

typedef struct {
	int client_id;
	int client_sock;
} thread_param_t;
thread_param_t *params = NULL;

int server_sock, client_sock;
struct sockaddr_in server_addr, client_addr;
unsigned int client_addr_size; // size of `client_addr`
int on = 1; // temp variable for `setsockopt`
pthread_t thread_id;

int main(int argc, char *argv[]) {
	// print `Usage`
	if (argc == 2) {
		if (strcmp(argv[1],"-h") == 0) {
			printf("Usage: %s <Port>\n", argv[0]);
			exit(1);
		}
		port = atoi(argv[1]);
	}

	pthread_mutex_init(&mutex, NULL);
	server_sock = socket(PF_INET, SOCK_STREAM, 0);

	// initialize `sockaddr_in`
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		error_handling("setsockopt(SO_REUSEADDR) Error - Why?");
	if (bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr))==-1)
		error_handling("bind() Error - Not enough privilleges(<1024) or already in use.");
	if (listen(server_sock, LISTEN_LIMIT)==-1)
		error_handling("listen() Error - Why?");

	printf("Listening to %d\n", port);
	while(1) {
		client_addr_size = sizeof(client_addr);
		client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);

		pthread_mutex_lock(&mutex);
		client_count++;
		client_latest++;
		printf("#%d Connected from %s:%d, TOTAL:%d\n",
				client_latest, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_count);
		pthread_mutex_unlock(&mutex);

		// memory allocation for `thread_param_t`
		params = malloc(sizeof(thread_param_t));
		params->client_id = client_latest;
		params->client_sock = client_sock;

		// 1 thread, 1 client
		pthread_create(&thread_id, NULL, handle_client, (void *) params);
		pthread_detach(thread_id);
	}
	return 0;
}

void *handle_client(void *params) {
	thread_param_t *p = (thread_param_t *) params;
	ssize_t str_len = 0;
	char msg[MAX_CHAR];

	// read from a file descriptor(client_sock)
	while((str_len = read(p->client_sock, msg, sizeof(msg)))!=0) {
		pthread_mutex_lock(&mutex);
		printf("#%d Received - ", p->client_id);
		for (int i = 0; i < str_len; i++)
			printf("%c", msg[i]);

		pthread_mutex_unlock(&mutex);
	}

	pthread_mutex_lock(&mutex);
	printf("#%d Disconnected, TOTAL:%d\n", p->client_id, --client_count);
	pthread_mutex_unlock(&mutex);

	close(p->client_sock);
	return NULL;
}

void error_handling(char * msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}