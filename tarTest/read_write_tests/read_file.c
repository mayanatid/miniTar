#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

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


int count_file_ascii(char* filename)
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
        counter += (int)c;
    }
    close(fd);
    return counter;
}

int main()
{
    int fd = open("foo.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(fd <0)
    {
        perror("r1");
        exit(1);
    }


    int sz = write(fd, "testing that this is written\n", strlen("testing that this is written\n"));
    printf("num of chars in file: %d\n", get_file_size("foo.txt"));
    printf("ascii sum of chars in file: %d\n", count_file_ascii("foo.txt"));
    char c;
    fd = open("foo.txt", O_RDONLY);
    while(read(fd, &c, 1))
    {
        write(1, &c, 1);
    }

    close(fd);


    

    return 0;
}