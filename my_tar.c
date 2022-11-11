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
#include <stdbool.h>

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

}my_tar_header;

typedef struct s_my_tar_node
{
    my_tar_header* header;
    char* data;
    struct s_my_tar_node* next;

}my_tar_node;

#endif

void print_header(my_tar_header* header)
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

void print_node(my_tar_node* node)
{
    printf("HEADER:\n");
    print_header(node->header);
    printf("\nDATA:\n");
    if(node->data)
    {
        printf("%s\n", node->data);
    }
    else
    {
        printf("DIR\n");
    }
    
}

void print_list(my_tar_node* head)
{
    if(head)
    {
        print_node(head);
        printf("\n");
        print_list(head->next);
    }
}

void strip_zeroes(char* octstring, char* stripedstring)
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

int s_oct_to_dec(my_tar_header* header)
{
    char stripped_string[12];
    strip_zeroes(header->size, stripped_string);
    int pwr = strlen(stripped_string) - 1;
    int dec = 0;
    int i =0;
    while(pwr >= 0)
    {
        dec += (stripped_string[i] - '0')*power(8,pwr);
        pwr--;
        i++;
    }

    return dec;
}

char* read_data_to_node(int fd, my_tar_header* header)
{
    int dataSize = s_oct_to_dec(header);
    if(dataSize == 0)
    {
        return NULL;
    }
    
    dataSize += 512 - dataSize % 512;
    char* data = (char*)malloc(sizeof(char)*(dataSize));
    read(fd, data, dataSize);
    return data;
}

my_tar_node* make_new_node(int fd)
{
    char burn[512];
    my_tar_node* node = malloc(sizeof(my_tar_node));
    my_tar_header* header = malloc(sizeof(my_tar_header));
    read(fd, header, sizeof(*header));
    read(fd, burn, 12);
    node->header = header;
    node->data = read_data_to_node(fd, header);
    node->next = NULL;
    return node;
}

void add_node(my_tar_node* head, my_tar_node* newNode)
{
    my_tar_node* nav = head;
    while(nav->next)
    {
        nav = nav->next;
    }
    nav->next = newNode;
}

my_tar_node* construct_linked_list_from_tar(int fd)
{

    bool eof = false;
    char c;
    my_tar_node* head = make_new_node(fd);

    while(!eof)
    {
        read(fd, &c, 1);
        if(c)
        {
            lseek(fd, -1, SEEK_CUR);
            add_node(head, make_new_node(fd));
        }
        else 
        {
            eof = true;
        }
        
    }

    return head;
}

// Destruction Functions

void free_node(my_tar_node* node)
{
    free(node->header);
    free(node->data);
    free(node);
}

void free_list(my_tar_node* head)
{
    my_tar_node* p_curr = head;
    my_tar_node* p_next;
    while(p_curr->next)
    {
        p_next = p_curr->next;
        free_node(p_curr);
        p_curr = p_next;
    }

    free_node(p_curr);
}

int main(int argc, char* argv[])
{
    int fd = open("multiple_txt_tar.tar", O_RDONLY);

    // // Test making a node
    // my_tar_node* tst_node1 = make_new_node(fd);
    // print_node(tst_node1);
    // printf("\n");

    // my_tar_node* tst_node2 = make_new_node(fd);
    // print_node(tst_node2);
    // printf("\n");

    // print_node(tst_node1);

    // free_node(tst_node1);
    // free_node(tst_node2);


    // // Test making a and printing list
    // my_tar_node* head = construct_linked_list_from_tar(fd);
    // print_list(head);
    // free_list(head);

    // close(fd);


    // Test reading tar dir
    fd = open("tar_dir_tar.tar", O_RDONLY);
    my_tar_node* dir_head = construct_linked_list_from_tar(fd);
    print_list(dir_head);
    free_list(dir_head);

    close(fd);


}