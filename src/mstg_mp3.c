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

    size_t tag_size;
    size_t frame_size;
    size_t rw_size;
    size_t max_frames;
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

    mstg_mp3_tag_size(mp3);
    mstg_mp3_frame_size(mp3);
    mp3->rw_size= mp3->frame_size - MP3_FRAME_HEADER_SIZE;
    mp3->max_frames = (mp3->size - mp3->tag_size)/mp3->rw_size;
    return mp3;
}

void mstg_mp3_close(mstg_mp3_t *mp3) {
    free(mp3->file_path);
    free(mp3->bytes);
    free(mp3);
}

size_t mstg_mp3_writeout(mstg_mp3_t *mp3) {
    mp3->file = fopen(mp3->file_path, "wb+");
    size_t bytes_written = fwrite(mp3->bytes, 1, mp3->size, mp3->file);
    fclose(mp3->file);
    return bytes_written;
}

size_t mstg_mp3_read_message(mstg_mp3_t *mp3, uint32_t key, void** msg) {

    size_t *msg_len_raw = (size_t*)mstg_mp3_read_frame(mp3, MSG_SIZE_FRAME, sizeof(size_t));
    size_t msg_len = 0;
    memcpy(&msg_len, msg_len_raw, sizeof(size_t));
    free(msg_len_raw);
    fprintf(stderr,"MSG SIZE: %zu\n", msg_len);

    srand(key);
    size_t bytes_read = 0;
    uint32_t frames = (msg_len + mp3->rw_size - 1)/mp3->rw_size;

    *msg = malloc(msg_len);

    if(!*msg) {
        fprintf(stderr, "Cannot allocate %zu bytes, quitting\n", msg_len);
        exit(1);
    }

    for(uint32_t i=0; i<frames; i++) {
        uint32_t frame_id = mstg_mp3_rand_frame(mp3->max_frames);
        size_t read_size = 0;
        if (msg_len - bytes_read >= mp3->rw_size) read_size = mp3->rw_size;
        else read_size = msg_len - bytes_read;

        fprintf(stderr,"Trying to read %zu bytes from frame %d\n", read_size, frame_id);
        
        void* data = mstg_mp3_read_frame(mp3, frame_id, read_size);
        memcpy(*msg+bytes_read, data, read_size);
        bytes_read += read_size;
        free(data);
    }

    return msg_len;

}

void* mstg_mp3_read_frame(mstg_mp3_t *mp3, uint32_t frame_number, size_t read_size) {
    void* inp = malloc(read_size);
    size_t read_pos = mp3->tag_size + frame_number*(mp3->frame_size) + MP3_FRAME_HEADER_SIZE;
    memcpy(inp, mp3->bytes+read_pos, read_size);

    return inp;
}

int8_t mstg_mp3_write_message(mstg_mp3_t *mp3, void* msg, size_t msg_len, uint32_t key) {
    srand(key);
    uint32_t frames = (msg_len + mp3->rw_size -1)/mp3->rw_size;
    fprintf(stderr,"DEFAULT SIZE: %zu MAX_FRAMES: %zu USED: %d\n", mp3->rw_size, mp3->max_frames, frames);

    /* one extra frame to hold the size of the message */
    if(frames + 1 > mp3->max_frames) {
        return -1; 
    }

    mstg_mp3_write_frame(mp3, MSG_SIZE_FRAME, &msg_len, sizeof(msg_len)); 

    for(uint32_t i=0; i<frames; i++) {
        size_t frame_id = mstg_mp3_rand_frame(mp3->max_frames);
        
        size_t write_size = 0;
        if (msg_len >= mp3->rw_size) write_size = mp3->rw_size;
        else write_size = msg_len;
        fprintf(stderr,"Trying to write %zu bytes to frame %zu\n", write_size, frame_id);

        mstg_mp3_write_frame(mp3, frame_id, msg, write_size);
        msg_len -= write_size;
    }
    return 0;
}

int8_t mstg_mp3_write_frame(mstg_mp3_t *mp3, uint32_t frame_number, void* msg, size_t msg_len) {
    size_t write_pos = mp3->tag_size + frame_number*(mp3->frame_size) + MP3_FRAME_HEADER_SIZE;
    //fprintf(stderr, "Writing to location: %zu of %zu\n", write_pos, mp3->size);
    if (mp3->size - write_pos >= msg_len) {
        memcpy(mp3->bytes + write_pos, msg, msg_len);
        return 0;
    }
    else {
        return -1;
    }
}

size_t mstg_mp3_rand_frame(uint32_t max_frames) {
    size_t frame_id = MSG_SIZE_FRAME;
    while (frame_id == MSG_SIZE_FRAME) {
        frame_id = rand() % max_frames;
    }
    return frame_id;
}

size_t mstg_mp3_frame_size(mstg_mp3_t *mp3) {
    if (!mp3->frame_size) {
        size_t bitrate = 0;
        size_t sample_rate = 0;
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

size_t mstg_mp3_tag_size(mstg_mp3_t *mp3) {
    if(!mp3) return 0;
    size_t size = 0;
    if (mp3->tag_size) { /* don't need to read the size again if we've got it already */
        return mp3->tag_size;
    }
    if (!strncmp(mp3->bytes, ID3V2_IDENTIFIER, strlen(ID3V2_IDENTIFIER))) {
        size_t synchsafe_size = 0;
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


