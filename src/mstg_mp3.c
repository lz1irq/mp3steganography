#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <arpa/inet.h>

#include "mstg_mp3.h"
#include "mstg_util.h"


struct _mstg_mp3_t {
    char* file_path;
    FILE* file;

    off_t size;
    size_t position;
    char* bytes;

    int32_t tag_size;
    uint32_t frame_size;
};

uint32_t bitrate_map[] = {0, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000,
192000, 224000, 256000, 320000, 0};
uint32_t samplerate_map[] = {44100, 48000, 32000, 0};

mstg_mp3_t* mstg_mp3_new(const char* file_path) {
    mstg_mp3_t *mp3 = (mstg_mp3_t*)malloc(sizeof(mstg_mp3_t));
    if (!mp3) return NULL;

    mp3->tag_size = 0;
    mp3->frame_size = 0;

    mp3->file_path = (char*)malloc(strlen(file_path)*sizeof(char)+1);
    if (!mp3->file_path) return NULL;
    strcpy(mp3->file_path, file_path);

    mp3->size = get_file_size(mp3->file_path);
    mp3->file = fopen(mp3->file_path, "rb");
    if(!mp3->file) return NULL;
    mp3->bytes = read_file(mp3->file, mp3->size);
    if(!mp3->bytes) return NULL;
    fclose(mp3->file);
    mp3->position = 0;

    mstg_mp3_parse_tag_size(mp3);
    mstg_mp3_frame_size(mp3);
    return mp3;
}

size_t mstg_mp3_writeout(mstg_mp3_t *mp3) {
    mp3->file = fopen(mp3->file_path, "wb+");
    size_t bytes_written = fwrite(mp3->bytes, 1, mp3->size, mp3->file);
    fclose(mp3->file);
    return bytes_written;
}

int8_t mstg_mp3_write_message(mstg_mp3_t *mp3, char* msg, size_t msg_len, uint32_t key) {
    srand(key);
    size_t default_write_size = mp3->frame_size - MP3_FRAME_HEADER_SIZE;
    uint32_t max_frames = (mp3->size - mp3->tag_size)/mp3->frame_size;
    uint32_t frames = (msg_len + default_write_size -1)/default_write_size;

    printf("FRAMES: %d\n", frames);

    if(frames > max_frames) {
        return -1; 
    }
    printf("LEN: %d\n", msg_len);
    for(uint32_t i=0; i<frames; i++) {
        uint32_t frame_id = rand() % max_frames;
        
        size_t write_size = 0;
        if (msg_len >= default_write_size) write_size = default_write_size;
        else write_size = msg_len;

        mstg_mp3_write_frame(mp3, frame_id, msg, write_size);
        msg_len -= write_size;
    }
    return 1;
}

int8_t mstg_mp3_write_frame(mstg_mp3_t *mp3, uint32_t frame_number, char* msg, size_t msg_len) {
    printf("WRITE_FRAME: FRAME %d, MESSAGE %s, LENGTH %d\n", frame_number, msg, msg_len);
    uint32_t write_pos = frame_number*(mp3->frame_size) + MP3_FRAME_HEADER_SIZE;
    if (mp3->size - write_pos >= msg_len) {
        memcpy(mp3->bytes + write_pos, msg, msg_len);
        return 0;
    }
    else {
        return -1;
    }
}

uint32_t mstg_mp3_frame_size(mstg_mp3_t *mp3) {
    if (!mp3->frame_size) {
        uint32_t bitrate = 0;
        uint32_t sample_rate = 0;
        uint8_t padding = 0;
        
        uint8_t byte1 = 0, byte2 = 0;
        while(1) {
            byte1 = mp3->bytes[mp3->position++];
            byte2 = mp3->bytes[mp3->position++];
            if(byte1 == MP3_FRAME_HEADER_BYTE0 && byte2 == MP3_FRAME_HEADER_BYTE1) break;
        } 
        
        byte1 = mp3->bytes[mp3->position];
        bitrate = bitrate_map[byte1 >> 4];
        
        byte2 = byte1;
        byte1 = byte1 << 4;
        byte1 = byte1 >> 6;
        sample_rate = samplerate_map[byte1];

        byte2 = byte2 << 7;
        padding = byte2 >> 7;
        mp3->frame_size = (144.0*(bitrate/sample_rate)) + padding;
    } 
    return mp3->frame_size;
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
        mp3->position = size;
    }
    return size;
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


