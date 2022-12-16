#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef TEST_S
#define TEST_S
typedef struct s_char_struct
{                                
    char name[100];               
    char typeflag;                

}CharStruct;
#endif

void set_char(char* a, char b)
{
    *a = b;
}

void set_struct_vals( CharStruct* cs)
{
    sprintf(cs->name, "%s", "test_name");
    cs->typeflag = '1';
}

int main()
{
    char a;
    CharStruct *cs = malloc(sizeof(CharStruct));
    set_struct_vals(cs);
    printf("Name: %s\n", cs->name);
    printf("Type: %c\n", cs->typeflag);
    free(cs);


    return 0;

}