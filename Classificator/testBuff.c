#include <stdio.h>
#include "MyBuff.h"

int main()
{

    // MyBuff *buff = NewBuff();
    // printf("New buff size: %d\n", HeapSize(GetProcessHeap(), 0, buff));

    // char *s1 = "abc";
    // char *s2 = "def";
    // char *s3 = "ghi";
    // char *s4 = "jkl";
    // char *s5 = "mnt";
    // char *s6 = "123";
    // char *s7 = "456";

    // // PushBuff(buff, s1, 0);
    // // PushBuff(buff, s2, 0);
    // // PushBuff(buff, s3, 0);
    // // PushBuff(buff, s4, 0);
    // // PushBuff(buff, s5, 0);
    // // PushBuff(buff, s6, 0);
    // // PushBuff(buff, s7, 0);

    // PushBuff(buff, s1, strlen(s1) + 1);
    // PushBuff(buff, s2, strlen(s1) + 1);
    // PushBuff(buff, s3, strlen(s1) + 1);
    // PushBuff(buff, s4, strlen(s1) + 1);
    // PushBuff(buff, s5, strlen(s1) + 1);
    // PushBuff(buff, s6, strlen(s1) + 1);
    // PushBuff(buff, s7, strlen(s1) + 1);

    // // for (int i = 0; i < 100; i++)
    // // {
    // //     char *q = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 5);
    // //     const char *w = "wooo";
    // //     memcpy(q, w, strlen(w));
    // //     PushBuff(buff, q);
    // // }

    // printf("HasBuff: %s: %d\n", s4, HasBuff(buff, s4));
    // printf("IndexOfBuff: %s: %d\n", s4, IndexOfBuff(buff, s4));
    // printf("IndexOfBuff: %s: %d\n", "Nigga", IndexOfBuff(buff, "Nigga"));
    // printf("EraseBuff: %s: %d\n", s5, EraseBuff(buff, IndexOfBuff(buff, s5)));

    // int cap = CapsBuff(buff);
    // printf("caps: %d\n", cap);

    // int size = SizeBuff(buff);
    // printf("size: %d\n", size);

    // for (int i = 0; i < size; ++i)
    // {
    //     int status = -1;
    //     char *el = AtBuff(buff, i, &status);

    //     if (status == NODE_ASSABLE)
    //     {
    //         printf("idx: %d, el: %s\n", i, el);
    //     }
    // }
    // printf("Before DeAlloc: %d\n", HeapSize(GetProcessHeap(), 0, buff));
    // FreeBuff(&buff);

    // printf("End\n");

    MyBuff *fileNameBuff = NewBuff();
    FreeBuff(&fileNameBuff);

    return 0;
}