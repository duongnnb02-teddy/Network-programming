#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main()
{
    int sockfd, newsockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];

    // Tạo socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    // Thiết lập thông tin server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Gắn socket với địa chỉ server
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding socket");
        exit(1);
    }

    // Lắng nghe kết nối từ client
    if (listen(sockfd, 5) < 0)
    {
        perror("Error listening on socket");
        exit(1);
    }

    printf("Server is running and listening on port %d...\n", PORT);

    // Khởi tạo server 
    while(1)
    {
        // Chờ kết nối mới
        int client = accept(listener, NULL, NULL);
        printf("New client connected: %d\n", client);
        // Nhận dữ liệu từ client và in ra màn hình
        char buf[256];
        int ret = recv(client, buf, sizeof(buf), 0);
        buf[ret] = 0;
        puts(buf);
        // Trả lại kết quả cho client
        char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin 
            chao cac ban</ h1></ body></ html> ";
            send(client, msg, strlen(msg), 0);
        // Đóng kết nối
        close(client);
    }
}
