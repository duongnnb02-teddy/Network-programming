#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define ROOT_DIR "root"
#define BUFFER_SIZE 1024
const char ch[2] = "/";

void *client_thread(void *);
void send_directory(int clien, const char *path);
void send_file(int client, char *path);
void send_response(int client, const char *response_header, const char *response_body);

void signal_handler(int signo)
{
    wait(NULL);
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    signal(SIGPIPE, signal_handler);

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
            ;
        }
        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(listener);

    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[2048];

    int ret = recv(client, buf, sizeof(buf) - 1, 0);
    if (ret <= 0)
        return NULL;

    buf[ret] = 0;
    printf("Received from %d: %s\n", client, buf);
    char method[16];
    char path[256];
    sscanf(buf, "%s %s", method, path);
    printf("URL: %s\n", path);
    char *file_name = strrchr(path, '/'); // Tìm vị trí cuối cùng của '/' trong chuỗi path
    char out[128];
    char *rett;
    rett = strchr(path + 1, '/');
   if (rett == NULL)
    {
        if (strlen(path) > 2)
        {
            strcpy(out, path + 1);
            printf("Ten file chuan: %s\n", out);
        }
        else
        {
            strcpy(out, path);
            printf("Ten file: %s\n", out);
        }
    }
    else
    {   
        if (strlen(file_name) < 2) {
            int a = strlen(path) - 2;
            int i = 0;
            while(path[a] != '/') {
                out[i] = path[a];
                a--;
                i++;
            }
            for(int a = 0; a<i/2; a++){
                char tg = out[a];
                out[a] = out[i-a-1];
                out[i-a-1] = tg;
            }
            out[i] = 0;
        }
        else{
            strcpy(out, file_name + 1);
        }
        printf("Ten thu muc: %s\n", out);
    }
    struct stat file_stat;
    if (stat(out, &file_stat) == -1)
    {
        send_response(client, "HTTP/1.1 404 Not Found", "Khong tim thay ten thu muc");
        return;
    }
    if (S_ISDIR(file_stat.st_mode))
    {
        // Gửi nội dung thư mục
        send_directory(client, out);
    }
    else if (S_ISREG(file_stat.st_mode))
    {
        // Gửi nội dung tệp

        send_file(client, out);
    }
    close(client);
}

void send_response(int client, const char *response_header, const char *response_body)
{
    char response[BUFFER_SIZE];

    snprintf(response, sizeof(response),
             "%s\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %lu\r\n"
             "\r\n"
             "%s",
             response_header, strlen(response_body), response_body);

    // Gửi phản hồi cho client
    send(client, response, strlen(response), 0);
}

void send_file(int client, char *path)
{
    char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";
    send(client, response_header, strlen(response_header), 0);
    char buf[128];
    FILE *f = fopen(path, "rb");
    while (1)
    {
        int len = fread(buf, 1, sizeof(buf), f);
        if (len <= 0)
            break;
        send(client, buf, len, 0);
    }
    fclose(f);
}

void send_directory(int client, const char *path)
{
    char response_content[BUFFER_SIZE];
    memset(response_content, 0, sizeof(response_content));
    printf("path thu muc: %s \n", path);
    if (*(path + 1) == NULL)
    {
        strcpy(path, ".");
    }
    DIR *dir = opendir(path);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        char entry_path[PATH_MAX];
        snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR)
        {
            // Thư mục
            snprintf(response_content + strlen(response_content), sizeof(response_content) - strlen(response_content),
                     "<p><strong><a href=\"%s/\">%s</a></strong></p>", entry->d_name, entry->d_name);
        }
        else if (entry->d_type == DT_REG)
        {
            // Tệp
            snprintf(response_content + strlen(response_content), sizeof(response_content) - strlen(response_content),
                     "<p><em><a href=\"%s\">%s</a></em></p>", entry->d_name, entry->d_name);
        }
    }

    closedir(dir);

    // Gửi phản hồi với nội dung thư mục
    send_response(client, "HTTP/1.1 200 OK", response_content);
}