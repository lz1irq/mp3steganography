# include <sys/types.h>
off_t get_file_size(const char* file_path);
char* read_file(FILE* fp, off_t fsize);
size_t write_file(FILE* fp, char* content, size_t content_len);
