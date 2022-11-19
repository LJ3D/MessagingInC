#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <syscall.h>

#define perror_exit(msg) perror(msg); exit(EXIT_FAILURE);

/* receive incoming data from the server and print it to stdout */
/* will make this format things properly in the future */
void *receive_messages(void* p_fd){
    int fd = *(int*)p_fd;
    char* buffer;
    long bytes_received;
    buffer = calloc(1024, sizeof(char));
    while(1){
        memset(buffer, 0, 1024); /* set the buffer to all 0s */
        bytes_received = recv(fd, buffer, 1024, 0);
        if(bytes_received == 0){
            printf("Server closed connection\n");
            break;
        }
        printf("Received %d bytes from server: %s\n", bytes_received, buffer);
    }
    free(buffer);
    perror_exit("Connection closed");
    return NULL;
}


int main(int argc, char const* argv[]){
    int client_fd;
    struct sockaddr_in server_address;
    int opt = 1;
    char ip_address[17] = {0};
    char port_arr[6] = {0};
    char user_id_input[32] = {0};
    char* reqcon;
    int port = 0;
    unsigned long bytes_received;
    char* buffer;
    pthread_t receive_thread; /* used to receive messages from the server while the user is typing */

    signal(SIGPIPE, SIG_IGN); /* Ignore SIGPIPE, this prevents the program from crashing if the server disconnects */
    /* get connection info from command line */
    if(argc != 3){
        printf("Usage: %s <ip address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    strncpy(ip_address, argv[1], 16);
    strncpy(port_arr, argv[2], 5);
    port = atoi(port_arr);
    /* set up server address sockaddr_in struct */
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if(inet_pton(AF_INET, ip_address, &server_address.sin_addr) <= 0){
        perror_exit("ERR: invalid address (inet_pton() failed)");
    }
    /* create a socket to connect to the server with */
    if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror_exit("ERR: socket() failed");
    }
    /* connect to the server */
    if(connect(client_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
        perror_exit("ERR: connect() failed");
    }

    /* == now that all the connecting crap is done, we can start talking == */

    /* ask user to specify ID */
    printf("Enter ID to use on the server (max 32 chars): ");
    scanf("%32s", user_id_input);
    /* assemble "{REQCON{user_id_input}}" */
    reqcon = calloc(8+32+3, sizeof(char)); /* allocate enough memory for the absolute maximum size of REQCON */
    sprintf(reqcon, "{REQCON{%s}}", user_id_input);
    /* send reqcon to the server */
    send(client_fd, reqcon, 8+32+3, 0); /* this sends all of reqcon, but it's not a big deal, the server can handle that problem for us */
    free(reqcon); /* free the memory we allocated for reqcon */
    /* see if we got {CON_ACCEPTED} back */
    buffer = calloc(15, sizeof(char)); /* allocate enough memory to store "{CON_ACCEPTED}" (15*sizeof(char)) */
    bytes_received = recv(client_fd, buffer, 15, 0); /* receive the message */
    if(strcmp(buffer, "{CON_ACCEPTED}")==0){ /* if the message is "{CON_ACCEPTED}", all is well */
        printf("Successful connection to server!\n");
    }else{ /* if the message is not "{CON_ACCEPTED}", something went wrong and we should exit */
        printf("Connection to server failed...\n");
    }
    free(buffer);

    /* messaging and everything goes here */
    /* create a thread to receive messages from the server while the user is typing */
    pthread_create(&receive_thread, NULL, receive_messages, &client_fd);
    while(1){
        int msg_size = 0;
        char* userInput = calloc(1016, sizeof(char));
        fgets(userInput, 1016, stdin);
        buffer = calloc(1024, sizeof(char));
        sprintf(buffer, "{MSG{%s}}", userInput);
        /* figure out how long the message is (avoid sending null bytes) */
        msg_size = strlen(buffer);
        send(client_fd, buffer, msg_size+1, 0);
        free(buffer);
        free(userInput);
    }

    /* clean everything up */
    close(client_fd);

    return 0;
}
