/*
    Copyright 2005-2014 Intel Corporation.  All Rights Reserved.

    This file is part of Threading Building Blocks. Threading Building Blocks is free software;
    you can redistribute it and/or modify it under the terms of the GNU General Public License
    version 2  as  published  by  the  Free Software Foundation.  Threading Building Blocks is
    distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See  the GNU General Public License for more details.   You should have received a copy of
    the  GNU General Public License along with Threading Building Blocks; if not, write to the
    Free Software Foundation, Inc.,  51 Franklin St,  Fifth Floor,  Boston,  MA 02110-1301 USA

    As a special exception,  you may use this file  as part of a free software library without
    restriction.  Specifically,  if other files instantiate templates  or use macros or inline
    functions from this file, or you compile this file and link it with other files to produce
    an executable,  this file does not by itself cause the resulting executable to be covered
    by the GNU General Public License. This exception does not however invalidate any other
    reasons why the executable file might be covered by the GNU General Public License.
*/

#include "proxy.h"
#include "tbb/tbb_config.h"

#if !defined(__EXCEPTIONS) && !defined(_CPPUNWIND) && !defined(__SUNPRO_CC) || defined(_XBOX)
    #if TBB_USE_EXCEPTIONS
        #error Compilation settings do not support exception handling. Please do not set TBB_USE_EXCEPTIONS macro or set it to 0.
    #elif !defined(TBB_USE_EXCEPTIONS)
        #define TBB_USE_EXCEPTIONS 0
    #endif
#elif !defined(TBB_USE_EXCEPTIONS)
    #define TBB_USE_EXCEPTIONS 1
#endif

#if MALLOC_UNIXLIKE_OVERLOAD_ENABLED || MALLOC_ZONE_OVERLOAD_ENABLED

#ifndef __THROW
#define __THROW
#endif

/*** service functions and variables ***/

#include <string.h> // for memset
#include <unistd.h> // for sysconf

static long memoryPageSize;

static inline void initPageSize()
{
    memoryPageSize = sysconf(_SC_PAGESIZE);
}

#if MALLOC_UNIXLIKE_OVERLOAD_ENABLED
#include "Customize.h" // FencedStore
#include <dlfcn.h>
#include <malloc.h>    // mallinfo

/* __TBB_malloc_proxy used as a weak symbol by libtbbmalloc for:
   1) detection that the proxy library is loaded
   2) check that dlsym("malloc") found something different from our replacement malloc
*/
extern "C" void *__TBB_malloc_proxy(size_t) __attribute__ ((alias ("malloc")));

static void *orig_msize;

#elif MALLOC_ZONE_OVERLOAD_ENABLED

#include "proxy_overload_osx.h"

#endif // MALLOC_ZONE_OVERLOAD_ENABLED

// Original (i.e., replaced) functions,
// they are never changed for MALLOC_ZONE_OVERLOAD_ENABLED.
static void *orig_free,
    *orig_realloc;

#if MALLOC_UNIXLIKE_OVERLOAD_ENABLED
#define ZONE_ARG
#define PREFIX(name) name

static void *orig_libc_free,
    *orig_libc_realloc;

// We already tried to find ptr to original functions.
static intptr_t origFuncSearched;

inline void InitOrigPointers()
{
    // race is OK here, as different threads found same functions
    if (!origFuncSearched) {
        orig_free = dlsym(RTLD_NEXT, "free");
        orig_realloc = dlsym(RTLD_NEXT, "realloc");
        orig_msize = dlsym(RTLD_NEXT, "malloc_usable_size");
        orig_libc_free = dlsym(RTLD_NEXT, "__libc_free");
        orig_libc_realloc = dlsym(RTLD_NEXT, "__libc_realloc");

        FencedStore(origFuncSearched, 1);
    }
}

