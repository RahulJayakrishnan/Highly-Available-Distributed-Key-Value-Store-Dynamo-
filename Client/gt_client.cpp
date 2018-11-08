//
// Created by hrjanardhan on 4/8/18.
//
#include "gt_client.h"
#include <iostream>
#include "gt_client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int coord_port = 50000;

void init(struct env *env1) {
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr;

    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(coord_port);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return;
    }

    int tries = 0;

    label1:if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        ++tries;
        printf("\n Error : Connect Failed \n");
        ++coord_port;
        serv_addr.sin_port = htons(coord_port);

        if(tries > 3) {
            printf("Unable to connect to any coordinator\n");
            exit(0);
        } else {
            goto label1;
        }

    }

    write(sockfd, &(env1->client_id), sizeof(int));
    while ( (n = read(sockfd, env1, sizeof(env) - 128)) > 0)
    {
        printf("Priority 1: %d\n", env1->storage_port_1);
        printf("Priority 2: %d\n", env1->storage_port_2);
        printf("Priority 3: %d\n", env1->storage_port_3);

    }
    close(sockfd);
}

void put(struct env *env1, int key, const char *value) {
    int sockfd = 0, n = 0;
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(env1->storage_port_1);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return;
    }

    // Change port here
    label1:if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Node failure detected. Re-evaluating node priorities\n");
        init(env1);
        serv_addr.sin_port = htons(env1->storage_port_1);
        goto label1;
//        return;
    }

    ///// PUT Operation
    message msg1;
    msg1.flag = 'p';
    msg1.key = key;
    strcpy(msg1.cart, value);

    write(sockfd, &msg1, sizeof(message));
    close(sockfd);

    ////////////////// WRITTEN TO ONE NODE //////////////////////////////////////////
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return;
    }

    serv_addr.sin_port = htons(env1->storage_port_2);
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return;
    }

    // Change port here
   label2: if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Node failure detected. Re-evaluating node priorities\n");
        init(env1);
        serv_addr.sin_port = htons(env1->storage_port_2);
        goto label2;
    }

    ///// PUT Operation
    msg1.flag = 'p';
    msg1.key = key;
    strcpy(msg1.cart, value);

    write(sockfd, &msg1, sizeof(message));
    close(sockfd);
    printf("Put operation complete\n");
}

void get(struct env *env1, int key) {
    int sockfd = 0, n = 0, count = 0;
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(env1->storage_port_1);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return;
    }

    // Change port here
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Unable to connect\n");
//        return;
    }

    ///// GET Operation
    message msg;
    msg.flag = 'g';
    msg.key = key;

    write(sockfd, &msg, sizeof(message));
    cart myCart1, myCart2;
    myCart1.version = -1;
    myCart2.version = -1;
    myCart1.key = -1;
    myCart2.key = -1;

    while ( (n = read(sockfd, &myCart1, sizeof(cart))) > 0)
    {
        printf("Sourced from  %d | Version: %d, %s\n", env1->storage_port_1, myCart1.version, myCart1.items);
        myCart1.key = env1->storage_port_1;
        ++count;
        break;
    }
    close(sockfd);


    // Read from second server
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return;
    }
    serv_addr.sin_port = htons(env1->storage_port_2);
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Unable to connect\n");
    }

    write(sockfd, &msg, sizeof(message));
    if(count) {
        while ( (n = read(sockfd, &myCart2, sizeof(cart))) > 0)
        {
            printf("Sourced from  %d | Version: %d, %s\n", env1->storage_port_2, myCart2.version, myCart2.items);
            myCart2.key = env1->storage_port_2;
            ++count;
            break;
        }
    }

    if(!count) {
        while ( (n = read(sockfd, &myCart1, sizeof(cart))) > 0)
        {
            printf("Sourced from  %d | Version: %d, %s\n", env1->storage_port_1, myCart1.version, myCart1.items);
            myCart1.key = env1->storage_port_1;
            ++count;
            break;
        }
    }
    close(sockfd);

    // Reading from server 3
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return;
    }
    serv_addr.sin_port = htons(env1->storage_port_3);

    if(count != 2) {
        if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("Unable to connect\n");
        }

        write(sockfd, &msg, sizeof(message));
        while ( (n = read(sockfd, &myCart2, sizeof(cart))) > 0)
        {
            printf("Sourced from  %d | Version: %d, %s\n", env1->storage_port_3, myCart2.version, myCart2.items);
            myCart2.key = env1->storage_port_3;
            ++count;
            break;
        }

    }
    close(sockfd);

    if(myCart1.version < myCart2.version) {
        strcpy(env1->cart, myCart2.items);
        if(strcmp(myCart1.items, myCart2.items)) {
            printf("Up to date value at: %d, reconciling server at: %d\n", myCart2.key, myCart1.key);
            reconcile(env1, myCart1.key, key);
        }
    }

    else if(myCart2.version < myCart1.version) {
        strcpy(env1->cart, myCart1.items);
        if(strcmp(myCart1.items, myCart2.items)) {
            printf("Up to date value at: %d, reconciling server at: %d\n", myCart1.key, myCart2.key);
            reconcile(env1, myCart2.key, key);
        }
    }

    else {
        strcpy(env1->cart, myCart2.items);
        printf("Both versions up to date\n");
    }
}

void reconcile(struct env *env1, int serv_port, int key) {
    int sockfd = 0, n = 0;
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return;
    }

    // Change port here
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Node failure detected. Re-evaluating node priorities\n");
    }

    ///// PUT Operation
    message msg1;
    msg1.flag = 'p';
    msg1.key = key;
    strcpy(msg1.cart, env1->cart);

    write(sockfd, &msg1, sizeof(message));
    close(sockfd);
}

void finalize(struct env *env1) {
    env1 = NULL;
}
