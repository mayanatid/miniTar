#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>




// Going to work here on reading 512-bit sections and interpreting them
// Idea: get total size of file.... divide by 512 to get number of records
// Psuedo-code
// Start
// Get total size of file and divide by 512 to get number of records
// Read 512 bytes as first header
// (?) Check the file type here (?)
// We then write until we hit next header
//  Q: How do we know a new header is hit? Should we determine the number of files in the archive before writing? Can this be determined from the size field?
//  A: Yes can determine file size from header. So trick is to read first header then determine if dir
//     if it is dir then there will be some extra steps... If not then we have a single file (is that true? Maybe not)
//     Maybe it would be best to try and solve fore a single text file first, then dir of text files, etc.
//     

int getTarFileSize(char* filename)
{
    struct stat st;
    stat(filename, &st);
    return (int)st.st_size;
}

int getTarFileSizeWFileOpen(char* filename)
{
    int fd = open(filename, O_RDONLY);
    char c;
    int count = 0;

    while(read(fd, &c, 1))
    {
        count++;
    }

    close(fd);
    return count;
}

int main()
{
    int f_size = getTarFileSize("testDir.tar");
    int uf_size= getTarFileSizeWFileOpen("testDir.tar");
    char c[255];
    char d;


    printf("size of testDir.tar: %d bytes\n", f_size);
    printf("this means there are %d records\n", f_size/512);
    printf("size of testDir.tar: %d bytes\n", uf_size);
    printf("this means there are %d records\n", uf_size/512);

    int fd= open("test_txt.tar", O_RDONLY);
    int count = 0;
    lseek(fd, 512, SEEK_SET);
    while(read(fd, &d, 1))
    {
        count++;
    }
    printf("count: %d\n", count);
    close(fd);




    return 0;
}