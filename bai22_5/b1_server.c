#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <poll.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

int main() {
    int serverSocket, clientSockets[MAX_CLIENTS];
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrSize = sizeof(struct sockaddr_in);

    // Khởi tạo mảng clientSockets
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clientSockets[i] = -1;
    }

    // Tạo socket cho server
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ và cổng cho server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(9000);

    // Liên kết socket với địa chỉ và cổng
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối từ client
    if (listen(serverSocket, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for connections...\n");

    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1; // Số lượng file descriptor cần kiểm tra
    fds[0].fd = serverSocket;
    fds[0].events = POLLIN; // Kiểm tra sự kiện đọc

    while (1) {
        int ready = poll(fds, nfds, -1); // Chờ đến khi có sự kiện xảy ra

        if (ready == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        // Kiểm tra các file descriptor
        for (int i = 0; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                // Sự kiện đọc xảy ra trên file descriptor

                if (fds[i].fd == serverSocket) {
                    // Kết nối mới từ client
                    int newClientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize);
                    if (newClientSocket == -1) {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }

                    // Thêm client socket vào mảng clientSockets
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clientSockets[j] == -1) {
                            clientSockets[j] = newClientSocket;
                            break;
                        }
                    }

                    // Cập nhật số lượng clients đang kết nối
                    int numClients = 0;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clientSockets[j] != -1) {
                            numClients++;
                        }
                    }

                    // Gửi xâu chào kèm số lượng clients đến client mới
                    char welcomeMsg[BUFFER_SIZE];
                    snprintf(welcomeMsg, BUFFER_SIZE, "Xin chào. Hiện có %d clients đang kết nối\n", numClients);
                    send(newClientSocket, welcomeMsg, strlen(welcomeMsg), 0);

                    printf("Client connected: %s\n", inet_ntoa(clientAddr.sin_addr));
                }
                else {
                    // Dữ liệu đến từ một client đang kết nối

                    char buffer[BUFFER_SIZE];
                    int bytesRead = recv(fds[i].fd, buffer, BUFFER_SIZE - 1, 0);
                    if (bytesRead == -1) {
                        perror("recv");
                        exit(EXIT_FAILURE);
                    }
                    else if (bytesRead == 0) {
                        // Ngắt kết nối từ client
                        close(fds[i].fd);
                        fds[i].fd = -1;
                        clientSockets[i] = -1;

                        printf("Client disconnected\n");
                    }
                    else {
                        buffer[bytesRead] = '\0';
                        printf("Received data from client: %s\n", buffer);
                    }
                }
            }
        }

        // Cập nhật lại mảng fds và số lượng file descriptor cần kiểm tra
        nfds = 1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clientSockets[i] != -1) {
                fds[nfds].fd = clientSockets[i];
                fds[nfds].events = POLLIN;
                nfds++;
            }
        }
    }

    // Đóng kết nối
    close(serverSocket);

    return 0;
}
