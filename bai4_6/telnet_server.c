#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 12345
#define BUFFER_SIZE 1024

// Kiểm tra xem user và pass có hợp lệ không
int isValidCredentials(const char* user, const char* pass) {
    FILE* fp = fopen("database.txt", "r");
    if (fp == NULL) {
        perror("Error opening database file");
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    int valid = 0;
    while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
        char storedUser[BUFFER_SIZE];
        char storedPass[BUFFER_SIZE];
        sscanf(buffer, "%s %s", storedUser, storedPass);
        
        if (strcmp(user, storedUser) == 0 && strcmp(pass, storedPass) == 0) {
            valid = 1;
            break;
        }
    }

    fclose(fp);
    return valid;
}

// Thực hiện lệnh và ghi kết quả vào file out.txt
void executeCommand(const char* command) {
    FILE* fp = fopen("out.txt", "w");
    if (fp == NULL) {
        perror("Error opening output file");
        exit(1);
    }

    dup2(fileno(fp), STDOUT_FILENO);
    system(command);

    fclose(fp);
}

int main() {
    int sockfd, newsockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];

    // Tạo socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(1);
    }

    // Thiết lập thông tin server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Gắn socket với địa chỉ server
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    // Lắng nghe kết nối từ client
    if (listen(sockfd, 5) < 0) {
        perror("Error listening on socket");
        exit(1);
    }

    printf("Server is running and listening on port %d...\n", PORT);

    while (1) {
        // Chấp nhận kết nối mới
        client_len = sizeof(client_addr);
        newsockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
        if (newsockfd < 0) {
            perror("Error accepting connection");
            exit(1);
        }

        // Gửi yêu cầu nhập user và pass cho client
        const char* loginPrompt = "Username: ";
        if (send(newsockfd, loginPrompt, strlen(loginPrompt), 0) < 0) {
            perror("Error sending data");
            exit(1);
        }

        memset(buffer, 0, BUFFER_SIZE);
        if (recv(newsockfd, buffer, BUFFER_SIZE - 1, 0) < 0) {
            perror("Error receiving data");
            exit(1);
        }
        char* user = strdup(buffer);

        const char* passPrompt = "Password: ";
        if (send(newsockfd, passPrompt, strlen(passPrompt), 0) < 0) {
            perror("Error sending data");
            exit(1);
        }

        memset(buffer, 0, BUFFER_SIZE);
        if (recv(newsockfd, buffer, BUFFER_SIZE - 1, 0) < 0) {
            perror("Error receiving data");
            exit(1);
        }
        char* pass = strdup(buffer);

        // Kiểm tra user và pass
        if (!isValidCredentials(user, pass)) {
            const char* loginError = "Invalid credentials";
            if (send(newsockfd, loginError, strlen(loginError), 0) < 0) {
                perror("Error sending data");
                exit(1);
            }
            close(newsockfd);
            continue;
        }

        const char* successMessage = "Login successful\n";
        if (send(newsockfd, successMessage, strlen(successMessage), 0) < 0) {
            perror("Error sending data");
            exit(1);
        }

        while (1) {
            // Nhận lệnh từ client
            memset(buffer, 0, BUFFER_SIZE);
            if (recv(newsockfd, buffer, BUFFER_SIZE - 1, 0) < 0) {
                perror("Error receiving data");
                exit(1);
            }

            // Thực hiện lệnh và gửi kết quả về client
            executeCommand(buffer);

            FILE* fp = fopen("out.txt", "r");
            if (fp == NULL) {
                perror("Error opening output file");
                exit(1);
            }

            while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
                if (send(newsockfd, buffer, strlen(buffer), 0) < 0) {
                    perror("Error sending data");
                    exit(1);
                }
            }

            fclose(fp);
        }

        // Đóng kết nối mới
        close(newsockfd);
    }

    // Đóng socket
    close(sockfd);

    return 0;
}
