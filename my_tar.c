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
#include <dirent.h>


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


#define TMAGIC   "ustar "    /* ustar and a null */
#define TMAGIC_NULL "ustar\0"
#define TMAGLEN  6
#define TVERSION " \0"           /* 00 and no null */
#define TVERSLEN 2

/* Values used in typeflag field.  */
#define REGTYPE  '0'            /* regular file */
#define AREGTYPE '\0'           /* regular file */
#define LNKTYPE  '1'            /* link */
#define SYMTYPE  '2'            /* reserved */
#define CHRTYPE  '3'            /* character special */
#define BLKTYPE  '4'            /* block special */
#define DIRTYPE  '5'            /* directory */
#define FIFOTYPE '6'            /* FIFO special */
#define CONTTYPE '7'            /* reserved */

#define XHDTYPE  'x'            /* Extended header referring to the
                                   next file in the archive */
#define XGLTYPE  'g'            /* Global extended header */

#define CHKSUM_DEF "        \0"
/* Bits used in the mode field, values in octal.  */
#define TSUID    04000          /* set UID on execution */
#define TSGID    02000          /* set GID on execution */
#define TSVTX    01000          /* reserved */
                                /* file permissions */
#define TUREAD   00400          /* read by owner */
#define TUWRITE  00200          /* write by owner */
#define TUEXEC   00100          /* execute/search by owner */
#define TGREAD   00040          /* read by group */
#define TGWRITE  00020          /* write by group */
#define TGEXEC   00010          /* execute/search by group */
#define TOREAD   00004          /* read by other */
#define TOWRITE  00002          /* write by other */
#define TOEXEC   00001          /* execute/search by other */

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

// PRINT FUNCTIONS
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

// UTILITY FUNCTIONS
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

int size_oct_to_dec(my_tar_header* header)
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

int CalcAscii(const char* str, int len)
{
    int i =0;
    int sum =0;
    while(i<len)
    {
        sum += (int)str[i];
        i++;
    }

    return sum;
}

char determine_typeflag(struct stat st)
{

    if(S_ISREG(st.st_mode))
    {
        return REGTYPE;
    }
    if(S_ISDIR(st.st_mode))
    {
        return DIRTYPE;
    }
    if(S_ISFIFO(st.st_mode))
    {
        return FIFOTYPE;
    }
    if(S_ISCHR(st.st_mode))
    {
        return CHRTYPE;
    }
    if(S_ISLNK(st.st_mode))
    {
        return LNKTYPE;
    }
    if(S_ISBLK(st.st_mode))
    {
        return BLKTYPE;
    }
    return REGTYPE;
}

// READ TAR FILES TO STRUCTS
char* read_file_data_to_node(int fd, my_tar_header* header)
{
    int dataSize = size_oct_to_dec(header);
    if(dataSize == 0)
    {
        return NULL;
    }
    
    dataSize += 512 - dataSize % 512;
    char* data = (char*)malloc(sizeof(char)*(dataSize));
    memset(&data[0], 0, dataSize);
    read(fd, data, dataSize);
    return data;
}

