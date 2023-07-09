#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

char buf[2048];

int Get_Port_EPSV(client)
{
    send(client, "EPSV\r\n", 6, 0);
    int len;

    len = recv(client, buf, sizeof(buf), 0);
    buf[len] = 0;
    puts(buf);

    // Xac dinh gia tri cong cua kenh du lieu
    char *pos1 = strstr(buf, "|||") + 3;
    char *pos2 = strchr(pos1, '|');

    int value_length = pos2 - pos1;
    char value[16];
    memcpy(value, pos1, value_length);
    value[value_length] = 0;

    int port = atoi(value);
    return port;
}

int Get_Port_PASV(client)
{
    send(client, "pasv\r\n", 6, 0);
    int len;

    len = recv(client, buf, sizeof(buf), 0);
    buf[len] = 0;
    puts(buf);

    // Xac dinh gia tri cong cua kenh du lieu
    const char s[2] = ",";
    char *token;
    char toke2[50];
    int a, b;
    int i = 0;
    token = strtok(buf, s);
    while (token != NULL)
    {
        i++;
        if (i == 5)
        {
            a = atoi(token);
        }
        if (i == 6)
        {
            char *hi = strchr(token, ')');
            strncpy(toke2, token, sizeof(hi));
            b = atoi(toke2);
            break;
        }
        token = strtok(NULL, s);
    }
    int port = a * 256 + b;
    printf("PASV-PORT: %d\n", port);
    return port;
}

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("172.24.176.1");
    addr.sin_port = htons(21);

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("connect() failed");
        return 1;
    }

    // Nhan xau chao
    int len = recv(client, buf, sizeof(buf), 0);
    buf[len] = 0;

    puts(buf);

    char username[64], password[64];

    while (1)
    {
        printf("Nhap username: ");
        scanf("%s", username);
        printf("Nhap password: ");
        scanf("%s", password);

        // Gui lenh USER
        sprintf(buf, "USER %s\r\n", username);
        send(client, buf, strlen(buf), 0);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        // puts(buf);

        // Gui lenh PASS
        sprintf(buf, "PASS %s\r\n", password);
        send(client, buf, strlen(buf), 0);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        // puts(buf);

        if (strncmp(buf, "230", 3) == 0)
        {
            printf("Dang nhap thanh cong.\n");
            break;
        }
        else
        {
            printf("Dang nhap that bai. Hay nhap lai.\n");
        }
    }

    // Lay noi dung cua thu muc hien tai
    {
        // Gui lenh PASV
        int port = Get_Port_PASV(client);

        // Mo ket noi moi den kenh du lieu
        int data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        data_addr.sin_addr.s_addr = inet_addr("172.24.176.1");
        data_addr.sin_port = htons(port);

        if (connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr)))
        {
            perror("connect() failed");
            return 1;
        }

        // Gui lenh LIST
        send(client, "LIST\r\n", 6, 0);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        // Nhan du lieu o kenh du lieu
        while (1)
        {
            len = recv(data_socket, buf, sizeof(buf), 0);
            if (len <= 0)
                break;
            buf[len] = 0;
            puts(buf);
        }

        close(data_socket);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        printf("Noi dung thu muc truoc khi upload:\n");
        puts(buf);
    }

    // Download file
    {
        // Gui lenh EPSV
        int port = Get_Port_EPSV(client);
        // Gui lenh PASV
        //int port = Get_Port_PASV(client);

        // Mo ket noi moi den kenh du lieu
        int data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        data_addr.sin_addr.s_addr = inet_addr("172.24.176.1");
        data_addr.sin_port = htons(port);

        if (connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr)))
        {
            perror("connect() failed");
            return 1;
        }

        // Gui lenh STOR ten_file
        strcpy(buf, "STOR hello.txt\r\n");
        send(client, buf, strlen(buf), 0);

        // Phan hoi so 1 cua lenh STOR
        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);
        int ret1;
        // Nhan du lieu o kenh du lieu
        FILE *f = fopen("hello.txt", "rb");
        while (1)
        {
            ret1 = fread(buf, 1, sizeof(buf), f);
            if (ret1 <= 0)
                break;

            send(data_socket, buf, ret1, 0);
        }

        fclose(f);
        close(data_socket);

        // Phan hoi so 2 cua lenh STOR
        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);
    }

    {
        // Gui lenh EPSV
        int port = Get_Port_EPSV(client);
        // Gui lenh PASV
       //int port = Get_Port_PASV(client);

        // Mo ket noi moi den kenh du lieu
        int data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        data_addr.sin_addr.s_addr = inet_addr("172.24.176.1");
        data_addr.sin_port = htons(port);

        if (connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr)))
        {
            perror("connect() failed");
            return 1;
        }

        // Gui lenh LIST
        send(client, "LIST\r\n", 6, 0);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        // Nhan du lieu o kenh du lieu
        while (1)
        {
            len = recv(data_socket, buf, sizeof(buf), 0);
            if (len <= 0)
                break;
            buf[len] = 0;
            puts(buf);
        }

        close(data_socket);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        printf("Noi dung thu muc sau khi upload:\n");
        puts(buf);
    }
    // Gui lenh QUIT
    send(client, "QUIT\r\n", 6, 0);

    // Ket thuc, dong socket
    close(client);

    return 0;
}
