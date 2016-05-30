#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "mstg_mp3.h"

#define STDIN_BUF_SIZE 64 

enum mstg_mode {
    NONE,
    CONCEAL,
    REVEAL
};

void print_usage(char* prog_path);


int main(int argc, char** argv) {

    enum mstg_mode mode = NONE;
    char *mp3_path = NULL;
    uint32_t key = 0;
    int opt;
    while((opt = getopt(argc, argv, "crm:k:")) != -1) {
        switch(opt) {
            case 'c':
                mode = CONCEAL;
                break;
            case 'r':
                mode = REVEAL;
                break;
            case 'm':
                mp3_path = optarg;
                break;
            case 'k':
                key = atoi(optarg);
                break;
            default:
                print_usage(argv[0]);
                exit(1);
        }
    }

    if(mode == NONE || !mp3_path || !key) {
        print_usage(argv[0]);
        exit(1);
    }
    else if(mode == CONCEAL) {
        void *inp_buf = malloc(STDIN_BUF_SIZE);
        void *message = NULL;
        size_t total_size = 0;

        mstg_mp3_t *mp3 = mstg_mp3_new(mp3_path);
    
        while(!feof(stdin)) {
            size_t bytes_read = fread(inp_buf, sizeof(char), STDIN_BUF_SIZE, stdin); 
            total_size += bytes_read;
            message = realloc(message, total_size);
            memcpy(message+total_size-bytes_read, inp_buf, bytes_read);
        }
        mstg_mp3_write_message(mp3, (void*)message, total_size, key);
        mstg_mp3_writeout(mp3);

        free(inp_buf);
        free(message);
    }
    else {
        void* msg = NULL;
        mstg_mp3_t *mp3 = mstg_mp3_new(mp3_path);

        size_t msg_len = mstg_mp3_read_message(mp3, key, &msg);
        fwrite(msg, 1, msg_len, stdout);
    }

    return 0;
}

void print_usage(char* prog_path) {
    fprintf(stderr,
            "Usage:\n%s -c -m $mp3 -k $key to conceal message\n%s -r -m $mp3 -k $key to reveal",
            prog_path,
            prog_path);
}

