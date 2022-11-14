#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <syscall.h>

#define BUFFER_SIZE 1024

int main(int argc, char const* argv[]){
    int client_fd;
    struct sockaddr_in server_address;
    int opt = 1;
    char ip_address[17] = {0};
    char port_arr[6] = {0};
    int port = 0;
    signal(SIGPIPE, SIG_IGN); /* Ignore SIGPIPE, this prevents the program from crashing if the server disconnects */
    /* get connection info from user */
    printf("Enter IP address of server to connect to: ");
    scanf("%16s", ip_address);
    printf("Enter the port to connect through: ");
    scanf("%5s", port_arr);
    sscanf(port_arr, "%d", &port);
    /* set up server address sockaddr_in struct */
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if(inet_pton(AF_INET, ip_address, &server_address.sin_addr) <= 0){
        perror("ERR: invalid address (inet_pton() failed)");
        exit(1);
    }
    /* create a socket to connect to the server with */
    if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("ERR: socket() failed");
        exit(1);
    }
    /* connect to the server */
    if(connect(client_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
        perror("ERR: connect() failed");
        exit(1);
    }
    /* send {REQCON{ID}} to the server */
    send(client_fd, "{REQCON{FORTNITEGAMER69}}", 26, 0);
    /* close the client file descriptor */
    close(client_fd);
    return 0;
}