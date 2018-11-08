#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fstream>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "gt_coord.h"
#include <thread>
#include <vector>
#include <sys/time.h>

#define PORT 50000
#define STORAGE_PORT 40000
#define NUM_STR_NODES 200

using namespace std;
env env1;
int coord_port;

bool active[NUM_STR_NODES];
bool cur_status[NUM_STR_NODES];
uint8_t load[NUM_STR_NODES];

int main(int argc, char *argv[])
{
    coord_port = PORT;
    if(argc != 2) {
        return 1;
    }
    else {
        coord_port += atoi(argv[1]);
    }

    struct sockaddr_in serv_addr;

    for (int i = 0; i < NUM_STR_NODES; ++i) {
        active[i] = true;
    }
    for (int i = 0; i < NUM_STR_NODES; ++i) {
        cur_status[i] = false;
    }


    std::vector<std::thread *> threads;
    threads.push_back(new std::thread{heartbeat_check});
    threads.push_back(new std::thread{client_init});

    for(int ii = 0; ii < 2; ii++) {
        threads[ii]->join();
    }
    return 0;
}

void heartbeat_check() {
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr;

    while(1) {

        for (int ii = 0; ii < NUM_STR_NODES; ii++) {
            timeval start, end, diff;
            gettimeofday(&start, NULL);

            memset(recvBuff, '0', sizeof(recvBuff));

            if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                printf("\n Error : Could not create socket \n");
            }

            memset(&serv_addr, '0', sizeof(serv_addr));

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(40000 + ii);

            if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
                printf("\n inet_pton error occured\n");
                return;
            }

            if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
                if(cur_status[ii] != active[ii]) {
                    cur_status[ii] = active[ii];
                    printf("\n Error : Connect Failed: Storage Node %d \n", ii);
                }
//                printf("\n Error : Connect Failed: Storage Node %d \n", ii);
                active[ii] = false;
            }

            else {
                if(cur_status[ii] != active[ii]) {
                    cur_status[ii] = active[ii];
                    printf("Connected to %d\n", ii);
                }
//                printf("Connected to %d\n", ii);
                active[ii] = true;
            }
            close(sockfd);
            gettimeofday(&end, NULL);
            timersub(&end, &start, &diff);
//            printf("Time to connect: %lu sec %lu usec\n", diff.tv_sec, diff.tv_usec);
            usleep(10000);
        }


    }
}

void client_init() {
    int listenfd = 0, connfd = 0, client_id, key;
    struct sockaddr_in serv_addr;

    time_t ticks;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(coord_port);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);

    // Receving client request, and return storage nodes' adresses
    // Maintain load across the storage nodes

    while(1) {

        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        read(connfd, &client_id, sizeof(int));
        printf("Client ID: %d\n", client_id);

        load_balance(&env1, client_id);

        write(connfd, &env1, sizeof(env));

        close(connfd);
//        sleep(1);
    }
}

void load_balance(struct env *env1, int client_id) {
    int key[3];
    int temp, cur_key;

    env1->client_id = client_id;
    // Find load, and fill port nos

    cur_key = client_id % NUM_STR_NODES;
    cur_key = next_key(cur_key);
    key[0] = cur_key;

    cur_key = next_key(cur_key + 1);
    key[1] = cur_key;

    cur_key = next_key(cur_key + 1);
    key[2] = cur_key;

    temp = key[0];
    int idx = 0;
    for(int ii = 1; ii < 3; ++ii) {
        if(load[temp] > load[key[ii]]) {
            temp = key[ii];
            idx = ii;
        }
    }
    ++load[key[idx]];
    if(idx != 0) {
        int tmp = key[idx];
        key[idx] = key[0];
        key[0] = tmp;
    }

    env1->storage_port_1 = STORAGE_PORT + key[0];
    env1->storage_port_2 = STORAGE_PORT + key[1];
    env1->storage_port_3 = STORAGE_PORT + key[2];
    printf("Port 1: %d\n", env1->storage_port_1);
    printf("Port 2: %d\n", env1->storage_port_2);
    printf("Port 3: %d\n", env1->storage_port_3);
}

int next_key(int key) {
    int ii = 0;
    while(ii < NUM_STR_NODES) {
        if(active[(key + ii) % NUM_STR_NODES]) {
            printf("NEXT_KEY is returning: %d\n", (key + ii) % NUM_STR_NODES);
            return (key + ii) % NUM_STR_NODES;
        }
        ++ii;
    }
}