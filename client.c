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
        printf("Использование: ./client <IP> <порт> <метка> <секрет>\n");
        return 1;
    }
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &addr.sin_addr);
    
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    char req[BUFFER_SIZE];
    sprintf(req, "GET /index.html?label=%s HTTP/1.1\r\nHost: %s\r\nX-Auth: %s\r\n\r\n", 
            argv[3], argv[1], argv[4]);
    send(sock, req, strlen(req), 0);
    
    char buf[BUFFER_SIZE];
    int bytes = recv(sock, buf, sizeof(buf)-1, 0);
    buf[bytes] = 0;
    printf("%s\n", buf);
    
    close(sock);
    return 0;
}
