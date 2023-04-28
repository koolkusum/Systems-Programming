#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>

#define BUFLEN 256
int connect_inet(char *host, char *service) {
    struct addrinfo pippy, *infolist, *info;
    int sock, error;
    memset(&pippy, 0, sizeof(pippy));
    pippy.ai_family = AF_UNSPEC; // in practice, this means give us IPv4 or IPv6
    pippy.ai_socktype = SOCK_STREAM; // indicate we want a streaming socket
    error = getaddrinfo(host, service, &pippy, &infolist);
    if (error) {
        fprintf(stderr, "error looking up %s:%s: %s\n", host, service, gai_strerror(error));
        return -1;
    }
    for (info = infolist; info != NULL; info = info->ai_next) {
        sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (sock < 0) continue;
        error = connect(sock, info->ai_addr, info->ai_addrlen);
        if (error) {
            close(sock);
            continue;
        }
        break;
    }
    freeaddrinfo(infolist);
    if (info == NULL) {
        fprintf(stderr, "Unable to connect to %s:%s\n", host, service);
        return -1;
    }
    return sock;
}

void *read_from_server(void *arg) {
    int sock = *(int *)arg;
    char buf[BUFLEN];
    ssize_t bytes;
    while ((bytes = recv(sock, buf, BUFLEN - 1, 0)) > 0) {
        buf[bytes] = '\0';
        printf("%s\n", buf);
    }
    return NULL;
}
void *write_to_server(void *arg) {
    int sock = *(int *)arg;
    char buf[BUFLEN];
    ssize_t bytes;
    while ((bytes = read(STDIN_FILENO, buf, BUFLEN)) > 0) {
        ssize_t writtenbytes = write(sock, buf, bytes);
        if (writtenbytes != bytes) {
            perror("Error: could not write all data to the server");
            break;
        }
    }
    return NULL;
}
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Specify host and service\n");
        exit(EXIT_FAILURE);
    }
    int sock = connect_inet(argv[1], argv[2]);
    if (sock < 0) {
        exit(EXIT_FAILURE);
    }
    pthread_t read_thread, write_thread;
    pthread_create(&read_thread, NULL, read_from_server, (void *)&sock);
    pthread_create(&write_thread, NULL, write_to_server, (void *)&sock);
    pthread_join(read_thread, NULL);
    pthread_join(write_thread, NULL);
    close(sock);
    return EXIT_SUCCESS;
}
