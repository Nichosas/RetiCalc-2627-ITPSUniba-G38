#if defined(WIN32) || defined(_WIN32)
#include <winsock.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int recvString(int sock, char *msg) {
	if (recvAll(sock, msg, MSGSIZE) != MSGSIZE) {
		ErrorHandler("recv() fallita o connessione chiusa prematuramente.\n");
		return -1;
	}
	msg[MSGSIZE - 1] = '\0';
	return 0;
}

void clearInput() {
	int c;
	while ((c = getchar()) != '\n' && c != EOF)
		;
}

int main(void) {
	char serverIp[64];
	int port;

	printf("Inserisci l'indirizzo IP del server: ");
	while (scanf("%63s", serverIp) != 1 || inet_addr(serverIp) == INADDR_NONE) {
		if (feof(stdin))
			return -1;
		clearInput();
		printf("Indirizzo IP non valido, riprova: ");
	}
	printf("Inserisci il numero di porta del server: ");
	while (scanf("%d", &port) != 1 || port <= 0 || port > 65535) {
		if (feof(stdin))
			return -1;
		clearInput();
		printf("Numero di porta non valido, riprova: ");
	}

#if defined(WIN32) || defined(_WIN32)
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Errore in WSAStartup().\n");
		return -1;
	}
#endif

	int Csocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Csocket < 0) {
		ErrorHandler("Creazione della socket fallita.\n");
		ClearWinSock();
		return -1;
	}

	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(serverIp);
	sad.sin_port = htons(port);

	if (connect(Csocket, (struct sockaddr *) &sad, sizeof(sad)) < 0) {
		ErrorHandler("Connessione al server fallita.\n");
		closesocket(Csocket);
		ClearWinSock();
		return -1;
	}

	char msg[MSGSIZE];
	if (recvString(Csocket, msg) < 0) {
		closesocket(Csocket);
		ClearWinSock();
		return -1;
	}
	printf("%s\n", msg);

	int n1, n2, n3;
	struct numeri num;
	while (1) {
		printf("Inserisci tre interi n1 n2 n3: ");
		while (scanf("%d %d %d", &n1, &n2, &n3) != 3) {
			if (feof(stdin)) {
				closesocket(Csocket);
				ClearWinSock();
				return -1;
			}
			clearInput();
			printf("Input non valido, inserisci tre interi n1 n2 n3: ");
		}

		num.n1 = (int) htonl(n1);
		num.n2 = (int) htonl(n2);
		num.n3 = (int) htonl(n3);
		if (send(Csocket, (char *) &num, sizeof(num), 0) != (int) sizeof(num)) {
			ErrorHandler("send() ha inviato un numero di byte diverso da quello atteso.\n");
			break;
		}

		if (recvString(Csocket, msg) < 0)
			break;
		printf("Stringa ricevuta dal server: %s\n", msg);
		if (strcmp(msg, "QUIT") == 0)
			break;
		printf("Risultato res = %s\n", msg);
	}

	closesocket(Csocket);
	ClearWinSock();
	printf("Connessione chiusa.\n");
	return 0;
}
