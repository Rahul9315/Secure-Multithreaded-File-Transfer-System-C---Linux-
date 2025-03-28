#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Mutex  only one file is written at a time
pthread_mutex_t lock;

// handle each client connection
void *handle_client(void *arg);

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    pthread_mutex_init(&lock, NULL); // Initialized mutex

    // server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    // Define the servers address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //socket to the specified IP and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening  incoming connections
    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Main loop: Accept and handle client connections
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            perror("accept failed");
            continue;
        }

        // a new thread for each client
        pthread_t tid;
        int *pclient = malloc(sizeof(int));
        *pclient = new_socket;
        pthread_create(&tid, NULL, handle_client, pclient);
    }

    // to destory mutex if infinite loop
    pthread_mutex_destroy(&lock);
    return 0;
}

void *handle_client(void *arg) {
    int sock = *((int *)arg);
    free(arg); // Free memory allocated for client socket

    char buffer[BUFFER_SIZE];
    char department[50] = {0}; //  manufacturing or distribution
    char filename[100] = {0};  // Name of the file being sent
    long filesize = 0;         // Size of the file
    uid_t client_uid;          // UID of the client

    // Read department name from client
    if (read(sock, department, sizeof(department)) <= 0) {
        perror("Failed to read department");
        close(sock);
        return NULL;
    }
    printf("Department: %s\n", department);

    // Read filename from client
    if (read(sock, filename, sizeof(filename)) <= 0) {
        perror("Failed to read filename");
        close(sock);
        return NULL;
    }
    printf("Filename: %s\n", filename);

    // Read user ID from client
    if (read(sock, &client_uid, sizeof(client_uid)) != sizeof(client_uid)) {
        perror("Failed to read UID");
        close(sock);
        return NULL;
    }
    printf("Client UID: %d\n", client_uid);

    // Reading file size from client
    if (read(sock, &filesize, sizeof(filesize)) != sizeof(filesize)) {
        perror("Failed to read filesize");
        close(sock);
        return NULL;
    }
    printf("Filesize: %ld bytes\n", filesize);

    // Cross check  sizes
    if (filesize <= 0 || filesize > 100000000) {
        fprintf(stderr, "Invalid file size.\n");
        close(sock);
        return NULL;
    }

    // Build full file path based on department and filename
    char filepath[200];
    snprintf(filepath, sizeof(filepath), "server_data/%s/%s", department, filename);

    // Lock mutex to avoid conflicts during file writing
    pthread_mutex_lock(&lock);

    // Open the file for writing (create if it doesn't exist)
    int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0660);
    if (fd < 0) {
        perror("File open failed");
        char *msg = "FAIL";
        send(sock, msg, strlen(msg), 0);
        pthread_mutex_unlock(&lock);
        close(sock);
        return NULL;
    }

    // Changing the ownership of the file to the client user
    if (fchown(fd, client_uid, -1) != 0) {
        perror("fchown failed");
    }

    // Begin receiving and writing file content
    long total = 0;
    int bytes;
    while (total < filesize) {
        bytes = read(sock, buffer, BUFFER_SIZE);
        if (bytes <= 0) break;
        write(fd, buffer, bytes);
        total += bytes;
    }

    // Close file and release lock
    close(fd);
    pthread_mutex_unlock(&lock);

    printf("File received and saved to %s\n", filepath);

    // Notify client of success
    char *msg = "SUCCESS";
    send(sock, msg, strlen(msg), 0);

    // Close connection with the client
    close(sock);
    return NULL;
}
