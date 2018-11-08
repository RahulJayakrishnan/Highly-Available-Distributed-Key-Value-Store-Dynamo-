//
// Created by hrjanardhan on 4/8/18.
//
#include <strings.h>
#ifndef SERVER_GT_STORE_H
#define SERVER_GT_STORE_H

typedef struct cart {
    int key;
    int version;
    char items[128];
} cart;

typedef struct message {
    char flag;
    int key;
    char cart[128];
} message;

void add_to_cart(message msg);
void send_to_client(message msg);

#endif //SERVER_GT_STORE_H
