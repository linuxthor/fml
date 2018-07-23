#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mcheck.h>
#include <sqlite3.h>

#define QUOTE(...) #__VA_ARGS__

extern void *__libc_malloc(size_t size);
extern void __libc_free(void *ptr);

static void init_hook(void) __attribute__ ((constructor));

sqlite3 *db;
int hook_active = 0;

static void init_hook(void)
{
    printf("\n[+] Memory profiler starting\n\n");
    sqlite3_open("malloc.db",&db);
    if(db != NULL)
    {
        hook_active = 1;
    }
}

void create_db(void)
{
    char *err_msg = 0;
    int rc;

    const char *create_query = QUOTE(
        CREATE TABLE IF NOT EXISTS trace ( type INT, size INT,
                                           address TEXT, caller TEXT ));
    rc = sqlite3_exec(db, create_query, 0, 0, &err_msg);
    if (rc != SQLITE_OK )
    {
        sqlite3_free(err_msg);
    }
}

void update_db(int type, size_t size, char *address, char *caller)
{
    char *insrt, *err_msg;
    int rc;

    insrt = sqlite3_mprintf("INSERT INTO trace VALUES(%d, %d, %Q, %Q)",type,
                                                        size, address, caller);
    rc = sqlite3_exec(db, insrt, 0, 0, &err_msg);
    if (rc != SQLITE_OK )
    {
        sqlite3_free(err_msg);
    }
}

void * malloc_hook (size_t size, void *caller)
{
    void *result;
    char res[32], cal[32];

    hook_active = 0;

    result = malloc(size);
    sprintf(res, "%p", result);
    sprintf(cal, "%p", caller);
    create_db();
    update_db(1, size, res, cal);

    hook_active = 1;
    return result;
}

void free_hook(void *ptr, void *caller)
{
    char pchr[32], cal[32];

    hook_active = 0;

    sprintf(pchr, "%p", ptr);
    sprintf(cal,  "%p", caller);
    create_db();
    update_db(0, 0, pchr, cal);
    free(ptr);

    hook_active = 1;
}

void * malloc (size_t size)
{
    void *caller = __builtin_return_address(0);
    if (hook_active)
    {
        return malloc_hook(size, caller);
    }
    return __libc_malloc(size);
}

void free (void *ptr)
{
    void *caller = __builtin_return_address(0);
    if (hook_active)
    {
        free_hook(ptr, caller);
        return;
    }
    __libc_free(ptr);
    return;
}
