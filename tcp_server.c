#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

// server nhap tu ban phim.

int main(int argc, char *argv[])
{   
    char port[4];
    strcpy(port, argv[1]);
    printf("PORT: %s\n", port);
    printf("filename: %s\n", argv[2]);
    printf("filenameserver: %s\n", argv[3]);
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(port));

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
    
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(addr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("Client IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    char filename[20];
    strcpy(filename, argv[2]);
    FILE *f = fopen(filename, "rb");

    char buf[256];
    int ret;
    while (!feof(f))
    {
        ret = fread(buf, 1, sizeof(buf), f);
        if (ret <= 0)
            break;
        send(client, buf, ret, 0);
    }
    
    char bufget[128];
    sprintf(bufget, "%s", argv[3]);
    FILE *fh = fopen(bufget, "wb");
    int retget = recv(client, bufget, sizeof(bufget), 0);
    if (retget <= 0) {printf("failed recv\n");}
    else { 
        fwrite(bufget, 1, retget, fh); 
        fclose(fh);
        printf("%d bytes nhan: %s\n", retget, bufget);
        printf("Luu vao file: %s\n", argv[3]);
    }

    fclose(f);
    close(client);
    close(listener);
}