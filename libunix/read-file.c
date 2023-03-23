#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// allocate buffer, read entire file into it, return it.   
// buffer is zero padded to a multiple of 4.
//
//  - <size> = exact nbytes of file.
//  - for allocation: round up allocated size to 4-byte multiple, pad
//    buffer with 0s. 
//
// fatal error: open/read of <name> fails.
//   - make sure to check all system calls for errors.
//   - make sure to close the file descriptor (this will
//     matter for later labs).
// 
void *read_file(unsigned *size, const char *name) {
    int fd = open(name,O_RDONLY);
    assert(fd != -1);
    //    - use stat to get the size of the file.
    struct stat f_stats;
    assert(stat(name,&f_stats) != -1);
    //    - round up to a multiple of 4.
    *size = (unsigned) f_stats.st_size;
    //    - allocate a buffer
    char * buff = calloc(*size + (*size%4),1);
    //    - read entire file into buffer.  
    
    int amount_read = 0;
    char * bufC = buff;
    while(amount_read < *size){
        int bytes_read = read(fd,bufC,*size - amount_read);
        assert (bytes_read != -1);
        amount_read += bytes_read;
        bufC += bytes_read;
    }
    close(fd);
    return buff;
}
