#include <stdio.h>
#include "mstg_mp3.h"

int main() {
    mstg_mp3_t *mp3 = mstg_mp3_new("./geneva.mp3");
    printf("ID3v2 header size: %d\n", mstg_mp3_get_tag_size(mp3));

}
