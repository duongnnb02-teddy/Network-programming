#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>



int main(int argc, char *argv[])
{
    char ip[9];
    strcpy(ip, argv[1]);
    int port = atoi(argv[2]);
    printf("IP: %s\n", ip);
    printf("PORT: %d\n", port);

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
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
// client nhap du lieu
    printf("MSSV: ");
    char mssv[10];
    fgets(mssv, 10, stdin);
    mssv[strcspn(mssv, "\n")] = '\0';  

    printf("Ho va ten: ");
    char hoTen[20];
    fgets(hoTen, 20, stdin);
    hoTen[strcspn(hoTen, "\n")] = '\0';

    printf("Ngay sinh: ");
    char ngaySinh[15];
    fgets(ngaySinh, 15, stdin);
    ngaySinh[strcspn(ngaySinh, "\n")] = '\0';

    printf("Điem trung binh: ");
    char diemTrungBinh[6];
    fgets(diemTrungBinh, 6, stdin);
    diemTrungBinh[strcspn(diemTrungBinh, "\n")] = '\0';

    time_t rawtime;
    struct tm * time_str;
    time(&rawtime);
    time_str = localtime(&rawtime); 

    //encapsulation
    char data[123];
    snprintf(data, sizeof(data), " %d-%d-%d %d:%d:%d %s %s %s %s", 
            time_str->tm_mday, time_str->tm_mon + 1, time_str->tm_year + 1900,
            time_str->tm_hour, time_str->tm_min , time_str->tm_sec, 
            mssv, hoTen, ngaySinh, diemTrungBinh);
    send(client, data, strlen(data), 0);
    printf("Gui: %s den Port %d\n", data, port);
    
    close(client);
    
}