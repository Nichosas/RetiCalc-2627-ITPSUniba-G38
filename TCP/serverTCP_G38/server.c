#if defined(WIN32) || defined(_WIN32)
#include <winsock.h>
typedef int addrlen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define closesocket close
typedef socklen_t addrlen_t;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROTOPORT 27015
#define QLEN 6
#define MSGSIZE 64

struct numeri {
	int n1;
	int n2;
	int n3;
};

void ErrorHandler(char *errorMessage) {
	printf("%s", errorMessage);
}

void ClearWinSock() {
#if defined(WIN32) || defined(_WIN32)
	WSACleanup();
#endif
}

int recvAll(int sock, char *buf, int len) {
	int totalBytesRcvd = 0;
	int bytesRcvd;
	while (totalBytesRcvd < len) {
		bytesRcvd = recv(sock, buf + totalBytesRcvd, len - totalBytesRcvd, 0);
		if (bytesRcvd <= 0)
			return bytesRcvd;
		totalBytesRcvd += bytesRcvd;
	}
	return totalBytesRcvd;
}

int sendString(int sock, const char *s) {
	char msg[MSGSIZE];
	memset(msg, 0, MSGSIZE);
	strncpy(msg, s, MSGSIZE - 1);
	if (send(sock, msg, MSGSIZE, 0) != MSGSIZE) {
		ErrorHandler("send() ha inviato un numero di byte diverso da quello atteso.\n");
		return -1;
	}
	return 0;
}

void HandleTCPClient(int clientSocket) {
	struct numeri num;
	char resString[MSGSIZE];
	char tmp[MSGSIZE];
	int bytesRcvd;
	int n1, n2, n3, res;

	if (sendString(clientSocket, "connessione avvenuta") < 0) {
		closesocket(clientSocket);
		return;
	}

	while (1) {
		bytesRcvd = recvAll(clientSocket, (char *) &num, sizeof(num));
		if (bytesRcvd == 0) {
			printf("Il client ha chiuso la connessione.\n");
			break;
		}
		if (bytesRcvd < 0) {
			ErrorHandler("recv() fallita.\n");
			break;
		}

		n1 = (int) ntohl(num.n1);
		n2 = (int) ntohl(num.n2);
		n3 = (int) ntohl(num.n3);
		res = n1 + n2 - n3;
		printf("la somma di %d+%d-%d=%d\n", n1, n2, n3, res);

		if (res == 100) {
			if (sendString(clientSocket, "QUIT") == 0) {
				while (recv(clientSocket, tmp, MSGSIZE, 0) > 0)
					;
				printf("Inviata la stringa QUIT, connessione chiusa dal client.\n");
			}
			break;
		}

		sprintf(resString, "%d", res);
		if (sendString(clientSocket, resString) < 0)
			break;
	}
	closesocket(clientSocket);
}

int main(int argc, char *argv[]) {
	int port = PROTOPORT;
	if (argc > 1) {
		port = atoi(argv[1]);
		if (port <= 0 || port > 65535) {
			printf("Numero di porta non valido: %s\n", argv[1]);
			return 0;
		}
	}

#if defined(WIN32) || defined(_WIN32)
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandler("Errore in WSAStartup().\n");
		return -1;
	}
#endif

	int MySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (MySocket < 0) {
		ErrorHandler("Creazione della socket fallita.\n");
		ClearWinSock();
		return -1;
	}

	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = htonl(INADDR_ANY);
	sad.sin_port = htons(port);
	if (bind(MySocket, (struct sockaddr *) &sad, sizeof(sad)) < 0) {
		ErrorHandler("bind() fallita.\n");
		closesocket(MySocket);
		ClearWinSock();
		return -1;
	}

	if (listen(MySocket, QLEN) < 0) {
		ErrorHandler("listen() fallita.\n");
		closesocket(MySocket);
		ClearWinSock();
		return -1;
	}

	struct sockaddr_in cad;
	int clientSocket;
	addrlen_t clientLen;
	while (1) {
		printf("Server in attesa di connessioni sulla porta %d...\n", port);
		fflush(stdout);
		clientLen = sizeof(cad);
		clientSocket = accept(MySocket, (struct sockaddr *) &cad, &clientLen);
		if (clientSocket < 0) {
			ErrorHandler("accept() fallita.\n");
			continue;
		}
		printf("connesso con il client il cui nome IP: %s\n", inet_ntoa(cad.sin_addr));
		HandleTCPClient(clientSocket);
	}

	closesocket(MySocket);
	ClearWinSock();
	return 0;
}
