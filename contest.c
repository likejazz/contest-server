/*
	CONTEST: A Very Simple TCP CONnection TESTer

		by Sang-Kil Park
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 255
#define DEFAULT_PORT 11212
#define LISTEN_LIMIT 128
#define RESPONSE_MSG "END\r\n"

void * handle_clnt(void * params);
void error_handling(char * msg);

int clnt_cnt=0;
int clnt_latest=0;
int port = DEFAULT_PORT;
pthread_mutex_t mutx;

typedef struct {
	int   clnt_id;
	int   clnt_sock;
} thread_parm_t;

int main(int argc, char *argv[]) {

	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	int on=1;
	pthread_t t_id;
	thread_parm_t *parms=NULL;

	if (argc==2) {
		if (strcmp(argv[1],"-h") == 0) {
			printf("Usage: %s <Port>\n", argv[0]);
			exit(1);
		}
		port = atoi(argv[1]);
	}

	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(port);

	if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		error_handling("setsockopt(SO_REUSEADDR) Error - Why?");
	if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() Error - Not enough privilleges(<1024) or already in use.");
	if (listen(serv_sock, LISTEN_LIMIT)==-1)
		error_handling("listen() Error - Why?");

	while(1) {
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);

		pthread_mutex_lock(&mutx);
		clnt_cnt++;
		clnt_latest++;
		printf("Connected %d(#%d) - %s:%d, TC:%d\n",
			clnt_latest, clnt_sock, inet_ntoa(clnt_adr.sin_addr), ntohs(clnt_adr.sin_port), clnt_cnt);
		pthread_mutex_unlock(&mutx);

		parms = malloc(sizeof(thread_parm_t));
		parms->clnt_id = clnt_latest;
		parms->clnt_sock = clnt_sock;

		pthread_create(&t_id, NULL, handle_clnt, (void *)parms);
		pthread_detach(t_id);
	}
	close(serv_sock);
	return 0;
}

void * handle_clnt(void * parms) {
	thread_parm_t *p = (thread_parm_t *)parms;
	int str_len=0, i;
	char msg[BUF_SIZE];

	while((str_len=read(p->clnt_sock, msg, sizeof(msg)))!=0) {
		pthread_mutex_lock(&mutx);
		printf("Received %d(#%d) - ", p->clnt_id, p->clnt_sock);
		for (i=0; i<str_len; i++)
			printf("%c", msg[i]);

		write(p->clnt_sock, RESPONSE_MSG, sizeof(RESPONSE_MSG) - 1);
		pthread_mutex_unlock(&mutx);
	}

	pthread_mutex_lock(&mutx);
	clnt_cnt--;
	printf("Disconnected %d(#%d), TC:%d\n", p->clnt_id, p->clnt_sock, clnt_cnt);
	pthread_mutex_unlock(&mutx);

	close(p->clnt_sock);
	return NULL;
}

void error_handling(char * msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
