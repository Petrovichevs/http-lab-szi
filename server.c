#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define MAX_WHITELIST 10

char whitelist[MAX_WHITELIST][16];
int num_whitelist = 0;
char auth_secret[64] = "mysecret123";
int server_port = 8080;
int server_socket = -1;

void read_config() {
    FILE *f = fopen("server.conf", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "port=", 5) == 0) sscanf(line+5, "%d", &server_port);
            if (strncmp(line, "secret=", 7) == 0) sscanf(line+7, "%s", auth_secret);
        }
        fclose(f);
    }
    
    f = fopen("whitelist.conf", "r");
    if (f) {
        char line[256];
        num_whitelist = 0;
        while (fgets(line, sizeof(line), f) && num_whitelist < MAX_WHITELIST) {
            line[strcspn(line, "\r\n")] = 0;
            if (strlen(line) > 0) strcpy(whitelist[num_whitelist++], line);
        }
        fclose(f);
    }
}

int is_in_whitelist(const char *ip) {
    for (int i = 0; i < num_whitelist; i++)
        if (strcmp(whitelist[i], ip) == 0) return 1;
    return 0;
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) exit(0);
    setsid();
    chdir("/");
    close(STDIN_FILENO); close(STDOUT_FILENO); close(STDERR_FILENO);
    open("/dev/null", O_RDWR); dup(0); dup(0);
}

volatile sig_atomic_t keep_running = 1;

void handle_signal(int sig) {
    keep_running = 0;
    if (server_socket != -1) close(server_socket);
    exit(0);
}

int main() {
    read_config();
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);

    daemonize();
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(server_port);
    
    bind(server_socket, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_socket, 5);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_sock = accept(server_socket, (struct sockaddr*)&client_addr, &len);
        if (client_sock < 0) continue;
        
        if (fork() == 0) {
            close(server_socket);
            char buffer[BUFFER_SIZE];
            int bytes = recv(client_sock, buffer, sizeof(buffer)-1, 0);
    buffer[bytes] = 0;
    
    char *label_pos = strstr(buffer, "?label=");
    int label = (label_pos) ? atoi(label_pos + 7) : -1;
    
    if (!is_in_whitelist(client_ip)) {
        const char *resp = "HTTP/1.1 403 Forbidden\r\n\r\n";
        send(client_sock, resp, strlen(resp), 0);
        close(client_sock);
        exit(0);
    }
    
    if (strstr(buffer, auth_secret) == NULL) {
        const char *resp = "HTTP/1.1 401 Unauthorized\r\n\r\n";
        send(client_sock, resp, strlen(resp), 0);
        close(client_sock);
        exit(0);
    }
            const char *resp = "HTTP/1.1 200 OK\r\n\r\nOK\n";
            send(client_sock, resp, strlen(resp), 0);
            close(client_sock);
            exit(0);
        }
        close(client_sock);
    }
    return 0;
}
