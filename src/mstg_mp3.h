#include <stdint.h>
#include <sys/stat.h>

#define ID3V2_IDENTIFIER "ID3"
#define ID3V2_HEADER_SIZE 10
#define ID3V2_SIZE_OFFSET 6

#define MP3_FRAME_HEADER_SIZE 4
#define MP3_FRAME_HEADER_BYTE0 255
#define MP3_FRAME_HEADER_BYTE1 251

typedef struct _mstg_mp3_t mstg_mp3_t;

mstg_mp3_t* mstg_mp3_new(const char* file_path);

uint32_t mstg_mp3_parse_tag_size(mstg_mp3_t* mp3);
uint32_t mstg_mp3_get_tag_size(mstg_mp3_t* mp3);
uint32_t mstg_mp3_frame_size(mstg_mp3_t *mp3);

int8_t mstg_mp3_write_frame(mstg_mp3_t *mp3, uint32_t frame_number, char* msg, size_t msg_len);
int8_t mstg_mp3_write_message(mstg_mp3_t *mp3, char* msg, size_t msg_len, uint32_t key);
size_t mstg_mp3_writeout(mstg_mp3_t *mp3);

int unsynchsafe(int in); 
