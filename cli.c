#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 9001
#define MAX_COMMAND_LINE_LEN 1024
#define MAX_RESPONSE_LEN 65536

static char* getCommandLine(char *command_line)
{
    do {
        if (fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) {
            if (ferror(stdin)) fprintf(stderr, "fgets error\n");
            exit(0);                /* EOF (Ctrl-D) or read error */
        }
    } while (command_line[0] == '\n');   /* skip blank lines */

    command_line[strcspn(command_line, "\n")] = '\0';   /* strip newline */
    return command_line;
}

static void print_menu(void)
{
    printf("COMMANDS:\n---------\n"
           "1. print\n2. get_length\n3. add_back <value>\n4. add_front <value>\n"
           "5. add_position <index> <value>\n6. remove_back\n7. remove_front\n"
           "8. remove_position <index>\n9. get <index>\n10. exit\n");
}

int main(void)
{
    int sockID = socket(AF_INET, SOCK_STREAM, 0);
    char buf[MAX_COMMAND_LINE_LEN];
    char *response = malloc(MAX_RESPONSE_LEN);
    struct sockaddr_in servAddr;
    char *token;
    ssize_t n;

    if (sockID < 0 || response == NULL) {
        perror("setup");
        return 1;
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(sockID, (struct sockaddr *)&servAddr, sizeof(servAddr)) == -1) {
        perror("connect (is the server running?)");
        return 1;
    }

    while (1) {
        printf("Enter Command (or menu): ");
        getCommandLine(buf);

        /* Copy the command for token parsing, since strtok changes its input */
        char cmd[MAX_COMMAND_LINE_LEN];
        strcpy(cmd, buf);
        token = strtok(cmd, " ");
        if (token == NULL) token = "";

        send(sockID, buf, strlen(buf), 0);

        if (strcmp(token, "exit") == 0) {
            break;                  /* server shuts down and sends nothing back */
        }
        if (strcmp(token, "menu") == 0) {
            print_menu();
        }

        n = recv(sockID, response, MAX_RESPONSE_LEN - 1, 0);
        if (n <= 0) {
            printf("Server disconnected.\n");
            break;
        }
        response[n] = '\0';
        printf("\nSERVER RESPONSE: %s\n", response);
    }

    free(response);
    close(sockID);
    return 0;
}
