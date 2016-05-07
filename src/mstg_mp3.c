#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "mstg_mp3.h"
#include "mstg_util.h"


struct _mstg_mp3_t {
    char* file_path;
    FILE* file;
    off_t size;
    char* bytes;
    int32_t tag_size;
    size_t sample_size;
};

mstg_mp3_t* mstg_mp3_new(const char* file_path) {
    mstg_mp3_t *mp3 = (mstg_mp3_t*)malloc(sizeof(mstg_mp3_t));
    if (!mp3) return NULL;

    mp3->tag_size = 0;
    mp3->sample_size = 0;

    mp3->file_path = (char*)malloc(strlen(file_path)*sizeof(char)+1);
    if (!mp3->file_path) return NULL;
    strcpy(mp3->file_path, file_path);

    mp3->size = get_file_size(mp3->file_path);
    mp3->file = fopen(mp3->file_path, "rb");
    if(!mp3->file) return NULL;
    mp3->bytes = read_file(mp3->file, mp3->size);
    if(!mp3->bytes) return NULL;

    mstg_mp3_parse_tag_size(mp3);

    return mp3;
}

uint32_t mstg_mp3_get_tag_size(mstg_mp3_t *mp3) {
    return mp3->tag_size;
}

uint32_t mstg_mp3_parse_tag_size(mstg_mp3_t *mp3) {
    if(!mp3) return 0;
    uint32_t size = 0;
    if (mp3->tag_size) { /* don't need to read the size again if we've got it already */
        return mp3->tag_size;
    }
    if (!strncmp(mp3->bytes, ID3V2_IDENTIFIER, strlen(ID3V2_IDENTIFIER))) {
        uint32_t synchsafe_size = 0;
        memcpy(&synchsafe_size, mp3->bytes+ID3V2_SIZE_OFFSET, sizeof(uint32_t));
        /* using htonl() because the size is stored in big-endian order */
        size = unsynchsafe(htonl(synchsafe_size)) + ID3V2_HEADER_SIZE;
        mp3->tag_size = size;
    }
}

int unsynchsafe(int in) {
    int out = 0, mask = 0x7F000000;

    while (mask) {
            out >>= 1;
            out |= in & mask;
            mask >>= 8;
        }

    return out;
}


