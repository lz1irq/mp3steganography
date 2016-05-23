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

int unsynchsafe(int in); 
