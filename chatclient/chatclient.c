/*******************************************************************************
 * Name        : chatclient.c
 * Author      : Eric Rudzin and Tudor Rus
 * Date        : May 7, 2021
 * Description : Chat Client Implementation
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "util.h"

int client_socket = -1;
char username[MAX_NAME_LEN + 1];
char inbuf[BUFLEN + 1];
char outbuf[MAX_MSG_LEN + 1];

int handle_stdin() {
    if (get_string(outbuf, MAX_MSG_LEN + 1) == TOO_LONG){
        printf("Sorry, limit your message to %d characters.\n", MAX_MSG_LEN);
    } 
    else {
        if (send(client_socket, outbuf, strlen(outbuf), 0) < 0){
            fprintf(stderr, "Error: Failed to send message to server. %s.\n", strerror(errno));
        }
        if (strcmp(outbuf, "bye") == 0){
            printf("Goodbye.\n");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int handle_client_socket() {
    int bytes_recvd;
    if((bytes_recvd = recv(client_socket, inbuf, BUFLEN - 1, 0)) < 0){
        fprintf(stderr, "Warning: Failed to receive incoming message. %s.\n", strerror(errno));
    }
    if (bytes_recvd == 0){
        fprintf(stderr, "\nConnection to server has been lost.\n");
        return EXIT_FAILURE;
    } 
    else {
        inbuf[bytes_recvd] = '\0';
        if (strcmp(inbuf, "bye") == 0){
            printf("\nServer initiated shutdown.\n");
            return EXIT_FAILURE;
        }
        else {
        printf("\n%s\n", inbuf);
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    if(argc != 3){
        fprintf(stderr, "Usage: %s <server IP> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    int bytes_recvd, ip_conversion, retval = EXIT_SUCCESS;
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    memset(&server_addr, 0, addrlen);

    ip_conversion = inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    if (ip_conversion == 0){
        fprintf(stderr, "Error: Invalid IP address '%s'.\n", argv[1]);
        return EXIT_FAILURE;
    }

    int port;
    if (!parse_int(argv[2], &port, "port number")){
        return EXIT_FAILURE;
    }
    
    if (port < 1024 || port > 65535){
        fprintf(stderr, "Error: Port must be in range [1024, 65535].\n");
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    int user_checker = NO_INPUT;
    printf("Enter your username: ");
    fflush(stdout);
    while((user_checker = get_string(username, MAX_NAME_LEN + 1)) != OK){
        if (user_checker == TOO_LONG){
            fprintf(stderr, "Sorry, limit your username to %d characters.\n", MAX_NAME_LEN);
        }
        printf("Enter your username: ");
    }

    printf("Hello, %s. Let's try to connect to the server.\n", username);

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Error: Failed to create socket. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, addrlen) < 0){
        fprintf(stderr, "Error: Failed to connect to server. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    if ((bytes_recvd = recv(client_socket, inbuf, BUFLEN - 1, 0)) < 0){
        fprintf(stderr, "Error: Failed to receive message from server. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    if(bytes_recvd == 0){
        fprintf(stderr, "All connections are busy. Try again later.\n");
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    inbuf[bytes_recvd] = '\0';  
    printf("\n%s\n\n", inbuf);

    if (send(client_socket, username, strlen(username), 0) < 0){
        fprintf(stderr, "Error: Failed to send username to server. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    fd_set s;

    while(true){
        printf("[%s]: ", username);
        fflush(stdout);

        FD_ZERO(&s);
        FD_SET(STDIN_FILENO, &s);
        FD_SET(client_socket, &s);

        if (select(FD_SETSIZE, &s, NULL, NULL, NULL) < 0) 
        {
            fprintf(stderr, "Error: %s", strerror(errno));
            retval = EXIT_FAILURE;
            goto EXIT;
        }

        int status;

        if (FD_ISSET(STDIN_FILENO, &s)){
            status = handle_stdin();
            if (status == EXIT_FAILURE) {
                retval = EXIT_FAILURE;
                goto EXIT;
            }
        }
        
        if (FD_ISSET(client_socket, &s)){
            status = handle_client_socket();
            if (status == EXIT_FAILURE) {
                retval = EXIT_FAILURE;
                goto EXIT;
            }
        }
    }

EXIT:
    if (fcntl(client_socket, F_GETFD) >= 0) {
        close(client_socket);
    }    
    return retval;
}