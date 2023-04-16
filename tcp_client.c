#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{   int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    char ip[9];
    strcpy(ip, argv[1]);
    int port = atoi(argv[2]);
    printf("IP: %s\n", ip);
    printf("PORT: %d\n", port);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);


    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect() failed");
        return 1;
    }

    char buf[256];
    sprintf(buf, "new.txt");
    FILE *f = fopen(buf, "wb");
   
    int ret1;
    ret1 = recv(client, buf, sizeof(buf), 0);
    fwrite(buf, 1, ret1, f);
    printf("%s\n", buf);
         

    //client gui du lieu den server
    char bufsend[128];
    printf("Nhap xau:");
    fgets(bufsend, sizeof(bufsend), stdin);
    send(client, bufsend, strlen(bufsend), 0);

    fclose(f);
    close(client);
}