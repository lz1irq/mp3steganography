#ifndef _MSTG_MP3_H
#define _MSTG_MP3_H

#include <stdint.h>
#include <sys/stat.h>

#define ID3V2_IDENTIFIER "ID3"
#define ID3V2_HEADER_SIZE 10
#define ID3V2_SIZE_OFFSET 6

#define MSG_SIZE_FRAME 0

#define MP3_FRAME_HEADER_SIZE 4
#define MP3_FRAME_HEADER_BYTE0 255
#define MP3_FRAME_HEADER_BYTE1 251

typedef struct _mstg_mp3_t mstg_mp3_t;

mstg_mp3_t* mstg_mp3_new(const char* file_path);
void mstg_mp3_close(mstg_mp3_t *mp3);

uint32_t mstg_mp3_parse_tag_size(mstg_mp3_t* mp3);
uint32_t mstg_mp3_get_tag_size(mstg_mp3_t* mp3);
uint32_t mstg_mp3_frame_size(mstg_mp3_t *mp3);

int8_t mstg_mp3_write_frame(mstg_mp3_t *mp3, uint32_t frame_number, void* msg, size_t msg_len);
int8_t mstg_mp3_write_message(mstg_mp3_t *mp3, void* msg, size_t msg_len, uint32_t key);
size_t mstg_mp3_writeout(mstg_mp3_t *mp3);

void* mstg_mp3_read_frame(mstg_mp3_t *mp3, uint32_t frame_number, size_t read_size);
size_t mstg_mp3_read_message(mstg_mp3_t *mp3, uint32_t key, void** msg);
uint32_t mstg_mp3_rand_frame(uint32_t max_frames);


int unsynchsafe(int in); 

#endif
