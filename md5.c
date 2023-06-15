#include <Windows.h>
#include <stdio.h>

const unsigned int ConstTable[4][16] = {
    {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
     0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
     0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
     0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821},
    {0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
     0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
     0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
     0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a},
    {0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
     0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
     0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
     0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665},
    {0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
     0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
     0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
     0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391}
    // ConstTable
};
const unsigned int leftShiftBits[4][16] = {
    {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22},
    {5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20},
    {4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23},
    {6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21}
    // leftShiftBits
};

static inline unsigned int RotateLeft(unsigned int x, unsigned int s)
{
    return (x << s) | (x >> (32 - s));
}

static inline unsigned int F(unsigned int x, unsigned int y, unsigned int z) { return (x & y) | ((~x) & z); }
static inline unsigned int G(unsigned int x, unsigned int y, unsigned int z) { return (x & z) | (y & (~z)); }
static inline unsigned int H(unsigned int x, unsigned int y, unsigned int z) { return x ^ y ^ z; }
static inline unsigned int I(unsigned int x, unsigned int y, unsigned int z) { return y ^ (x | (~z)); }

unsigned char *ToHexStringLE(const unsigned int N)
{
    unsigned int tempN = N;
    unsigned char *hexStr = (unsigned char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(unsigned char) * 8);
    const unsigned char *hexCode = "0123456789abcdef";

    int digitOrder = 0;
    do
    {
        memset(hexStr + digitOrder, hexCode[(tempN & 0xf0) >> 4], 1);
        memset(hexStr + digitOrder + 1, hexCode[(tempN & 0xf)], 1);
        digitOrder += 2;
        tempN = tempN >> 8;
    } while (tempN > 0);

    return hexStr;
}

unsigned char *Padding(const unsigned char *message, const unsigned int length, unsigned int *outBitLength)
{
    unsigned int bitLength = length * 8 + 1;

    long paddingLength = 0L;
    long multiple = 0L;

    for (;;)
    {
        if ((paddingLength = 512 * multiple + 448 - (long)bitLength) >= 0)
        {
            break;
        }
        ++multiple;
    }

    // 將全部的長度填充為 64 bit
    size_t totalLength = bitLength + (unsigned int)paddingLength;

    unsigned char *padMessage = (unsigned char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(unsigned char) * (totalLength / 8 + 8));

    // 信息先寫進去
    memcpy(padMessage, message, length);

    // 補位第一位填 1
    memset(padMessage + length, 0x80, 1);

    // 把 64 bit 字串接到最後面
    size_t bitLengthBeforePadding = length * 8;

    memcpy(padMessage + (totalLength / 8), &bitLengthBeforePadding, sizeof(size_t));

    *outBitLength = totalLength + 64;

    return padMessage;
}

