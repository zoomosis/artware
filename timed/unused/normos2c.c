

void pascal normalize(char *s)
{
    char *tmp = s;

    while (*s)
    {
        if (*s == 0x8d)
            s++;
        else if (*s == '\n')
            s++;
        else
            *tmp++ = *s++;
    }

    *tmp = '\0';
}
