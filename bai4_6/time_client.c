#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024
#define SERVER_PORT 8888

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE];
    char format[MAX_BUFFER_SIZE];

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
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Địa chỉ IP của time_server

    // Kết nối tới server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error in connect");
        exit(EXIT_FAILURE);
    }

    // Nhập định dạng thời gian từ người dùng
    printf("Enter the time format: ");
    fgets(format, sizeof(format), stdin);
    format[strcspn(format, "\n")] = '\0'; // Xóa ký tự xuống dòng

    // Gửi lệnh tới server
    sprintf(buffer, "GET_TIME %s", format);
    if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
        perror("Error in send");
        exit(EXIT_FAILURE);
    }

    // Nhận thời gian từ server
    memset(buffer, 0, sizeof(buffer));
    if (recv(sockfd, buffer, sizeof(buffer), 0) < 0) {
        perror("Error in recv");
        exit(EXIT_FAILURE);
    }

    // In ra thời gian nhận được
    printf("Time from server: %s\n", buffer);

    // Đóng kết nối
    close(sockfd);

    return 0;
}
