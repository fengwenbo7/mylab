#ifndef _BLOOMFILTER_H__
#define _BLOOMFILTER_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define _BLOOMFILTER_VERSION__ "1.1"
#define _MGAIC_CODE__ (0x01464C42)

#pragma pack(1)

typedef struct{
    uint8_t cInitFlag;
    uint8_t cResv[3];

    uint32_t dwMaxItems;//n-max elements
    double dProbFalse;//p-error jugle ratio
    uint32_t dwFilterBits;//m-bits number
    uint32_t dwHashFuncs;//k-count of hash functions

    uint32_t dwSeed;
    uint32_t dwCount;

    uint32_t dwFilterSize;
    unsigned char * pstFilter;
    uint32_t *pdwHashPos;
}BaseBloomFilter;

typedef struct{
    uint32_t dwMagicCode;
    uint32_t dwSeed;
    uint32_t dwCount;

    uint32_t dwMaxItems;
    double dProbFalse;
    uint32_t dwFilterBits;
    uint32_t dwHashFuncs;

    uint32_t dwResv[6];
    uint32_t dwFileCrc;
    uint32_t dwFileterSize;
}BloomFileHead;

#pragma pack()

static inline void calcBloomFilterParam(uint32_t)

#endif