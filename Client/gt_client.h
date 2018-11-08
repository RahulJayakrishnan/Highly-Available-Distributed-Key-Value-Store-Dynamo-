//
// Created by hrjanardhan on 4/8/18.
//

#ifndef CLIENT_GT_CLIENT_H
#define CLIENT_GT_CLIENT_H

// Uncomment init
// Change port number to env1->port1

typedef struct env {
    int client_id;
    int storage_port_1;
    int storage_port_2;
    int storage_port_3;
    char cart[128];
} env;

typedef struct message {
    char flag;
    int key;
    char cart[128];
} messsage;

typedef struct cart {
    int key;
    int version;
    char items[128];
} cart;

void init(struct env *env1);
void put(struct env *env1, int key, const char *value);
void get(struct env *env1, int key);
void reconcile(struct env *env1, int serv_port, int key);
void finalize(struct env *env1);

#endif //CLIENT_GT_CLIENT_H
