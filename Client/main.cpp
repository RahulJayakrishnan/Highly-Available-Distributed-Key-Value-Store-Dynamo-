#include <iostream>
#include "gt_client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
// using namespace std;

using namespace std;

int main(int argc, char *argv[])
{
    env env1;


    if(argc != 2)
    {
        printf("Client ID not entered\n");
        return 1;
    }

    int client_id = atoi(argv[1]);
    printf("Client ID: %d\n", client_id);
    env1.client_id = client_id;

    timeval start, end, diff;

    init(&env1);

    char items[128] = "Pen Drive, Mouse";



    put(&env1, client_id, items);



    sleep(1);

    get(&env1, client_id);

    sleep(1);

    strcpy(items, "Toothpaste, Soap");
    put(&env1, client_id, items);
    sleep(1);
    get(&env1, client_id);
    printf("*** Hint: Crash the servers now to test fault tolerance! ***\n");
    sleep(6);

    get(&env1, client_id);
    sleep(1);
    strcpy(items, "Book, Pen");
    put(&env1, client_id, items);
    sleep(1);


    strcpy(items, "Echo Dot, Google Home");
    put(&env1, client_id, items);
    sleep(1);
    get(&env1, client_id);
    sleep(1);

    strcpy(items, "iPhone X, MacBook");
    put(&env1, client_id, items);
    sleep(1);
    get(&env1, client_id);

    finalize(&env1);

    return 0;
}
