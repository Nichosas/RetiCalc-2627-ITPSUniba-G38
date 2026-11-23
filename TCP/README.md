# Consegna TCP - Gruppo G38

**Sistema operativo in uso:** Windows 11 (64 bit)

Il codice è portabile (Windows con Winsock / Unix) tramite `#if defined(WIN32) || defined(_WIN32)`.

## Progetti

- `clientTCP_G38/client.c`: client TCP
- `serverTCP_G38/server.c`: server TCP

## Protocollo

1. Il server è avviato su una porta a scelta (argomento da riga di comando, default 27015) e resta sempre in ascolto.
2. Il client legge da tastiera l'IP dell'host e la porta del server e richiede la connessione.
3. Il server stampa `connesso con il client il cui nome IP: <ip>` e invia `connessione avvenuta`, che il client stampa.
4. Il client legge tre interi `n1 n2 n3` e li invia al server (in network byte order).
5. Il server stampa `la somma di n1+n2-n3=res`; se `res = 100` invia `QUIT`, altrimenti invia `res`.
6. Il client stampa la stringa ricevuta: se è `QUIT` chiude la connessione e termina, altrimenti stampa `res` e torna al punto 4.
7. Il server torna in ascolto per il client successivo.

## Compilazione (MinGW su Windows)

Nel progetto Eclipse aggiungere la libreria `ws2_32` al linker, oppure da riga di comando:

```
gcc -o server serverTCP_G38/server.c -lws2_32
gcc -o client clientTCP_G38/client.c -lws2_32
```
