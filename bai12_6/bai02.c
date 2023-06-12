#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>

struct myStructure
{
    int Num;
    char ID[10];
};

char STC[] = "ID?\n";
char fail[] = "Pls wait for another client\n";
char sen[] = "Pls write to send\n";

int client[64];
int num_clientnumber = 0;
struct myStructure client_array[2];

pthread_mutex_t num_clientnumber_m = PTHREAD_MUTEX_INITIALIZER;

void *client_thread(void *);

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

    while (1)
    {
        int clients = accept(listener, NULL, NULL);
        if (clients == -1)
        {
            perror("accept() failed");
            continue;
            ;
        }
        printf("Ket noi moi: %d\n", clients);
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &clients);
        pthread_detach(thread_id);
    }
    close(listener);
    return 0;
}

void *client_thread(void *param)
{
    int clients = *(int *)param;
    char bufrecv[30];
    send(clients, STC, sizeof(STC), 0);
    int ret2 = recv(clients, bufrecv, sizeof(bufrecv), 0);
    bufrecv[ret2 - 1] = 0;
    printf("%s\n", bufrecv);
    client_array[num_clientnumber].Num = clients;
    client[num_clientnumber] = clients;
    strcpy(client_array[num_clientnumber].ID, bufrecv);
    strcat(client_array[num_clientnumber].ID, ": ");
    int a = num_clientnumber;

    num_clientnumber++;
    printf("so luong nguoi dang ket noi: %d\n", num_clientnumber);
    int b = 1;
    while (num_clientnumber == 1)
    {
        if (b == 1)
        {
            send(clients, fail, sizeof(fail), 0);
            b++;
        }
    }

    send(clients, sen, sizeof(sen), 0);
    while (1)
    {
        char ID_client[10];
        strcpy(ID_client, client_array[a].ID);
        char buf[256];
        int ret1 = recv(clients, buf, sizeof(buf), 0);
        if (ret1 <= 0)
        {
            continue;
        }
        buf[ret1] = 0;
        printf("Du lieu nhan tu %d: %s\n", clients, buf);
        if (strncmp(buf, "exit", 4) == 0)
        {
            num_clientnumber = 0;
            close(client[0]);
            close(client[1]);
            break;
        }
        strcat(ID_client, buf);
        if (a == 0)
        {
            send(client[1], ID_client, strlen(ID_client), 0);
            printf("sent to %d\n", client[1]);
        }

        if (a == 1)
        {
            send(client[0], ID_client, strlen(ID_client), 0);
            printf("sent to %d\n", client[0]);
        }
    }

    pthread_exit(param);
}
