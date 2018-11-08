//
// Created by hrjanardhan on 4/8/18.
//

typedef struct env {
    int client_id;
    int storage_port_1;
    int storage_port_2;
    int storage_port_3;
} env;

void load_balance(struct env *, int);
void heartbeat_check();
void client_init();
int next_key(int key);