unsigned char *ProcessMessage(const unsigned char *message, const unsigned int bitLength)
{

    unsigned int A = 0x67452301, B = 0xEFCDAB89, C = 0x98BADCFE, D = 0x10325476;

    for (int i = 0; i < (bitLength / 512); ++i)
    {
        unsigned int AA = A, BB = B, CC = C, DD = D;
        unsigned int *Group = (unsigned int *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(unsigned int) * 16);
        memcpy(Group, (unsigned int *)(message + i * 64), sizeof(unsigned int) * 16);

        // Round 1
        int startIndex = 0, diff = 1;
        for (int j = 0; j < 4; ++j)
        {
            AA = BB + RotateLeft(AA + F(BB, CC, DD) + Group[(startIndex + (j * 4) * diff) % 16] + ConstTable[0][j * 4], leftShiftBits[0][j * 4]);
            DD = AA + RotateLeft(DD + F(AA, BB, CC) + Group[(startIndex + (j * 4 + 1) * diff) % 16] + ConstTable[0][j * 4 + 1], leftShiftBits[0][j * 4 + 1]);
            CC = DD + RotateLeft(CC + F(DD, AA, BB) + Group[(startIndex + (j * 4 + 2) * diff) % 16] + ConstTable[0][j * 4 + 2], leftShiftBits[0][j * 4 + 2]);
            BB = CC + RotateLeft(BB + F(CC, DD, AA) + Group[(startIndex + (j * 4 + 3) * diff) % 16] + ConstTable[0][j * 4 + 3], leftShiftBits[0][j * 4 + 3]);
        }

        // Round 2
        startIndex = 1, diff = 5;
        for (int j = 0; j < 4; ++j)
        {
            // leftShiftBits[1][j], ConstTable[1][j] 位置反了
            AA = BB + RotateLeft(AA + G(BB, CC, DD) + Group[(startIndex + (j * 4) * diff) % 16] + ConstTable[1][j * 4], leftShiftBits[1][j * 4]);
            DD = AA + RotateLeft(DD + G(AA, BB, CC) + Group[(startIndex + (j * 4 + 1) * diff) % 16] + ConstTable[1][j * 4 + 1], leftShiftBits[1][j * 4 + 1]);
            CC = DD + RotateLeft(CC + G(DD, AA, BB) + Group[(startIndex + (j * 4 + 2) * diff) % 16] + ConstTable[1][j * 4 + 2], leftShiftBits[1][j * 4 + 2]);
            BB = CC + RotateLeft(BB + G(CC, DD, AA) + Group[(startIndex + (j * 4 + 3) * diff) % 16] + ConstTable[1][j * 4 + 3], leftShiftBits[1][j * 4 + 3]);
        }

        // Round 3
        startIndex = 5, diff = 3;
        for (int j = 0; j < 4; ++j)
        {
            AA = BB + RotateLeft(AA + H(BB, CC, DD) + Group[(startIndex + (j * 4) * diff) % 16] + ConstTable[2][j * 4], leftShiftBits[2][j * 4]);
            DD = AA + RotateLeft(DD + H(AA, BB, CC) + Group[(startIndex + (j * 4 + 1) * diff) % 16] + ConstTable[2][j * 4 + 1], leftShiftBits[2][j * 4 + 1]);
            CC = DD + RotateLeft(CC + H(DD, AA, BB) + Group[(startIndex + (j * 4 + 2) * diff) % 16] + ConstTable[2][j * 4 + 2], leftShiftBits[2][j * 4 + 2]);
            BB = CC + RotateLeft(BB + H(CC, DD, AA) + Group[(startIndex + (j * 4 + 3) * diff) % 16] + ConstTable[2][j * 4 + 3], leftShiftBits[2][j * 4 + 3]);
        }

        // Round 4
        startIndex = 0, diff = 7;
        for (int j = 0; j < 4; ++j)
        {
            AA = BB + RotateLeft(AA + I(BB, CC, DD) + Group[(startIndex + (j * 4) * diff) % 16] + ConstTable[3][j * 4], leftShiftBits[3][j * 4]);
            DD = AA + RotateLeft(DD + I(AA, BB, CC) + Group[(startIndex + (j * 4 + 1) * diff) % 16] + ConstTable[3][j * 4 + 1], leftShiftBits[3][j * 4 + 1]);
            CC = DD + RotateLeft(CC + I(DD, AA, BB) + Group[(startIndex + (j * 4 + 2) * diff) % 16] + ConstTable[3][j * 4 + 2], leftShiftBits[3][j * 4 + 2]);
            BB = CC + RotateLeft(BB + I(CC, DD, AA) + Group[(startIndex + (j * 4 + 3) * diff) % 16] + ConstTable[3][j * 4 + 3], leftShiftBits[3][j * 4 + 3]);
        }
        A += AA;
        B += BB;
        C += CC;
        D += DD;

        HeapFree(GetProcessHeap(), 0, Group);
    }

    unsigned char *hash = (unsigned char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(unsigned char) * 32);

    unsigned char *AHash = ToHexStringLE(A);
    unsigned char *BHash = ToHexStringLE(B);
    unsigned char *CHash = ToHexStringLE(C);
    unsigned char *DHash = ToHexStringLE(D);

    memcpy(hash, AHash, 8);
    memcpy(hash + 8, BHash, 8);
    memcpy(hash + 16, CHash, 8);
    memcpy(hash + 24, DHash, 8);

    HeapFree(GetProcessHeap(), 0, AHash);
    HeapFree(GetProcessHeap(), 0, BHash);
    HeapFree(GetProcessHeap(), 0, CHash);
    HeapFree(GetProcessHeap(), 0, DHash);

    return hash;
}

__declspec(dllexport) unsigned char *Md5Hash(const unsigned char *inputMessage, const unsigned int inputMessageLength)
{
    unsigned int bitLength = 0;
    unsigned char *paddedMessage = Padding(inputMessage, inputMessageLength, &bitLength);

    unsigned char *hashString = ProcessMessage(paddedMessage, bitLength);
    HeapFree(GetProcessHeap(), 0, paddedMessage);

    return hashString;
}

__declspec(dllexport) unsigned char *Md5FileHash(const char *filePath)
{

    if (GetFileAttributes(filePath) == INVALID_FILE_ATTRIBUTES)
    {
        return NULL;
    }

    FILE *fp = fopen(filePath, "rb");

    if (fp == NULL)
    {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    unsigned int fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char *fileBuffer = (unsigned char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(unsigned char) * fileSize);
    fread(fileBuffer, fileSize, sizeof(unsigned char), fp);

    unsigned char *hashString = Md5Hash(fileBuffer, fileSize);

    fclose(fp);
    HeapFree(GetProcessHeap(), 0, fileBuffer);

    return hashString;
}

__declspec(dllexport) void ReleaseHashString(unsigned char **buffer)
{
    if (*buffer == NULL)
    {
        return;
    }

    HeapFree(GetProcessHeap(), 0, *buffer);
    *buffer = NULL;
}
