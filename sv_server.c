#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{   
    char port[4];
    strcpy(port, argv[1]);
    printf("PORT: %s\n", port);
    
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
    
    
    
    
    char clientIP[258];
    strcpy(clientIP, inet_ntoa(clientAddr.sin_addr));
    
    char bufget[128];
    FILE *f = fopen("sv_log.txt", "wb");
    int retget = recv(client, bufget, sizeof(bufget), 0);
    strcat(clientIP, bufget);
    if (retget <= 0) {printf("failed recv\n");}
    else { 
        fwrite(clientIP, 1, sizeof(clientIP), f); 
        fclose(f);
        printf("Nhận: %s\n", clientIP);
        printf("Lưu vào: sv_log.txt.\n");
    }

    close(client);
    close(listener);
}
