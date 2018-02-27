#include <stdio.h>
#include <string.h>
#include <msgapi.h>
#include <progprot.h>

int main(int argc, char **argv)
{
    int i;

    if (argc < 2)
    {
        fprintf(stderr,
                "Calculates CRC32 values as used in timEd config.c\n\n");
        fprintf(stderr, "Usage: %s [keyword] [keyword] [...]\n", argv[0]);
        return 1;
    }

    for (i = 1; i < argc; i++)
    {
        char buf[1024];
        dword crc;

        strcpy(buf, argv[i]);
        strupr(buf);
        crc = JAMsysCrc32(buf, strlen(buf), -1L);
        printf("%-20s : 0x%08xL\n", buf, crc);
    }

    return 0;
}
