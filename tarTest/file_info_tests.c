#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>



int main()
{
    struct stat st;
    struct group *grp;
    struct passwd *pwd;
    FILE *fptr;
    FILE *tptr;
    fptr = fopen("test.txt", "r");
    tptr = fopen("test_text.tar", "r");
    if(fptr == NULL)
    {
        printf("Error!");
        exit(1);
    }
    if(tptr == NULL)
    {
        printf("Error!");
        exit(1);
    }

    fseek(fptr, SEEK_SET, SEEK_END);
    printf("num chars in text: %lu\n", ftell(fptr));
    fseek(tptr, SEEK_SET, SEEK_END);
    printf("num chars in tar: %lu\n", ftell(tptr));

    fclose(tptr);
    fclose(fptr);

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

    printf("len: %lu\n", strlen("test.txt^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@"));

     

    return 0;
}
