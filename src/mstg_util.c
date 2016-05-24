#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

off_t get_file_size(const char* file_path) {
    struct stat fstat_str;
    if (stat(file_path, &fstat_str) != 0) {
        fprintf( stderr, "%s\n", strerror( errno ));
        return 0;
    }
    return fstat_str.st_size;
}

/* Please note that you must manually free the memory storing the file contents */
char* read_file(FILE* fp, off_t fsize) {
    char* buffer = (char*)malloc(fsize);
    if (!buffer) return NULL;
    size_t bytes_read = fread(buffer, 1, fsize, fp);
    
    if ( bytes_read != fsize) return NULL;

    return buffer;
}

size_t write_file(FILE* fp, char* content, size_t content_len) {
    size_t bytes_written = 0;
    bytes_written = fwrite(content, 1, content_len, fp);
    if(!bytes_written) {
        printf("%s\n", strerror(errno));
    }
    return bytes_written;
}

