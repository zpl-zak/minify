// (c) 2016 ZaKlaus; All Rights Reserved

#if !defined(MINIFY_PLATFORM_H)
#define MINIFY_PLATFORM_H

#define ALLOC_MEM(name) void * name(size_t memsize)
typedef ALLOC_MEM(alloc_mem);

#define FREE_MEM(name) void  name(void * memptr)
typedef FREE_MEM(free_mem);

struct PlatformAPI
{
    alloc_mem * AllocMem;
    free_mem * FreeMem;
    
    char * FileContent;
};

#define MINIFY_FILE(name) char* name(PlatformAPI * api)
typedef MINIFY_FILE(minify_file);

#define GET_EXTENSION(name) const char * name()
typedef GET_EXTENSION(get_extension);

#endif