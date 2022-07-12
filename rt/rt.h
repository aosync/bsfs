#ifndef RT_RT_H
#define RT_RT_H

#include <assert.h>
#include <stdint.h>

#define nil ((void*)0)

void *alloc(size_t n);
void *ralloc(void *ptr, size_t n);

typedef unsigned char uchar;
typedef unsigned short int ushort;
typedef unsigned int uint;
typedef unsigned long int ulong;
typedef long long int vlong;
typedef unsigned long long int uvlong;

typedef char i8;
typedef unsigned char u8;

typedef short int i16;
typedef unsigned short int u16;
static_assert(sizeof(i16) == 2, "i16 is not 16-bits");
static_assert(sizeof(u16) == 2, "u16 is not 16-bits");

typedef int i32;
typedef unsigned int u32;
static_assert(sizeof(i32) == 4, "i32 is not 32-bits");
static_assert(sizeof(u32) == 4, "u32 is not 32-bits");

typedef long long int i64;
typedef unsigned long long int u64;
static_assert(sizeof(i64) == 8, "i64 is not 64-bits");
static_assert(sizeof(u64) == 8, "u64 is not 64-bits");

typedef long long int ssize_t;
static_assert(sizeof(ssize_t) == 8, "ssize_t is not 64-bits");

typedef size_t off_t;

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#define tls __thread
#include <alloca.h>
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#define tls __declspec( thread )
#define alloca _alloca
#endif

#endif