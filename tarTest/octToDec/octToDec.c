#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

int main()
{
    char octalstring[] = "00000067";
    char strippedString[12];
    StripZeros(octalstring, strippedString);
    int pwr = strlen(strippedString) - 1;
    int dec = 0;
    int i =0;
    while(pwr >= 0)
    {
        dec += (strippedString[i] - '0')*power(8,pwr);
        pwr--;
        i++;
    }

    // printf("8 to power of 1== %d\n", power(8,1));

    printf("%d\n", dec);

    // block_size test

    int bs = 33;
    int bs2 = 2400;

    printf("bs1 = %d\n", (bs/512) * 512 + 512);
    printf("bs2 = %d\n", (bs2/512) * 512 + 512);

    return 0;
}