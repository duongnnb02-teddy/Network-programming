#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>

struct myStructure {
  int Num;
  char ID[5];
  char Name[11];
};

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

    fd_set fdread;

    char STC[] = "ID: NAME?\n";
    char fail[] = "fail! send again.\n";
    char buf[128];
    int client[64];
    int num_clientnumber = 0;

    while (1)
    {
        FD_ZERO(&fdread);

        FD_SET(listener, &fdread);

        int max = listener + 1;

        for (int i = 0; i < num_clientnumber; i++)
        {
            FD_SET(client[i], &fdread);
            if(max < client[i] + 1) max = client[i] + 1;
        }

       struct myStructure client_array[5]; 
        int ret = select(max, &fdread, NULL, NULL, NULL);
        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }
        
         if (FD_ISSET(listener, &fdread))
        {   
            int clients = accept(listener, NULL, NULL);
            printf("Ket noi moi: %d\n", clients);
            char bufrecv[30];
            while (1)
            {   
                char ID1[4];
                char Name1[10];
                char test[5];
                send(clients, STC, sizeof(STC), 0);
                int ret2 = recv(clients, bufrecv, sizeof(bufrecv), 0);
                bufrecv[ret2] = 0;
                int check = sscanf(bufrecv, "%s %s %s", ID1, Name1, test);
                if(check == 2) 
                {
                    printf("%s\n", bufrecv);
                    client_array[num_clientnumber].Num = clients;
                    strcpy(client_array[num_clientnumber].Name, Name1);
                    strcpy(client_array[num_clientnumber].ID, ID1);
                    
                    client[num_clientnumber++] = clients;
                    break;
                }
                else
                {
                    send(clients, fail, sizeof(fail), 0);
                }
                
            }
            
        }

        for (int i = 0; i < num_clientnumber; i++)
        {
            if(FD_ISSET(client[i], &fdread)) {
                int ret1 = recv(client[i], buf, sizeof(buf), 0);
                 if (ret1 <= 0)
                {
                    continue;
                }
                buf[ret1] = 0;
                printf("Du lieu nhan tu %d: %s\n", client[i], buf);
                strcat(client_array[i].ID, buf );
                for (int j = 0; j < num_clientnumber; j++)  
                {
                    if(j == client[i] - 4) continue;
                    send(client[j], client_array[i].ID, strlen(client_array[i].ID), 0);
                    printf("sent to %d\n", client[j]);
                }
                
            }
        }
        
    }
    

    close(listener);    

    return 0;
}
