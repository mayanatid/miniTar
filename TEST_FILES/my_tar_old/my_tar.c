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

// Each tar file consists of records
// Each record will have a header and file contents
// 
typedef struct s_my_tar_file{
    MyTarHeader *header;
    char* _data; // This will be dynamic and dependent on file size

}MyTarFile;

typedef struct s_my_tar_node{
    MyTarFile *file;
    struct MyTarNode *next;
    struct MyTarNode *prev;

}MyTarNode;

typedef struct s_my_tar_option{

}MyTarOption;


typedef struct s_my_tar_program{

}MyTar;

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

void StripZeros(char* octstring, char* stripedstring)
{
    int i=0;
    int k=0;
    while(octstring[i] == '0')
    {
        i++;
    }
    // printf("%d\n", i);
    // printf("%s\n", octstring+i);
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

int OctToDec(char* oct)
{
    char strippedString[12];
    StripZeros(oct, strippedString);
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

void set_char(char* a, char b)
{
    *a = b;
}

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

void ReadFileToString(char* filename, char* str, int len)
{
    //char *c = (char*)malloc(sizeof(char)*len);
    int fd = open(filename, O_RDONLY);
    read(fd, str, len);
    close(fd);
}

void my_bzero(char* str, int len)
{
    for(int i=0; i < len; i++)
    {
        str[i] = '\0';
    }
}

void CopyField(char* str, char* fld, int sidx, int eidx)
{
    int i;
    int k = 0;
    for(i=sidx; i<eidx;i++)
    {
        if(str[i] && (str[i] != '@' && str[i] != '^'))
        {
            fld[k] = str[i];
            k++;
        }
        else
        {
            fld[k] = '\0';
            break;
        }
    }
}

void ReadFieldFromFilename(char* filename, char* fld, int sidx)
{
    int fd = open(filename, O_RDONLY);
    lseek(fd, sidx, SEEK_SET);
    read(fd, fld, sizeof(fld));
    close(fd);

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

// CREATING TAR FILES
void CalculateChkSum(MyTarHeader *header)
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
    header->chksum[6] = '\0';



}

void PopulateHeaderFromFile(char* filename, MyTarHeader* header)
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
    sprintf(header->size, "%011o", (int)st.st_size);
    grp = getgrgid(st.st_gid);
    sprintf(header->gname, "%s", grp->gr_name);
    pwd = getpwuid(st.st_uid);
    sprintf(header->uname, "%s",  pwd->pw_name);
    sprintf(header->magic, "%s", TMAGIC);
    sprintf(header->version, "%s", TVERSION);
    // sprintf(header->devmajor, "%d", major(st.st_rdev));
    // sprintf(header->devminor, "%d", minor(st.st_rdev));
    header->typeflag = '0';
    CalculateChkSum(header);

}

// READING TAR FILES:
void PopulateHeaderFromTar(int fd, MyTarHeader* header)
{  
    read(fd, header->name, sizeof(header->name));
    read(fd, header->mode, sizeof(header->mode));
    read(fd, header->uid, sizeof(header->uid));
    read(fd, header->gid, sizeof(header->gid));
    read(fd, header->size, sizeof(header->size));
    read(fd, header->mtime, sizeof(header->mtime));
    read(fd, header->chksum, sizeof(header->chksum));
    read(fd, &header->typeflag, 1);
    read(fd, header->linkname, sizeof(header->linkname));
    read(fd, header->magic, sizeof(header->magic));
    read(fd, header->version, sizeof(header->version));
    read(fd, header->uname, sizeof(header->uname));
    read(fd, header->gname, sizeof(header->gname));
    read(fd, header->devmajor, sizeof(header->devmajor));
    read(fd, header->devminor, sizeof(header->devminor));
    read(fd, header->prefix, sizeof(header->prefix));

}

void PopulateHeaderFromString(char* str, MyTarHeader* header)
{
    int mult = 1;

    CopyField(str, header->name, 0*mult, 100*mult);
    CopyField(str, header->mode, 100*mult, 108*mult);
    CopyField(str, header->uid, 108*mult, 116*mult);
    CopyField(str, header->gid, 116*mult, 124*mult);
    CopyField(str, header->size, 124*mult, 136*mult);
    CopyField(str, header->mtime, 136*mult, 148*mult);
    CopyField(str, header->chksum, 148*mult, 156*mult);
    header->typeflag = str[156*mult];
    CopyField(str, header->linkname, 157*mult, 257*mult);
    CopyField(str, header->magic, 257*mult, 263*mult);
    CopyField(str, header->version, 263*mult, 265*mult);
    CopyField(str, header->uname, 265*mult, 297*mult);
    CopyField(str, header->gname, 297*mult, 329*mult);
    CopyField(str, header->devmajor, 329*mult, 337*mult);
    CopyField(str, header->devminor, 337*mult, 345*mult);
    CopyField(str, header->prefix, 345*mult, 500*mult);
}

void ResetTarHeader(MyTarHeader *header)
{
    bzero(header->name, sizeof(header->name));
    bzero(header->mode, sizeof(header->mode));
    bzero(header->uid, sizeof(header->uid));
    bzero(header->gid, sizeof(header->gid));
    bzero(header->size, sizeof(header->size));
    bzero(header->mtime, sizeof(header->mtime));
    bzero(header->chksum, sizeof(header->chksum));
    header->typeflag = '\0';
    bzero(header->linkname, sizeof(header->linkname));
    bzero(header->magic, sizeof(header->magic));
    bzero(header->version, sizeof(header->version));
    bzero(header->uname, sizeof(header->uname));
    bzero(header->gname, sizeof(header->gname));
    bzero(header->devmajor, sizeof(header->devmajor));
    bzero(header->devminor, sizeof(header->devminor));
    bzero(header->prefix, sizeof(header->prefix));
}

void AddDataToFileStruct(MyTarFile* file, int size)
{
    file->_data = (char*)malloc(sizeof(char)*size);
}

void CreateListFromTarFile(char* tar_file, MyTarNode* list)
{
    int fd = open(tar_file, O_RDONLY);
    int dec_size;
    int block_size;
    MyTarFile *file = malloc(sizeof(MyTarFile));
    MyTarHeader *header = malloc(sizeof(MyTarHeader));
    PopulateHeaderFromTar(fd, header);
    file->header = header;

    // lseek(fd, 512, 0); // Move out of first header
    dec_size = OctToDec(header->size);
    if(dec_size % 512 == 0)
    {
        block_size = (dec_size/512) * 512;
    }
    else 
    {
         block_size = (dec_size/512) * 512 + 512;
    }
    printf("Block Size: %d\n", block_size);
    AddDataToFileStruct(file, block_size);
    read(fd, file->_data, block_size);
    list->file = file;
    close(fd);


}

int GetFileSize(MyTarFile* file)
{
    int dec_size;
    dec_size = OctToDec(file->header->size);
    if(dec_size % 512 == 0)
    {
        return (dec_size/512) * 512;
    }
    else 
    {
         return (dec_size/512) * 512 + 512;
    }
}

void PopulateDataFromFile(int fd, MyTarFile* file, int size)
{
    file->_data = (char*)malloc(sizeof(char)*size);
    read(fd, file->_data, size);
}

MyTarHeader* CreateHeaderFromTarFile(int fd, int offset)
{

	MyTarHeader* header = malloc(sizeof(MyTarHeader));
	PopulateHeaderFromTar(fd, header);
	return header;

}

MyTarFile* CreateFileFromTarFile(char* tar_file, int offset)
{
    int fd = open(tar_file, O_RDONLY);
    lseek(fd, offset, 0);
	MyTarFile* file = malloc(sizeof(MyTarFile));
	MyTarHeader* header = malloc(sizeof(MyTarHeader));
	file->header = header;
	PopulateHeaderFromTar(fd, file->header);
	int size = GetFileSize(file);
	PopulateDataFromFile(fd, file, offset);
    close(fd);
	return file;	
}


int main(int argc, char* argv[])
{
    // MyTarHeader *header = malloc(sizeof(MyTarHeader));
    // PopulateHeaderFromFile("test.txt", header);
    // // header->typeflag = REGTYPE;
    // printf("TYPE: %c\n", header->typeflag);
    // print_header(header);
    // printf("oct size: %s\n", header->size);
    // printf("dec size: %d\n", OctToDec(header->size));
    

    // // Read from file
    // printf("\n");
    // MyTarHeader *header2 = malloc(sizeof(MyTarHeader));
    // // CopyField(tstStr, header2->name, 0, 100);
    // // printf("name: %s\n", header2->name);
    // PopulateHeaderFromTar("txt_tar.tar", header2, 0);
    // print_header(header2);
    // free(header);
    // free(header2);

    // struct stat st1;
    // struct stat st2;
    // stat("txt_tar.tar", &st1);
    // stat("test.txt", &st2);

    // printf("tar file size: %lu\n", st1.st_size);
    // printf("tar file blksize: %lu\n", st1.st_blksize);
    // printf("tar file blocks: %lu\n", st1.st_blocks);

    // printf("text file size: %lu\n", st2.st_size);
    // printf("text file blksize: %lu\n", st2.st_blksize);
    // printf("text file blocks: %lu\n", st2.st_blocks);
    char tar_file[] = "txt_tar.tar";

    MyTarNode *node = malloc(sizeof(MyTarNode));
    CreateListFromTarFile("txt_tar.tar", node);
    print_header(node->file->header);
    printf("%s\n", node->file->_data);

    printf("\n");

    MyTarFile *file = CreateFileFromTarFile(tar_file, 0);
    print_header(file->header);
    printf("%s\n", file->_data);
    
    free(node);



    return 0;
}