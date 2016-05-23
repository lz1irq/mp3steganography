#include <stdio.h>
#include <string.h>
#include "mstg_mp3.h"

int main(int argc, char** argv) {
    if (argc >= 2) {
        mstg_mp3_t *mp3 = mstg_mp3_new(argv[1]);
        printf("ID3v2 header size: %d\n", mstg_mp3_get_tag_size(mp3));
        printf("Frame size: %d\n", mstg_mp3_frame_size(mp3));
        
        char* msg = "secret message here";
        mstg_mp3_write_message(mp3, msg, strlen(msg), 0x321fa);

        printf("WRITE: %d\n", mstg_mp3_writeout(mp3));
    }

}
