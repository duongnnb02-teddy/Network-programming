#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>

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

    char STC[] = "Name and Pass?\n";
    char fail[] = "fail Name or Pass! Pls send again.\n";
    char acc[] = "Logged in successfully.\n";
    char buf1[128];
    int client[64];
    int num_clientnumber = 0;
    FILE *f = fopen("user.txt", "rb");
    char buf[256];
    fread(buf, 1, sizeof(buf), f);
   // printf("buffer: %s\n", buf);

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
                char ID1[10];
                char Name1[10];
                char test[5];
                send(clients, STC, sizeof(STC), 0);
                int ret2 = recv(clients, bufrecv, sizeof(bufrecv), 0);
                bufrecv[ret2] = 0;
                int check = sscanf(bufrecv, "%s %s %s", ID1, Name1, test);
                printf("bufrecv: %s\n", bufrecv);
                if(strstr(buf, bufrecv) != NULL && check == 2) 
                {   
                    client[num_clientnumber++] = clients;
                    send(clients, acc, sizeof(acc), 0);
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
                int ret1 = recv(client[i], buf1, sizeof(buf1), 0);
                if (ret1 <= 0)
                {
                    continue;
                }
                char tmp[32];
                buf1[ret1 - 1] = 0;
                printf("Du lieu nhan tu %d: %s\n", client[i], buf1);
                sprintf(tmp, "%s > out.txt", buf1);
                    printf("%s\n", tmp);
                    system(tmp);
                    FILE *fs = fopen("out.txt", "rb");
                    char bufsend[256];
                    int ret;
                    while (!feof(fs))
                    {
                        ret = fread(bufsend, 1, sizeof(bufsend), fs);
                        if (ret <= 0)
                            break;
                        send(client[i], bufsend, ret, 0);
                    }
                    fclose(fs);

                
            }
        }
        
    }
    
    fclose(f);

    close(listener);    

    return 0;
}
