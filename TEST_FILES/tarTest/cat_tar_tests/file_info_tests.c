#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <fcntl.h>
#include <unistd.h>

int count_file_ascii(char* filename)
{
    int fd = open(filename, O_RDONLY);
    int counter =0;
    int position =0;
    int POSITION_LIMIT = 400;
    char c;
    if(fd <0)
    {
        perror("r1");
        exit(1);
    }

    while(read(fd, &c, 1) && position < POSITION_LIMIT)
    {
        position++;
        if(!c)
        {
            continue;
        }
        counter += (int)c;
    }
    close(fd);
    return counter;
}

int get_file_size(char* filename)
{
    int fd = open(filename, O_RDONLY);
    int counter =0;
    char c;
    if(fd <0)
    {
        perror("r1");
        exit(1);
    }

    while(read(fd, &c, 1))
    {
        counter++;
    }
    close(fd);
    return counter;
}


int main()
{
    struct stat st;
    struct group *grp;
    struct passwd *pwd;

    stat("test.txt", &st);
    printf("mode: %o\n", st.st_mode);
    printf("group ID: %o\n", st.st_gid);
    printf("user ID: %o\n", st.st_uid);
    printf("mod time: %o\n", (int)st.st_mtim.tv_sec);
    printf("size %o\n", (int)st.st_size);
    grp = getgrgid(st.st_gid);
    printf("group name %s/\n", grp->gr_name);
    pwd = getpwuid(st.st_uid);
    printf("user name %s/\n", pwd->pw_name);
    printf("block size: %lu\n", st.st_blksize);
    printf("num blocks: %lu\n", st.st_blocks);

    printf("Size of file in ASCII: %d\n", count_file_ascii("test_text.tar") );

     

    return 0;
}
