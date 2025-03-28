#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char filepath[200], department[50];
    char buffer[BUFFER_SIZE];

    // Step 1 Get input from user
    printf("Enter file path to send: ");
    scanf("%s", filepath);
    printf("Enter department (manufacturing/distribution): ");
    scanf("%s", department);

    // Step 2: Extract filename
    char *filename = strrchr(filepath, '/');
    if (filename)
        filename++;
    else
        filename = filepath;

    // Step 3   : Get file size
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        perror("File not found");
        return 1;
    }
    fseek(fp, 0L, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    // Step  Create socket and connect to server
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }


    // Step 5 :  Send metadata
    uid_t uid = getuid();


    send(sock, department, sizeof(department), 0);
    send(sock, filename, 100, 0); // Max length
    send(sock, &uid, sizeof(uid), 0);
    send(sock, &filesize, sizeof(filesize), 0);

    // Step 6: Send file content
    size_t bytes;
    while ((bytes = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        send(sock, buffer, bytes, 0);
    }
    fclose(fp);

    // Step 7: Receive server response
    char response[20];
    read(sock, response, sizeof(response));
    printf("Server: %s\n", response);

    close(sock);
    return 0;
}
