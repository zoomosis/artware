#include "includes.h"
#include "unused.h"

#define MEM_LIMIT 64 * 1024

static void mem_logwrite(const char *str)
{
#ifdef MEM_LOGWRITE
    FILE *fp;
    fp = fopen("timed.log", "a+");
    fprintf(fp, "%s\n", str);
    fclose(fp);
#else
    unused(str);
#endif
}

static void mem_log(const char *func, unsigned size)
{
    char tmp[1024];
    sprintf(tmp, "%s(): %d bytes requested", func, size);
    mem_logwrite(tmp);
}

void *mem_malloc(unsigned n)
{
    void *p;

    mem_log("malloc", n);

    if (n > MEM_LIMIT)
    {
	char tmp[1024];
	sprintf(tmp, "Memory requested was above limit of %d bytes", MEM_LIMIT);
        mem_logwrite(tmp);
	return NULL;
    }
    
    p = malloc(n);
    
    if (p == NULL)
    {
        cls();
        MoveXY(1, 1);
        printf("Attempt to allocate memory failed (malloc)!\n");
        printf("Amount requested : %u bytes\n", n);
        printf("Further execution impossible. Press a key twice to exit\n");
        kbflush();
        get_idle_key(1, GLOBALSCOPE);
        get_idle_key(1, GLOBALSCOPE);
        exit(254);
    }

    memset(p, 0xFF, n);         /* Make memory 'dirty' */

    return p;
}

void *mem_calloc(unsigned n, unsigned t)
{
    void *p;

    mem_log("calloc", n * t);

    p = calloc(n, t);

    if (p == NULL)
    {
        cls();
        MoveXY(1, 1);
        printf("Attempt to allocate memory failed (calloc)!\n");
        printf("Amount requested : %u * %u bytes\n", n, t);
        printf("Further execution impossible. Press a key twice to exit\n");
        kbflush();
        get_idle_key(1, GLOBALSCOPE);
        get_idle_key(1, GLOBALSCOPE);
        exit(254);
    }

    return p;
}

void *mem_realloc(void *org, unsigned n)
{
    void *p = NULL;

    mem_log("realloc", n);

    if (n == 0)                 /* Free block */
    {
        if (NULL != org)
        {
            mem_free(org);
        }
    }
    else
    {
	p = realloc(org, n);

        if (p == NULL)
        {
            cls();
            MoveXY(1, 1);
            printf("Attempt to re-allocate memory failed (realloc)!\n");
            printf("Amount requested : %u bytes\n", n);
            printf("Further execution impossible. Press a key twice to exit\n");
            kbflush();
            get_idle_key(1, GLOBALSCOPE);
            get_idle_key(1, GLOBALSCOPE);
            exit(254);
        }
    }

    return p;
}

char *mem_strdup(char *s)
{
    char *p;

    mem_log("strdup", strlen(s) + 1);

    p = strdup(s);
    
    if (p == NULL)
    {
        cls();
        MoveXY(1, 1);
        printf("Attempt to duplicate string failed (strdup)!\n");
        printf("Amount requested : %lu bytes\n", (unsigned long) strlen(s));
        printf("Further execution impossible. Press a key twice to exit\n");
        kbflush();
        get_idle_key(1, GLOBALSCOPE);
        get_idle_key(1, GLOBALSCOPE);
        exit(254);
    }

    return p;
}

void mem_free(void *p)
{
    mem_log("free", 0);

    if (p == NULL)
    {
        Message("Attempt to free a NULL pointer!", -1, 0, YES);
        return;
    }

    free(p);
}