/*** replacements for malloc and the family ***/
extern "C" {
#elif MALLOC_ZONE_OVERLOAD_ENABLED

// each impl_* function has such 1st argument, it's unused
#define ZONE_ARG struct _malloc_zone_t *,
#define PREFIX(name) impl_##name
// not interested in original functions for zone overload
inline void InitOrigPointers() {}

#endif // MALLOC_UNIXLIKE_OVERLOAD_ENABLED and MALLOC_ZONE_OVERLOAD_ENABLED

void *PREFIX(malloc)(ZONE_ARG size_t size) __THROW
{
    return scalable_malloc(size);
}

void *PREFIX(calloc)(ZONE_ARG size_t num, size_t size) __THROW
{
    return scalable_calloc(num, size);
}

void PREFIX(free)(ZONE_ARG void *object) __THROW
{
    InitOrigPointers();
    __TBB_malloc_safer_free(object, (void (*)(void*))orig_free);
}

void *PREFIX(realloc)(ZONE_ARG void* ptr, size_t sz) __THROW
{
    InitOrigPointers();
    return __TBB_malloc_safer_realloc(ptr, sz, orig_realloc);
}

/* The older *NIX interface for aligned allocations;
   it's formally substituted by posix_memalign and deprecated,
   so we do not expect it to cause cyclic dependency with C RTL. */
void *PREFIX(memalign)(ZONE_ARG size_t alignment, size_t size) __THROW
{
    return scalable_aligned_malloc(size, alignment);
}

/* valloc allocates memory aligned on a page boundary */
void *PREFIX(valloc)(ZONE_ARG size_t size) __THROW
{
    if (! memoryPageSize) initPageSize();

    return scalable_aligned_malloc(size, memoryPageSize);
}

#undef ZONE_ARG
#undef PREFIX

#if MALLOC_UNIXLIKE_OVERLOAD_ENABLED

// match prototype from system headers
#if __ANDROID__
size_t malloc_usable_size(const void *ptr) __THROW
#else
size_t malloc_usable_size(void *ptr) __THROW
#endif
{
    InitOrigPointers();
    return __TBB_malloc_safer_msize(const_cast<void*>(ptr), (size_t (*)(void*))orig_msize);
}

int posix_memalign(void **memptr, size_t alignment, size_t size) __THROW
{
    return scalable_posix_memalign(memptr, alignment, size);
}

/* pvalloc allocates smallest set of complete pages which can hold
   the requested number of bytes. Result is aligned on page boundary. */
void *pvalloc(size_t size) __THROW
{
    if (! memoryPageSize) initPageSize();
    // align size up to the page size,
    // pvalloc(0) returns 1 page, see man libmpatrol
    size = size? ((size-1) | (memoryPageSize-1)) + 1 : memoryPageSize;

    return scalable_aligned_malloc(size, memoryPageSize);
}

int mallopt(int /*param*/, int /*value*/) __THROW
{
    return 1;
}

struct mallinfo mallinfo() __THROW
{
    struct mallinfo m;
    memset(&m, 0, sizeof(struct mallinfo));

    return m;
}

#if __ANDROID__
// Android doesn't have malloc_usable_size, provide it to be compatible
// with Linux, in addition overload dlmalloc_usable_size() that presented
// under Android.
size_t dlmalloc_usable_size(const void *ptr) __attribute__ ((alias ("malloc_usable_size")));
#else // __ANDROID__
// Those non-standard functions are exported by GLIBC, and might be used
// in conjunction with standard malloc/free, so we must ovberload them.
// Bionic doesn't have them. Not removing from the linker scripts,
// as absent entry points are ignored by the linker.
void *__libc_malloc(size_t size) __attribute__ ((alias ("malloc")));
void *__libc_calloc(size_t num, size_t size) __attribute__ ((alias ("calloc")));
void *__libc_memalign(size_t alignment, size_t size) __attribute__ ((alias ("memalign")));
void *__libc_pvalloc(size_t size) __attribute__ ((alias ("pvalloc")));
void *__libc_valloc(size_t size) __attribute__ ((alias ("valloc")));

// call original __libc_* to support naive replacement of free via __libc_free etc
void __libc_free(void *ptr)
{
    InitOrigPointers();
    __TBB_malloc_safer_free(ptr, (void (*)(void*))orig_libc_free);
}

void *__libc_realloc(void *ptr, size_t size)
{
    InitOrigPointers();
    return __TBB_malloc_safer_realloc(ptr, size, orig_libc_realloc);
}
#endif // !__ANDROID__

} /* extern "C" */

/*** replacements for global operators new and delete ***/

#include <new>

