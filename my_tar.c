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

#ifndef TAR_STRUCTS
#define TAR_STRUCTS
typedef struct s_my_tar_header
{                                 /* byte offset */
    char name[100];               /*   0 */
    char mode[8];                 /* 100 */
    char uid[8];                  /* 108 */
    char gid[8];                  /* 116 */
    char size[12];                /* 124 */
    char mtime[12];               /* 136 */
    char chksum[8];               /* 148 */
    char typeflag;                /* 156 */
    char linkname[100];           /* 157 */
    char magic[6];                /* 257 */
    char version[2];              /* 263 */
    char uname[32];               /* 265 */
    char gname[32];               /* 297 */
    char devmajor[8];             /* 329 */
    char devminor[8];             /* 337 */
    char prefix[155];             /* 345 */
                                  /* 500 */

}MyTarHeader;

typedef struct s_my_tar_node
{
    MyTarHeader* header;
    char* data;
    struct s_my_tar_node* next;

}MyTarNode;

#endif

void print_header(MyTarHeader* header)
{
    printf("name: %s\n", header->name);
    printf("mode: %s\n", header->mode);
    printf("uid: %s\n", header->uid);
    printf("gid: %s\n", header->gid);
    printf("size: %s\n", header->size);
    printf("mtime: %s\n", header->mtime);
    printf("chksum: %s\n", header->chksum);
    printf("typeflag: %c\n", header->typeflag);
    printf("linkname: %s\n", header->linkname);
    printf("magic: %s\n", header->magic);
    printf("version: %s\n", header->version);
    printf("uname: %s\n", header->uname);
    printf("gname: %s\n", header->gname);
    printf("devmajor: %s\n", header->devmajor);
    printf("devminor: %s\n", header->devminor);
    printf("prefix: %s\n", header->prefix);
}

void print_node(MyTarNode* node)
{
    printf("HEADER:\n");
    print_header(node->header);
    printf("\nDATA:\n");
    printf("%s\n", node->data);
}

void StripZeros(char* octstring, char* stripedstring)
{
    int i=0;
    int k=0;
    while(octstring[i] == '0')
    {
        i++;
    }
    sprintf(stripedstring, "%s", octstring+i);

}

int power(int base, int n)
{
    if(n == 0)
    {
        return 1;
    }
    return base * power(base, n-1);
}

int sOctToDec(MyTarHeader* header)
{
    char strippedString[12];
    StripZeros(header->size, strippedString);
    int pwr = strlen(strippedString) - 1;
    int dec = 0;
    int i =0;
    while(pwr >= 0)
    {
        dec += (strippedString[i] - '0')*power(8,pwr);
        pwr--;
        i++;
    }

    return dec;
}

char* ReadDataToNode(int fd, MyTarHeader* header)
{
    int dataSize = sOctToDec(header);
    dataSize += 512 - dataSize % 512;
    char* data = (char*)malloc(sizeof(char)*(dataSize));
    read(fd, data, dataSize);
    return data;
}

MyTarNode* MakeNewNode(int fd)
{
    char burn[512];
    MyTarHeader header;
    MyTarNode* node = malloc(sizeof(MyTarNode));
    read(fd, &header, sizeof(header));
    read(fd, burn, 12);
    node->header = &header;
    node->data = ReadDataToNode(fd, &header);
    return node;
}

void AddNode(MyTarNode* head, MyTarNode* newNode)
{
    MyTarNode* nav = head;
    while(nav->next)
    {
        nav = nav->next;
    }
    nav->next = newNode;
}

MyTarNode* ConstructLinkeListFromTar(int fd)
{

    char eof;
    MyTarNode* head = MakeNewNode(fd);

    while(read(fd, &eof, 1))
    {
        lseek(fd, -1, SEEK_CUR);
        AddNode(head, MakeNewNode(fd));
    }

    return head;
}

// Destruction Functions

void FreeNode(MyTarNode* node)
{
    free(node->header);
    free(node->data);
}

int main(int argc, char* argv[])
{
    int fd = open("txt_tar.tar", O_RDONLY);

    // Test making a node
    MyTarNode* tst_node1 = MakeNewNode(fd);
    print_header(tst_node1->header);

}