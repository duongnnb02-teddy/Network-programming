#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_BUFFER_SIZE 1024
#define SERVER_PORT 8888

void handle_client(int client_sock) {
    char buffer[MAX_BUFFER_SIZE];
    char formatted_time[MAX_BUFFER_SIZE];

    // Nhận lệnh từ client
    memset(buffer, 0, sizeof(buffer));
    if (recv(client_sock, buffer, sizeof(buffer), 0) < 0) {
        perror("Error in recv");
        exit(EXIT_FAILURE);
    }

    // Kiểm tra lệnh từ client
    char* command = strtok(buffer, " ");
    char* format = strtok(NULL, " ");

    if (strcmp(command, "GET_TIME") == 0) {
        // Lấy thời gian hiện tại
        time_t current_time = time(NULL);
        struct tm* time_info = localtime(&current_time);

        // Định dạng thời gian theo format
        if (strcmp(format, "dd/mm/yyyy") == 0) {
            strftime(formatted_time, sizeof(formatted_time), "%d/%m/%Y", time_info);
        } else if (strcmp(format, "dd/mm/yy") == 0) {
            strftime(formatted_time, sizeof(formatted_time), "%d/%m/%y", time_info);
        } else if (strcmp(format, "mm/dd/yyyy") == 0) {
            strftime(formatted_time, sizeof(formatted_time), "%m/%d/%Y", time_info);
        } else if (strcmp(format, "mm/dd/yy") == 0) {
            strftime(formatted_time, sizeof(formatted_time), "%m/%d/%y", time_info);
        } else {
            strcpy(formatted_time, "Invalid format");
        }

        // Gửi thời gian cho client
        if (send(client_sock, formatted_time, strlen(formatted_time), 0) < 0) {
            perror("Error in send");
            exit(EXIT_FAILURE);
        }
    } else {
        // Lệnh không hợp lệ
        strcpy(formatted_time, "Invalid command");
        if (send(client_sock, formatted_time, strlen(formatted_time), 0) < 0) {
            perror("Error in send");
            exit(EXIT_FAILURE);
        }
    }

    // Đóng kết nối với client
    close(client_sock);
    exit(EXIT_SUCCESS);
}

int main() {
    int sockfd, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;

    // Tạo socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error in socket");
        exit(EXIT_FAILURE);
    }

    // Cấu hình thông tin server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Gán địa chỉ IP và cổng cho socket
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error in bind");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối từ client
    if (listen(sockfd, 5) < 0) {
        perror("Error in listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is running...\n");

    while (1) {
        // Chấp nhận kết nối từ client
        client_sock = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Error in accept");
            exit(EXIT_FAILURE);
        }

        // Xử lý client bằng multiprocessing
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error in fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Quá trình con xử lý client
            close(sockfd);
            handle_client(client_sock);
            exit(EXIT_SUCCESS);
        } else {
            // Quá trình cha đóng kết nối với client
            close(client_sock);
            wait(NULL);
        }
    }

    // Đóng socket
    close(sockfd);

    return 0;
}