void * operator new(size_t sz) throw (std::bad_alloc) {
    void *res = scalable_malloc(sz);
#if TBB_USE_EXCEPTIONS
    if (NULL == res)
        throw std::bad_alloc();
#endif /* TBB_USE_EXCEPTIONS */
    return res;
}
void* operator new[](size_t sz) throw (std::bad_alloc) {
    void *res = scalable_malloc(sz);
#if TBB_USE_EXCEPTIONS
    if (NULL == res)
        throw std::bad_alloc();
#endif /* TBB_USE_EXCEPTIONS */
    return res;
}
void operator delete(void* ptr) throw() {
    InitOrigPointers();
    __TBB_malloc_safer_free(ptr, (void (*)(void*))orig_free);
}
void operator delete[](void* ptr) throw() {
    InitOrigPointers();
    __TBB_malloc_safer_free(ptr, (void (*)(void*))orig_free);
}
void* operator new(size_t sz, const std::nothrow_t&) throw() {
    return scalable_malloc(sz);
}
void* operator new[](std::size_t sz, const std::nothrow_t&) throw() {
    return scalable_malloc(sz);
}
void operator delete(void* ptr, const std::nothrow_t&) throw() {
    InitOrigPointers();
    __TBB_malloc_safer_free(ptr, (void (*)(void*))orig_free);
}
void operator delete[](void* ptr, const std::nothrow_t&) throw() {
    InitOrigPointers();
    __TBB_malloc_safer_free(ptr, (void (*)(void*))orig_free);
}

#endif /* MALLOC_UNIXLIKE_OVERLOAD_ENABLED */
#endif /* MALLOC_UNIXLIKE_OVERLOAD_ENABLED || MALLOC_ZONE_OVERLOAD_ENABLED */


#ifdef _WIN32
#include <windows.h>

#if !__TBB_WIN8UI_SUPPORT

#include <stdio.h>
#include "tbb_function_replacement.h"
#include "shared_utils.h"

void __TBB_malloc_safer_delete( void *ptr)
{
    __TBB_malloc_safer_free( ptr, NULL );
}

void* safer_aligned_malloc( size_t size, size_t alignment )
{
    // workaround for "is power of 2 pow N" bug that accepts zeros
    return scalable_aligned_malloc( size, alignment>sizeof(size_t*)?alignment:sizeof(size_t*) );
}

// we do not support _expand();
void* safer_expand( void *, size_t )
{
    return NULL;
}

