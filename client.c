#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Использование: ./client <IP_сервера> <порт> <метка_0_или_1> <секрет>\n");
        return 1;
    }
    char *ip = argv[1];
    int port = atoi(argv[2]);
    int label = atoi(argv[3]);
    char *secret = argv[4];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    char req[BUFFER_SIZE];
    sprintf(req, "GET /index.html?label=%d HTTP/1.1\r\nHost: %s\r\nX-Auth: %s\r\nConnection: close\r\n\r\n", label, ip, secret);
    send(sock, req, strlen(req), 0);

    char buf[BUFFER_SIZE];
    int bytes = recv(sock, buf, sizeof(buf)-1, 0);
    buf[bytes] = 0;
    printf("Ответ сервера:\n%s\n", buf);

    close(sock);
    return 0;
}
