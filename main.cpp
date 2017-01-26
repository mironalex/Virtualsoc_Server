#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <bits/errno.h>
#include <errno.h>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <netdb.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#include <pqxx/pqxx>
#include "user.h"
#include "requestHandler.h"
enum sock_state {
    OPEN, CLOSED
};

using namespace std;
using namespace pqxx;

FILE * LOGS;


bool getSocketState(int sock) {
    fd_set rfd;
    FD_ZERO(&rfd);
    FD_SET(sock, &rfd);
    timeval tv = {0};
    select(sock + 1, &rfd, 0, 0, &tv);
    if (!FD_ISSET(sock, &rfd))
        return OPEN;
    int n = 0;
    ioctl(sock, FIONREAD, &n);
    if (n == 0) return CLOSED;
    else return OPEN;
}

int openServerSocket(const char *servspec) {
    const int one = 1;
    struct addrinfo hints = {};
    struct addrinfo *res = 0, *ai = 0, *ai4 = 0;
    char *node = strdup(servspec);
    char *service = strrchr(node, ':');
    int sock;

    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    *service++ = '\0';
    getaddrinfo(*node ? node : "0::0", service, &hints, &res);
    free(node);

    for (ai = res; ai; ai = ai->ai_next) {
        if (ai->ai_family == PF_INET6) break;
        else if (ai->ai_family == PF_INET) ai4 = ai;
    }
    ai = ai ? ai : ai4;

    sock = socket(ai->ai_family, SOCK_STREAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    bind(sock, ai->ai_addr, ai->ai_addrlen);
    listen(sock, 256);
    freeaddrinfo(res);
    return sock;
}

void startConnection(int sock) {
    char *username;
    username = new char[26];
    while (getSocketState(sock) == OPEN) {
        unsigned long messageSize;
		messageSize = readInt(sock);
		if(messageSize != 3){
            sendMessage(sock,"Bad Request.");
            break;
        }
		char * reqBuffer = new char[messageSize+1];
        reqBuffer[3] = 0;
		read(sock,reqBuffer,messageSize);
		handleRequest(reqBuffer,username,sock);
    }
    delete[] username;
    close(sock);
}

void clientHandlerLoop(const char *server_spec) {
	char * username;
    int sock = openServerSocket(server_spec);
    while (true) {
        printf("[server]Waiting for client connection\n");
        int new_client_sock = accept(sock, 0, 0);
        printf("[server]Recieved connection, creating new thread\n");
        std::thread t(startConnection, new_client_sock);
        t.detach();
    }
}


int main(int argc, char *argv[]) {
	const char *server = "127.0.0.1:1337";
	LOGS = fopen("log.txt","w");
    signal(SIGPIPE, SIG_IGN);
    if (argc > 1) server = argv[1];
    clientHandlerLoop(server);
	return 0;
}
