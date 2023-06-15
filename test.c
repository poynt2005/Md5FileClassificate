#include <Windows.h>
#include <stdio.h>

int main()
{
    char s[MAX_PATH] = {'\0'};
    GetCurrentDirectory(MAX_PATH, s);

    printf("%s\n", s);

    return 0;
}