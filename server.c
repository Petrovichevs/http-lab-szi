#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#define MAX_WHITELIST 10
#define MAX_CHILDREN 100
#define BUFFER_SIZE 1024

char whitelist[MAX_WHITELIST][16];
int num_whitelist = 0;
char auth_secret[64] = "mysecret123";
int server_port = 8080;
int server_socket = -1;
pid_t children[MAX_CHILDREN];
int num_children = 0;
volatile sig_atomic_t keep_running = 1;

void read_config() {
    FILE *f = fopen("server.conf", "r");
    if (!f) { perror("Нет server.conf"); exit(1); }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "port=", 5) == 0) sscanf(line+5, "%d", &server_port);
        if (strncmp(line, "secret=", 7) == 0) sscanf(line+7, "%s", auth_secret);
    }
    fclose(f);

    f = fopen("whitelist.conf", "r");
    if (!f) { perror("Нет whitelist.conf"); exit(1); }
    num_whitelist = 0;
    while (fgets(line, sizeof(line), f) && num_whitelist < MAX_WHITELIST) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) > 0) strcpy(whitelist[num_whitelist++], line);
    }
    fclose(f);
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
    umask(0);
    chdir("/");
    close(STDIN_FILENO); close(STDOUT_FILENO); close(STDERR_FILENO);
    open("/dev/null", O_RDWR); dup(0); dup(0);
}

void handle_signal(int sig) {
    keep_running = 0;
    for (int i = 0; i < num_children; i++) {
        if (children[i] > 0) {
            kill(children[i], SIGTERM);
            waitpid(children[i], NULL, 0);
        }
    }
    if (server_socket != -1) close(server_socket);
    exit(0);
}

void handle_client(int client_sock, struct sockaddr_in client_addr) {
    char client_ip[16];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    if (!is_in_whitelist(client_ip)) {
        const char *resp = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\n\r\n<h1>Forbidden</h1>";
        send(client_sock, resp, strlen(resp), 0);
        close(client_sock);
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes = recv(client_sock, buffer, sizeof(buffer)-1, 0);
    buffer[bytes] = 0;

    if (strstr(buffer, "X-Auth:") == NULL || strstr(buffer, auth_secret) == NULL) {
        const char *resp = "HTTP/1.1 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>Unauthorized</h1>";
        send(client_sock, resp, strlen(resp), 0);
        close(client_sock);
        return;
    }

    char *label_pos = strstr(buffer, "?label=");
    int label = (label_pos) ? atoi(label_pos + 7) : -1;

    char response[BUFFER_SIZE];
    if (label == 0) {
        sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Метка 0: Простая страница</h1></body></html>");
    } else if (label == 1) {
        sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Метка 1: Защищённые данные</h1></body></html>");
    } else {
        sprintf(response, "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>Invalid label</h1>");
    }

    send(client_sock, response, strlen(response), 0);
    close(client_sock);
}

int main() {
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);

    read_config();
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

    while (keep_running) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_sock = accept(server_socket, (struct sockaddr*)&client_addr, &len);
        if (client_sock < 0) continue;

        pid_t pid = fork();
        if (pid == 0) {
            close(server_socket);
            handle_client(client_sock, client_addr);
            exit(0);
        } else if (pid > 0) {
            close(client_sock);
            if (num_children < MAX_CHILDREN) children[num_children++] = pid;
        }
    }
    return 0;
}
