#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "list.h"

#define PORT 9001
#define ACK "ACK"
#define CMD_BUF_SIZE 1024

static list_t *mylist = NULL;
static int servSockD = -1;
static int clientSocket = -1;

/* Ctrl-C or "exit": free the list, close sockets, exit */
static void shutdown_server(int sig)
{
    (void)sig;
    if (mylist != NULL) {
        list_free(mylist);
        mylist = NULL;
    }
    if (clientSocket != -1) {
        close(clientSocket);
        clientSocket = -1;
    }
    if (servSockD != -1) {
        close(servSockD);
        servSockD = -1;
    }
    exit(0);
}

/* Read the next integer token from the command. Returns 1 on success. */
static int next_int(int *out)
{
    char *tok = strtok(NULL, " ");
    if (tok == NULL) return 0;
    *out = atoi(tok);
    return 1;
}

int main(void)
{
    char buf[CMD_BUF_SIZE];
    char sbuf[CMD_BUF_SIZE];
    char *token;
    char *reply;
    char *heap_reply;
    int n, val, idx, len;
    struct sockaddr_in servAddr;

    signal(SIGINT, shutdown_server);

    servSockD = socket(AF_INET, SOCK_STREAM, 0);
    if (servSockD < 0) {
        perror("socket");
        return 1;
    }

    /* Lets you restart the server right away without "address already in use" */
    int opt = 1;
    setsockopt(servSockD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(servSockD, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        perror("bind");
        return 1;
    }
    if (listen(servSockD, 1) < 0) {
        perror("listen");
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);
    clientSocket = accept(servSockD, NULL, NULL);
    if (clientSocket < 0) {
        perror("accept");
        return 1;
    }

    mylist = list_alloc();

    while (1) {
        n = recv(clientSocket, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;          /* client disconnected or error */
        buf[n] = '\0';

        /* Default reply, so the client's recv() never blocks */
        reply = sbuf;
        heap_reply = NULL;
        strcpy(sbuf, "ERROR: unknown command");

        token = strtok(buf, " ");

        if (token == NULL) {
            strcpy(sbuf, "");
        }
        else if (strcmp(token, "exit") == 0) {
            shutdown_server(0);
        }
        else if (strcmp(token, "menu") == 0) {
            strcpy(sbuf, "");       /* client prints the menu itself */
        }
        else if (strcmp(token, "print") == 0) {
            heap_reply = listToString(mylist);   /* full length, not truncated */
            reply = heap_reply;
        }
        else if (strcmp(token, "get_length") == 0) {
            snprintf(sbuf, sizeof(sbuf), "Length = %d", list_length(mylist));
        }
        else if (strcmp(token, "add_back") == 0) {
            if (next_int(&val)) {
                list_add_to_back(mylist, val);
                snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
            } else {
                strcpy(sbuf, "ERROR: add_back needs <value>");
            }
        }
        else if (strcmp(token, "add_front") == 0) {
            if (next_int(&val)) {
                list_add_to_front(mylist, val);
                snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
            } else {
                strcpy(sbuf, "ERROR: add_front needs <value>");
            }
        }
        else if (strcmp(token, "add_position") == 0) {
            if (next_int(&idx) && next_int(&val)) {
                len = list_length(mylist);
                if (idx < 1 || idx > len + 1) {
                    strcpy(sbuf, "ERROR: index out of range");
                } else {
                    list_add_at_index(mylist, val, idx);   /* value, then index */
                    snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
                }
            } else {
                strcpy(sbuf, "ERROR: add_position needs <index> <value>");
            }
        }
        else if (strcmp(token, "remove_back") == 0) {
            if (list_length(mylist) == 0) {
                strcpy(sbuf, "ERROR: list is empty");
            } else {
                val = list_remove_from_back(mylist);
                snprintf(sbuf, sizeof(sbuf), "%d", val);
            }
        }
        else if (strcmp(token, "remove_front") == 0) {
            if (list_length(mylist) == 0) {
                strcpy(sbuf, "ERROR: list is empty");
            } else {
                val = list_remove_from_front(mylist);
                snprintf(sbuf, sizeof(sbuf), "%d", val);
            }
        }
        else if (strcmp(token, "remove_position") == 0) {
            if (next_int(&idx)) {
                len = list_length(mylist);
                if (idx < 1 || idx > len) {
                    strcpy(sbuf, "ERROR: index out of range");
                } else {
                    val = list_remove_at_index(mylist, idx);
                    snprintf(sbuf, sizeof(sbuf), "%d", val);
                }
            } else {
                strcpy(sbuf, "ERROR: remove_position needs <index>");
            }
        }
        else if (strcmp(token, "get") == 0) {
            if (next_int(&idx)) {
                len = list_length(mylist);
                if (idx < 1 || idx > len) {
                    strcpy(sbuf, "ERROR: index out of range");
                } else {
                    val = list_get_elem_at(mylist, idx);
                    snprintf(sbuf, sizeof(sbuf), "%d", val);
                }
            } else {
                strcpy(sbuf, "ERROR: get needs <index>");
            }
        }

        /* Always reply exactly once, including the terminating '\0' */
        send(clientSocket, reply, strlen(reply) + 1, 0);
        free(heap_reply);           /* free(NULL) is safe */
        memset(buf, 0, sizeof(buf));
    }

    shutdown_server(0);
    return 0;
}