#define __TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(CRTLIB)                                        \
void (*orig_free_##CRTLIB)(void*);                                                              \
void __TBB_malloc_safer_free_##CRTLIB(void *ptr)                                                \
{                                                                                               \
    __TBB_malloc_safer_free( ptr, orig_free_##CRTLIB );                                         \
}                                                                                               \
                                                                                                \
size_t (*orig_msize_##CRTLIB)(void*);                                                           \
size_t __TBB_malloc_safer_msize_##CRTLIB(void *ptr)                                             \
{                                                                                               \
    return __TBB_malloc_safer_msize( ptr, orig_msize_##CRTLIB );                                \
}                                                                                               \
                                                                                                \
size_t (*orig_aligned_msize_##CRTLIB)(void*, size_t, size_t);                                   \
size_t __TBB_malloc_safer_aligned_msize_##CRTLIB( void *ptr, size_t alignment, size_t offset)   \
{                                                                                               \
    return __TBB_malloc_safer_aligned_msize( ptr, alignment, offset, orig_aligned_msize_##CRTLIB ); \
}                                                                                               \
                                                                                                \
void* __TBB_malloc_safer_realloc_##CRTLIB( void *ptr, size_t size )                             \
{                                                                                               \
    orig_ptrs func_ptrs = {orig_free_##CRTLIB, orig_msize_##CRTLIB};                            \
    return __TBB_malloc_safer_realloc( ptr, size, &func_ptrs );                                 \
}                                                                                               \
                                                                                                \
void* __TBB_malloc_safer_aligned_realloc_##CRTLIB( void *ptr, size_t size, size_t aligment )    \
{                                                                                               \
    orig_ptrs func_ptrs = {orig_free_##CRTLIB, orig_msize_##CRTLIB};                            \
    return __TBB_malloc_safer_aligned_realloc( ptr, size, aligment, &func_ptrs );               \
}

// limit is 30 bytes/60 symbols per line
const char* known_bytecodes[] = {
#if _WIN64
    "4883EC284885C974",       //release free() win64
    "4883EC384885C975",       //release msize() win64
    "4885C974375348",         //release free() 8.0.50727.42 win64
    "48894C24084883EC28BA",   //debug prologue for win64
    "4C8BC1488B0DA6E4040033", //win64 SDK
    "4883EC284885C975",       //release msize() 10.0.21003.1 win64
    "48895C2408574883EC20",   //release _aligned_msize() win64
    "4C894424184889542410",   //debug _aligned_msize() win64
#else
    "558BEC6A018B",           //debug free() & _msize() 8.0.50727.4053 win32
    "6A1868********E8",       //release free() 8.0.50727.4053 win32
    "6A1C68********E8",       //release _msize() 8.0.50727.4053 win32
    "558BEC837D08000F",       //release _msize() 11.0.51106.1 win32
    "8BFF558BEC6A",           //debug free() & _msize() 9.0.21022.8 win32
    "8BFF558BEC83",           //debug free() & _msize() 10.0.21003.1 win32
    "8BFF558BEC8B4508",       //release _aligned_msize() 10.0 win32
    "8BFF558BEC8B4510",       //debug _aligned_msize() 10.0 win32
    "558BEC8B451050",         //debug _aligned_msize() 11.0 win32
#endif
    NULL
    };

#if _WIN64
#define __TBB_ORIG_ALLOCATOR_REPLACEMENT_CALL(CRT_VER)\
    ReplaceFunctionWithStore( #CRT_VER "d.dll", "free",  (FUNCPTR)__TBB_malloc_safer_free_ ## CRT_VER ## d,  known_bytecodes, (FUNCPTR*)&orig_free_ ## CRT_VER ## d );  \
    ReplaceFunctionWithStore( #CRT_VER  ".dll", "free",  (FUNCPTR)__TBB_malloc_safer_free_ ## CRT_VER,       known_bytecodes, (FUNCPTR*)&orig_free_ ## CRT_VER );       \
    ReplaceFunctionWithStore( #CRT_VER "d.dll", "_msize",(FUNCPTR)__TBB_malloc_safer_msize_ ## CRT_VER ## d, known_bytecodes, (FUNCPTR*)&orig_msize_ ## CRT_VER ## d ); \
    ReplaceFunctionWithStore( #CRT_VER  ".dll", "_msize",(FUNCPTR)__TBB_malloc_safer_msize_ ## CRT_VER,      known_bytecodes, (FUNCPTR*)&orig_msize_ ## CRT_VER );      \
    ReplaceFunctionWithStore( #CRT_VER "d.dll", "realloc",         (FUNCPTR)__TBB_malloc_safer_realloc_ ## CRT_VER ## d,         0, NULL); \
    ReplaceFunctionWithStore( #CRT_VER  ".dll", "realloc",         (FUNCPTR)__TBB_malloc_safer_realloc_ ## CRT_VER,              0, NULL); \
    ReplaceFunctionWithStore( #CRT_VER "d.dll", "_aligned_free",   (FUNCPTR)__TBB_malloc_safer_free_ ## CRT_VER ## d,            0, NULL); \
    ReplaceFunctionWithStore( #CRT_VER  ".dll", "_aligned_free",   (FUNCPTR)__TBB_malloc_safer_free_ ## CRT_VER,                 0, NULL); \
    ReplaceFunctionWithStore( #CRT_VER "d.dll", "_aligned_realloc",(FUNCPTR)__TBB_malloc_safer_aligned_realloc_ ## CRT_VER ## d, 0, NULL); \
    ReplaceFunctionWithStore( #CRT_VER  ".dll", "_aligned_realloc",(FUNCPTR)__TBB_malloc_safer_aligned_realloc_ ## CRT_VER,      0, NULL); \
    ReplaceFunctionWithStore( #CRT_VER "d.dll", "_aligned_msize",(FUNCPTR)__TBB_malloc_safer_aligned_msize_ ## CRT_VER ## d, known_bytecodes, (FUNCPTR*)&orig_aligned_msize_ ## CRT_VER ## d ); \
    ReplaceFunctionWithStore( #CRT_VER  ".dll", "_aligned_msize",(FUNCPTR)__TBB_malloc_safer_aligned_msize_ ## CRT_VER,      known_bytecodes, (FUNCPTR*)&orig_aligned_msize_ ## CRT_VER );
#else
#define __TBB_ORIG_ALLOCATOR_REPLACEMENT_CALL(CRT_VER)\
    ReplaceFunctionWithStore( #CRT_VER "d.dll", "free",  (FUNCPTR)__TBB_malloc_safer_free_ ## CRT_VER ## d,  known_bytecodes, (FUNCPTR*)&orig_free_ ## CRT_VER ## d );  \
    ReplaceFunctionWithStore( #CRT_VER  ".dll", "free",  (FUNCPTR)__TBB_malloc_safer_free_ ## CRT_VER,       known_bytecodes, (FUNCPTR*)&orig_free_ ## CRT_VER );       \
    ReplaceFunctionWithStore( #CRT_VER "d.dll", "_msize",(FUNCPTR)__TBB_malloc_safer_msize_ ## CRT_VER ## d, known_bytecodes, (FUNCPTR*)&orig_msize_ ## CRT_VER ## d ); \
    ReplaceFunctionWithStore( #CRT_VER  ".dll", "_msize",(FUNCPTR)__TBB_malloc_safer_msize_ ## CRT_VER,      known_bytecodes, (FUNCPTR*)&orig_msize_ ## CRT_VER );      \
    ReplaceFunctionWithStore( #CRT_VER "d.dll", "realloc",         (FUNCPTR)__TBB_malloc_safer_realloc_ ## CRT_VER ## d,         0, NULL); \
    ReplaceFunctionWithStore( #CRT_VER  ".dll", "realloc",         (FUNCPTR)__TBB_malloc_safer_realloc_ ## CRT_VER,              0, NULL); \
    ReplaceFunctionWithStore( #CRT_VER "d.dll", "_aligned_free",   (FUNCPTR)__TBB_malloc_safer_free_ ## CRT_VER ## d,            0, NULL); \
    ReplaceFunctionWithStore( #CRT_VER  ".dll", "_aligned_free",   (FUNCPTR)__TBB_malloc_safer_free_ ## CRT_VER,                 0, NULL); \
    ReplaceFunctionWithStore( #CRT_VER "d.dll", "_aligned_realloc",(FUNCPTR)__TBB_malloc_safer_aligned_realloc_ ## CRT_VER ## d, 0, NULL); \
    ReplaceFunctionWithStore( #CRT_VER  ".dll", "_aligned_realloc",(FUNCPTR)__TBB_malloc_safer_aligned_realloc_ ## CRT_VER,      0, NULL); \
    ReplaceFunctionWithStore( #CRT_VER "d.dll", "_aligned_msize",(FUNCPTR)__TBB_malloc_safer_aligned_msize_ ## CRT_VER ## d, known_bytecodes, (FUNCPTR*)&orig_aligned_msize_ ## CRT_VER ## d ); \
    ReplaceFunctionWithStore( #CRT_VER  ".dll", "_aligned_msize",(FUNCPTR)__TBB_malloc_safer_aligned_msize_ ## CRT_VER,      known_bytecodes, (FUNCPTR*)&orig_aligned_msize_ ## CRT_VER );
#endif

__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr70d);
__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr70);
__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr71d);
__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr71);
__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr80d);
__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr80);
__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr90d);
__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr90);
__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr100d);
__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr100);
__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr110d);
__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr110);
__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr120d);
__TBB_ORIG_ALLOCATOR_REPLACEMENT_WRAPPER(msvcr120);


/*** replacements for global operators new and delete ***/

#include <new>

#if _MSC_VER && !defined(__INTEL_COMPILER)
#pragma warning( push )
#pragma warning( disable : 4290 )
#endif

void * operator_new(size_t sz) throw (std::bad_alloc) {
    void *res = scalable_malloc(sz);
    if (NULL == res) throw std::bad_alloc();
    return res;
}
void* operator_new_arr(size_t sz) throw (std::bad_alloc) {
    void *res = scalable_malloc(sz);
    if (NULL == res) throw std::bad_alloc();
    return res;
}
void operator_delete(void* ptr) throw() {
    __TBB_malloc_safer_delete(ptr);
}
#if _MSC_VER && !defined(__INTEL_COMPILER)
#pragma warning( pop )
#endif

void operator_delete_arr(void* ptr) throw() {
    __TBB_malloc_safer_delete(ptr);
}
void* operator_new_t(size_t sz, const std::nothrow_t&) throw() {
    return scalable_malloc(sz);
}
void* operator_new_arr_t(std::size_t sz, const std::nothrow_t&) throw() {
    return scalable_malloc(sz);
}
void operator_delete_t(void* ptr, const std::nothrow_t&) throw() {
    __TBB_malloc_safer_delete(ptr);
}
void operator_delete_arr_t(void* ptr, const std::nothrow_t&) throw() {
    __TBB_malloc_safer_delete(ptr);
}

const char* modules_to_replace[] = {
    "msvcr80d.dll",
    "msvcr80.dll",
    "msvcr90d.dll",
    "msvcr90.dll",
    "msvcr100d.dll",
    "msvcr100.dll",
    "msvcr110d.dll",
    "msvcr110.dll",
    "msvcr120d.dll",
    "msvcr120.dll",
    "msvcr70d.dll",
    "msvcr70.dll",
    "msvcr71d.dll",
    "msvcr71.dll",
    };

/*
We need to replace following functions:
malloc
calloc
_aligned_malloc
_expand (by dummy implementation)
??2@YAPAXI@Z      operator new                         (ia32)
??_U@YAPAXI@Z     void * operator new[] (size_t size)  (ia32)
??3@YAXPAX@Z      operator delete                      (ia32)
??_V@YAXPAX@Z     operator delete[]                    (ia32)
??2@YAPEAX_K@Z    void * operator new(unsigned __int64)   (intel64)
??_V@YAXPEAX@Z    void * operator new[](unsigned __int64) (intel64)
??3@YAXPEAX@Z     operator delete                         (intel64)
??_V@YAXPEAX@Z    operator delete[]                       (intel64)
??2@YAPAXIABUnothrow_t@std@@@Z      void * operator new (size_t sz, const std::nothrow_t&) throw()  (optional)
??_U@YAPAXIABUnothrow_t@std@@@Z     void * operator new[] (size_t sz, const std::nothrow_t&) throw() (optional)

and these functions have runtime-specific replacement:
realloc
free
_msize
_aligned_realloc
_aligned_free
*/

typedef struct FRData_t {
    //char *_module;
    const char *_func;
    FUNCPTR _fptr;
    FRR_ON_ERROR _on_error;
} FRDATA;

FRDATA routines_to_replace[] = {
    { "malloc",  (FUNCPTR)scalable_malloc, FRR_FAIL },
    { "calloc",  (FUNCPTR)scalable_calloc, FRR_FAIL },
    { "_aligned_malloc",  (FUNCPTR)safer_aligned_malloc, FRR_FAIL },
    { "_expand",  (FUNCPTR)safer_expand, FRR_IGNORE },
#if _WIN64
    { "??2@YAPEAX_K@Z", (FUNCPTR)operator_new, FRR_FAIL },
    { "??_U@YAPEAX_K@Z", (FUNCPTR)operator_new_arr, FRR_FAIL },
    { "??3@YAXPEAX@Z", (FUNCPTR)operator_delete, FRR_FAIL },
    { "??_V@YAXPEAX@Z", (FUNCPTR)operator_delete_arr, FRR_FAIL },
#else
    { "??2@YAPAXI@Z", (FUNCPTR)operator_new, FRR_FAIL },
    { "??_U@YAPAXI@Z", (FUNCPTR)operator_new_arr, FRR_FAIL },
    { "??3@YAXPAX@Z", (FUNCPTR)operator_delete, FRR_FAIL },
    { "??_V@YAXPAX@Z", (FUNCPTR)operator_delete_arr, FRR_FAIL },
#endif
    { "??2@YAPAXIABUnothrow_t@std@@@Z", (FUNCPTR)operator_new_t, FRR_IGNORE },
    { "??_U@YAPAXIABUnothrow_t@std@@@Z", (FUNCPTR)operator_new_arr_t, FRR_IGNORE }
};

#ifndef UNICODE
void ReplaceFunctionWithStore( const char*dllName, const char *funcName, FUNCPTR newFunc, const char ** opcodes, FUNCPTR* origFunc )
#else
void ReplaceFunctionWithStore( const wchar_t *dllName, const char *funcName, FUNCPTR newFunc, const char ** opcodes, FUNCPTR* origFunc )
#endif
{
    FRR_TYPE type = ReplaceFunction( dllName, funcName, newFunc, opcodes, origFunc );
    if (type == FRR_NODLL) return;
    if ( type != FRR_OK )
    {
        fprintf(stderr, "Failed to replace function %s in module %s\n",
                funcName, dllName);
        exit(1);
    }
}

void doMallocReplacement()
{
    // Replace functions and keep backup of original code (separate for each runtime)
    __TBB_ORIG_ALLOCATOR_REPLACEMENT_CALL(msvcr70)
    __TBB_ORIG_ALLOCATOR_REPLACEMENT_CALL(msvcr71)
    __TBB_ORIG_ALLOCATOR_REPLACEMENT_CALL(msvcr80)
    __TBB_ORIG_ALLOCATOR_REPLACEMENT_CALL(msvcr90)
    __TBB_ORIG_ALLOCATOR_REPLACEMENT_CALL(msvcr100)
    __TBB_ORIG_ALLOCATOR_REPLACEMENT_CALL(msvcr110)
    __TBB_ORIG_ALLOCATOR_REPLACEMENT_CALL(msvcr120)

    // Replace functions without storing original code
    for ( size_t j=0; j < arrayLength(modules_to_replace); j++ )
        for (size_t i = 0; i < arrayLength(routines_to_replace); i++)
        {
#if !_WIN64
            // in Microsoft* Visual Studio* 2012 and 2013 32-bit operator delete consists of 2 bytes only: short jump to free(ptr);
            // replacement should be skipped for this particular case.
            if ( ((strcmp(modules_to_replace[j], "msvcr110.dll") == 0) || (strcmp(modules_to_replace[j], "msvcr120.dll") == 0)) && (strcmp(routines_to_replace[i]._func, "??3@YAXPAX@Z") == 0)) continue;
            // in Microsoft* Visual Studio* 2013 32-bit operator delete[] consists of 2 bytes only: short jump to free(ptr);
            // replacement should be skipped for this particular case.
            if ((strcmp(modules_to_replace[j], "msvcr120.dll") == 0) && (strcmp(routines_to_replace[i]._func, "??_V@YAXPAX@Z") == 0)) continue;
#endif
            FRR_TYPE type = ReplaceFunction( modules_to_replace[j], routines_to_replace[i]._func, routines_to_replace[i]._fptr, NULL, NULL );
            if (type == FRR_NODLL) break;
            if (type != FRR_OK && routines_to_replace[i]._on_error==FRR_FAIL)
            {
                fprintf(stderr, "Failed to replace function %s in module %s\n",
                        routines_to_replace[i]._func, modules_to_replace[j]);
                exit(1);
            }
        }
}

#endif // !__TBB_WIN8UI_SUPPORT

extern "C" BOOL WINAPI DllMain( HINSTANCE hInst, DWORD callReason, LPVOID reserved )
{

    if ( callReason==DLL_PROCESS_ATTACH && reserved && hInst ) {
#if !__TBB_WIN8UI_SUPPORT
#if TBBMALLOC_USE_TBB_FOR_ALLOCATOR_ENV_CONTROLLED
        char pinEnvVariable[50];
        if( GetEnvironmentVariable("TBBMALLOC_USE_TBB_FOR_ALLOCATOR", pinEnvVariable, 50))
        {
            doMallocReplacement();
        }
#else
        doMallocReplacement();
#endif
#endif // !__TBB_WIN8UI_SUPPORT
    }

    return TRUE;
}

// Just to make the linker happy and link the DLL to the application
extern "C" __declspec(dllexport) void __TBB_malloc_proxy()
{

}

#endif //_WIN32