my_tar_node* make_new_node_from_tar_file(int fd)
{
    char burn[512];
    my_tar_node* node = malloc(sizeof(my_tar_node));
    my_tar_header* header = malloc(sizeof(my_tar_header));
    read(fd, header, sizeof(*header));
    read(fd, burn, 12);
    node->header = header;
    node->data = read_file_data_to_node(fd, header);
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

my_tar_node* construct_linked_list_from_tar_file(int fd)
{

    bool eof = false;
    char c;
    my_tar_node* head = make_new_node_from_tar_file(fd);

    while(!eof)
    {
        read(fd, &c, 1);
        if(c)
        {
            lseek(fd, -1, SEEK_CUR);
            add_node(head, make_new_node_from_tar_file(fd));
        }
        else 
        {
            eof = true;
        }
        
    }

    return head;
}

// READ FILES TO STRUCTS
void calculate_check_sum(my_tar_header *header)
{
    int sum = 0;
    sum += CalcAscii(header->name,100);
    sum += CalcAscii(header->mode,8);
    sum += CalcAscii(header->uid,8);
    sum += CalcAscii(header->gid,8);
    sum += CalcAscii(header->size,12);
    sum += CalcAscii(header->mtime,12);
    sum += CalcAscii(CHKSUM_DEF, 8); // Checksum default
    sum += (int)header->typeflag;
    sum += CalcAscii(header->linkname,100);
    sum += CalcAscii(header->magic,6);
    sum += CalcAscii(header->version,2);
    sum += CalcAscii(header->uname,32);
    sum += CalcAscii(header->gname,32);
    sum += CalcAscii(header->devmajor,8);
    sum += CalcAscii(header->devminor,8);
    sum += CalcAscii(header->prefix,155);
    
    // input result into checksum
    sprintf(header->chksum, "%06o", sum);
    header->chksum[6] = ' ';
    header->chksum[7] = '\0';
}

int determine_file_size(struct stat st)
{
    if(S_ISDIR(st.st_mode))
    {
        return 0;
    }

    return (int)st.st_size;
}

void populate_header_from_file_name(char* filename, my_tar_header* header)
{
    // Take a filename and a tar header struct and populates
    // header struct with relavent data

    struct stat st;
    struct group *grp;
    struct passwd *pwd;
    
    stat(filename, &st);
    sprintf(header->name, "%s", filename);
    sprintf(header->mode, "%07o", st.st_mode & 0777);
    sprintf(header->gid, "%07o", st.st_gid);
    sprintf(header->uid, "%07o", st.st_uid);
    sprintf(header->mtime, "%011o", (int)st.st_mtim.tv_sec);
    sprintf(header->size, "%011o", determine_file_size(st));
    grp = getgrgid(st.st_gid);
    sprintf(header->gname, "%s", grp->gr_name);
    pwd = getpwuid(st.st_uid);
    sprintf(header->uname, "%s",  pwd->pw_name);
    sprintf(header->magic, "%s", TMAGIC);
    sprintf(header->version, "%s", TVERSION);
    // sprintf(header->devmajor, "%d", major(st.st_rdev));
    // sprintf(header->devminor, "%d", minor(st.st_rdev));
    header->typeflag = determine_typeflag(st);
    calculate_check_sum(header);

}

my_tar_node* make_new_node_from_file_name(char* filename)
{
    int fd = open(filename, O_RDONLY);
    my_tar_header* header = malloc(sizeof(my_tar_header));
    memset(header, 0, sizeof(*header));
    my_tar_node* node = malloc(sizeof(my_tar_node));

    populate_header_from_file_name(filename, header);
    node->header = header;
    node->data = read_file_data_to_node(fd, node->header);
    // printf("inside make_new_node_from_file_name: %s\n", node->data);
    node->next =NULL;

    close(fd);
    return node;
}

my_tar_node* make_linked_list_from_dir(char* dirname)
{   char path[100] = {'\0'};
    DIR* folder;
    struct dirent* entry;
    char* n_dirname = (char*)malloc(sizeof(char) * (strlen(dirname) + 2));
    memset(n_dirname, 0, strlen(dirname) + 2);
    strcpy(n_dirname, dirname);
    n_dirname[strlen(dirname)] = '/';
    my_tar_node* head = make_new_node_from_file_name(n_dirname);

    folder = opendir(dirname);
    if(folder == NULL)
    {
        perror("Unable to read directory");
        return head;
    }
    while((entry = readdir(folder)))
    {
        if(entry->d_name[0] == '.')
        {
            continue;
        }
        memset(path, 0, 100);
        strcat(path, n_dirname);
        strcat(path, entry->d_name);
        add_node(head, make_new_node_from_file_name(path));
    }
    closedir(folder);
    return head;   
}

my_tar_node* make_linked_list_from_file_name(char* filename)
{
    struct stat st;
    stat(filename, &st);

    if(S_ISDIR(st.st_mode))
    {
        return make_linked_list_from_dir(filename);
    }
    else
    {
        return make_new_node_from_file_name(filename);
    }
}

// WRITE STRUCTS TO TAR
void make_tar_from_linked_list(char* tar_file_name, my_tar_node* head)
{
    char burn[10240] = {'\0'};
    int blk = 0;
    int fd = open(tar_file_name, O_CREAT | O_TRUNC | O_RDWR, 0644);
    my_tar_node* nav = head;

    while(nav)
    {
        // printf("INSIDE MAKE TAR FUNCTION\n");
        print_node(nav);
        write(fd, nav->header, sizeof(*nav->header));
        write(fd, &burn[0], 12);
        if(nav->data)
        {
             write(fd, nav->data, strlen(nav->data) + 512 - strlen(nav->data) % 512);
        }
       
        nav = nav->next;
    }
   
    write(fd, &burn[0], 512);
    write(fd, &burn[0], 512);
    if(lseek(fd, 0, SEEK_CUR) < 10240)
    {
        write(fd, &burn[0], 10240 - lseek(fd, 0, SEEK_CUR));
    }
    close(fd);
}

// WRITE FILES FROM TAR

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
    // printf("*****TEST:READ TAR FILE TO LINKED LIST*****\n");
    // int fd = open("multiple_txt_tar.tar", O_RDONLY);

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


    // // Test reading tar dir
    // int fd = open("tar_dir_tar.tar", O_RDONLY);
    // my_tar_node* dir_head = construct_linked_list_from_tar_file(fd);
    // print_list(dir_head);
    
    // close(fd);

    printf("\n*****TEST:CREATE NODE FROM FILENAME*****\n");
    char filename[] = "tar_dir";
    my_tar_node *node;
    node = make_linked_list_from_file_name(filename);
    print_list(node);

    make_tar_from_linked_list("test_create_dir_tar.tar", node);

    // // Test making file from one node
    // make_tar_from_linked_list("test_create_tar.tar", node);
    
    free_list(node);
    // free_list(dir_head);

    // // Test making tar file from linked list
    // printf("\n*****TEST:CREATE TAR FROM LINKED LIST*****\n");
    // int fd = open("multiple_txt_tar.tar", O_RDONLY);
    // my_tar_node* head = construct_linked_list_from_tar_file(fd);
    // print_list(head);

    // make_tar_from_linked_list("test_create_tar.tar", head);
    
    // close(fd);




    return 0;


}