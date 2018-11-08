#include <iostream>
#include "gt_store.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fstream>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <vector>

#define PORT 40000

using namespace std;
int port = PORT;

std::vector <cart> carts;
int listenfd = 0, connfd = 0;

int main(int argc, char *argv[])
{
    if(argc != 2) {
        return 1;
    }
    else {
        port += atoi(argv[1]);
    }
    printf("Started server at port: %d\n", port);

    struct sockaddr_in serv_addr;

    char sendBuff[1025];
    time_t ticks;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);


//    while(1)
//    {
//        int n = 0;
//        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
//        message buf;
//        if((n = read(connfd, &buf, sizeof(message))) > 0){
//            printf("Client ID: %d\n", buf.key);
//            printf("TYPE:%c\n", buf.flag);
//        }
//        close(connfd);
//        sleep(1);
//    }

    //////////
    while(1) {
        message msg1;
        int n = 0;
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        while ( (n = read(connfd, &msg1, sizeof(message))) > 0)
        {
            // 'p' - PUT ;'g' - GET
            if(msg1.flag == 'p') {
                add_to_cart(msg1);
            }
            if(msg1.flag == 'g') {
                send_to_client(msg1);
            }
        }

    }
}


void add_to_cart(message msg1) {
    std::vector<cart>::iterator iter;
    for (iter = carts.begin(); iter < carts.end(); ++iter) {
        if (msg1.key == iter->key) {
            ++iter->version;
            strcpy(iter->items, msg1.cart);
            printf("Updated cart\n");
            printf("Key: %d | Version: %d | Items: %s\n", iter->key, iter->version, iter->items);
            return;
        }
    }
    cart cart1;
    strcpy(cart1.items, msg1.cart);
    cart1.version = 0;
    cart1.key = msg1.key;
    carts.push_back(cart1);

    printf("Added to cart\n");
    printf("Key: %d | Version: %d | Items: %s\n", cart1.key, cart1.version, cart1.items);
}

void send_to_client(message msg1) {
    std::vector<cart>::iterator iter;
    cart myCart;
    for (iter = carts.begin(); iter < carts.end(); ++iter) {
        if (msg1.key == iter->key) {
            memcpy(&myCart, &*iter, sizeof(cart));
            write(connfd, &myCart, sizeof(cart));
            return;
        }
    }

    myCart.version = -1;
    strcmp(myCart.items, "Cart is empty");
    write(connfd, &myCart, sizeof(cart));

}