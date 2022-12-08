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
#include <time.h>
#include <utime.h>


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

void print_file_names(my_tar_node* head)
{
    if(head)
    {
        printf("%s\n", head->header->name);
        print_file_names(head->next);
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

int add_node(my_tar_node* head, my_tar_node* new_node)
{
    if(!new_node)
    {
        return -1;
    }
    my_tar_node* nav = head;
    while(nav->next)
    {
        nav = nav->next;
    }
    nav->next = new_node;
    return 0;
}

int add_node_if_new(my_tar_node* head, my_tar_node* new_node)
{
    if(!new_node)
    {
        return -1;
    }
    
    my_tar_node* nav1 = head;
    my_tar_node* nav2 = head;
    my_tar_node* nav3 = new_node;
    my_tar_node* last_added_nav3;

    while(nav1->next)
    {
        nav1 = nav1->next;
    }
    
    // Want to compare each node in new_node to each node head;
    // Compare each node in nav3 to all nodes in nav2;
    // When find same name, compare mod time;
    // If mode time of nav3 is older than mode time of nav2: then move to next node in new_node
    // Else move through rest of nav2 to see if another entry exists with same name that is newer
    // If reach end of nav2 and there isn't a newer node, then add 
    while(nav3)
    {
        if(strcmp(nav3->header->name, nav2->header->name))
        {
           if(atoi(nav3->header->mtime) <= atoi(nav2->header->mtime))
           {
               // Move on to next node in new_node
               nav3 = nav3->next;
           }
        }
        else if(nav2->next)
        {
            nav2 = nav2->next;
        }
        else 
        {   // Reached end of nav2 without finding; so add

            if(last_added_nav3)
            {
                last_added_nav3->next = nav3;
            }
            else
            {
                nav2->next = nav3;
                last_added_nav3 = nav3;
            }
            
            nav3 = nav3->next;
            nav2 = head;
        }
    }

    last_added_nav3->next = NULL;

    return 0;
}

my_tar_node* make_linked_list_from_tar_file(int fd)
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
    if(fd == -1)
    {
        return NULL;
    }
    my_tar_header* header = malloc(sizeof(my_tar_header));
    memset(header, 0, sizeof(*header));
    my_tar_node* node = malloc(sizeof(my_tar_node));

    populate_header_from_file_name(filename, header);
    node->header = header;
    node->data = read_file_data_to_node(fd, node->header);
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
        //printf("%s\n", path);
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
    int stat_check;
    stat_check = stat(filename, &st);
    if(stat_check == -1)
    {
        fprintf(stderr, "%s doesn't exist\n", filename);
        return NULL;
    }
    if(S_ISDIR(st.st_mode))
    {
        return make_linked_list_from_dir(filename);
    }
    else
    {
        return make_new_node_from_file_name(filename);
    }
}

my_tar_node* make_new_nodes_from_file_names(char** filenames, int args)
{
    int new_node_check;
    my_tar_node* head = make_linked_list_from_file_name(filenames[0]);
    if(!head)
    {
        return NULL;
    }
    int i=1;
    while(i < args)
    {
        new_node_check = add_node(head, make_linked_list_from_file_name(filenames[i]));
        if(new_node_check == -1)
        {
            return NULL;
        }
        i++;
    }
    return head;
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

// WRITE FILES FROM TAR STRUCT
void change_file_modify_time(char* filename, unsigned int new_mtime)
{
    struct stat st;
    struct utimbuf new_times;
    stat(filename, &st);
    new_times.actime = st.st_atime;
    // printf("Current time: %lu\n", st.st_mtime);
    // printf("Desired time: %d\n", new_mtime);
    new_times.modtime = new_mtime;
    utime(filename, &new_times);
    stat(filename, &st);
    // printf("Time is now: %lu\n", st.st_mtime);

}

void change_file_modify_time_from_node(my_tar_node* node)
{
    struct stat st;
    struct utimbuf new_times;
    unsigned int ret;
    char* ptr;
    stat(node->header->name, &st);
    new_times.actime = st.st_atime;
    ret = strtol(node->header->mtime, &ptr, 8);
    new_times.modtime = ret;
    utime(node->header->name, &new_times);
}

void create_file_from_node(my_tar_node* node)
{
    char mode[5];
    char* ptr;
    unsigned int ret;
    struct stat st;
    int fd;
    struct utimbuf new_times;
    int dir_ret;


    strncpy(mode, node->header->mode + 3, 5);
    ret = strtol(mode, &ptr, 8); // Octal conversion of mode

    // Check for dir
    if(node->header->typeflag == DIRTYPE)
    {
        dir_ret = mkdir(node->header->name, ret);
        change_file_modify_time_from_node(node);    
    }
    else 
    {
        // printf("HERE\n");
        fd = open(node->header->name, O_CREAT | O_RDWR, ret);
        change_file_modify_time_from_node(node);
        // printf("%s\n", node->data);
        write(fd, node->data, size_oct_to_dec(node->header));
    }
    close(fd);


    
    // chmod(node->header->name, ret);
}

void create_files_from_linked_list(my_tar_node* head)
{
    my_tar_node* nav = head;
    while(nav)
    {
        create_file_from_node(nav);
        nav = nav->next;
    }
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

int check_if_tar_file(char* filename)
{
    int i = 0;
    int k = 0;
    char tar_test[] = ".tar";
    char eof_f[5] = {'\0'};
    while(filename[i] != '\0')
    {
        if(i < strlen(filename) - 4)
        {
            i++;
            continue;
        }
        else
        {
            eof_f[k] = filename[i];
            k++;
        }
        i++;
    }
    return strcmp(tar_test, eof_f);
}

int main(int argc, char* argv[])
{
    // Check through options
    
    bool op_c, op_r, op_t, op_u, op_x, op_f, files_to_ll;
    int i = 0;

    if(argc < 3)
    {
        return 0;
    }

    
    while(argv[1][i] != '\0')
    {
        // printf("%c\n", argv[1][i]);   
        switch(argv[1][i])
        {
            case '-':
                i++;
                continue;
            case 'c':
                op_c = true;
                break;
            case 'r':
                op_r = true;
                break;
            case 't':
                op_t = true;
                break;
            case 'u':
                op_u = true;
                break;
            case 'x':
                op_x = true;
                break;
            case 'f':
                op_f = true;
                break;
            default:
                fprintf(stderr, "%c is not a valid option\n", argv[1][i]);
                return 1;
        }
        i++;
        
    }

    // should only be able to have one option other than f.
    if( (op_r || op_c || op_u || op_x || op_t) && !op_f)
    {
        fprintf(stderr, "%s", "option requires 'f'\n");
        return 1;
    }
    if(op_r && !op_f)
    {
        fprintf(stderr, "%s", "'r' option requires 'f' option\n");
        return 1;
    }
    if(op_c && !op_f)
    {
        fprintf(stderr, "%s", "'c' option requires 'f' option\n");
        return 1;
    }
    if(op_u && !op_f)
    {
        fprintf(stderr, "%s", "'u' option requires 'f' option\n");
        return 1;
    }
    if( (op_c && op_r) || (op_c && op_t) || (op_c && op_u) || (op_c && op_x) )
    {
        fprintf(stderr, "You may not specify more than once '-ctrux' options\n");
        return 1;
    }
    if( (op_r && op_t) || (op_r && op_u) || (op_r && op_x) )
    {
        fprintf(stderr, "You may not specify more than once '-ctrux' options\n");
        return 1;
    }
    if( (op_t && op_u) || (op_r && op_x) )
    {
        fprintf(stderr, "You may not specify more than once '-ctrux' options\n");
        return 1;
    }
    if( (op_u && op_x))
    {
        fprintf(stderr, "You may not specify more than once '-ctrux' options\n");
        return 1;
    }


    // Now go through remaining arguments and proceed based on options
    my_tar_node* head_c;
    my_tar_node* head_r;
    my_tar_node* head_tx;

    // Check that second argument is .tar
    if(check_if_tar_file(argv[2]) != 0)
    {
        fprintf(stderr, "%s", "Second argument must be a .tar file\n");
        return 1;
    }

    if(op_c | op_r | op_u)
    {
        files_to_ll = true;
        if(argc < 4)
        {
            fprintf(stderr, "Can't make empty archive\n");
            return 1;
        }

        char* filenames[argc - 3];
        for(int j = 0;j < argc - 3; j++)
        {
            filenames[j] = argv[j + 3];
            //printf("Filename: %s\n", filenames[j]);
        }
        head_c = make_new_nodes_from_file_names(filenames, argc - 3);
        if(!head_c)
        {
            fprintf(stderr,"One or more of the given filenames don't exist\n");
            return 1;
        }

        if(op_c)
        {
            make_tar_from_linked_list(argv[2], head_c);
        }
        if(op_r || op_u)
        {
            int fd = open(argv[2], O_RDWR); // NEED ERROR CHECK IN CASE DESON'T EXIST
            if(fd < 0)
            {
                make_tar_from_linked_list(argv[2], head_c);
                return 0;
            }
            head_r = make_linked_list_from_tar_file(fd);
            close(fd);

            if(op_u)
            {
                add_node_if_new(head_c, head_r);
            }
            else
            {
                add_node(head_c, head_r);
            }
            

            make_tar_from_linked_list(argv[2], head_c);
        }
        free_list(head_c);
    } 


    int ft; 
    if(op_t)
    {
        if(argc > 3)
        {
            fprintf(stderr, "too many arguments\n");
            return 1;
        }
        ft = open(argv[2], O_RDWR);
        head_tx = make_linked_list_from_tar_file(ft);
        print_file_names(head_tx);
        close(ft);
        free_list(head_tx);
        return 0;
    }

    if(op_x)
    {
        if(argc > 3)
        {
            fprintf(stderr, "too many arguments\n");
            return 1;
        }

        ft = open(argv[2], O_RDWR);
        head_tx = make_linked_list_from_tar_file(ft);
        create_files_from_linked_list(head_tx);
        if(head_tx->header->typeflag == DIRTYPE)
        {
            change_file_modify_time_from_node(head_tx);
        }
        close(ft);
        free_list(head_tx);
    }


    // int fd = open("tar_dir_man.tar", O_RDWR);
    // my_tar_node* node = make_linked_list_from_tar_file(fd);
    // print_list(node);

    // create_files_from_linked_list(node);
    // close(fd);

    // change_file_modify_time_from_node(node); // Seems like have to do this out here for some reason
    // struct stat st;
    // stat("tar_dir_man", &st);
    // printf("Mod Time: %lu\n", st.st_mtime);

    // free_list(node);

    return 0;
}