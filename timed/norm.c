
void _pascal normalize(char *s)
{
    char   *tmp = s;

    while (*s)
        if ((unsigned) (0xff & *s) == (unsigned) 0x8d)
            s++;
        else if (*s == 0x0a)
            s++;
        else if (*s == 0x0d)
            s++, *tmp++ = '\n';
        else {
            *tmp++ = (char) DOROT13((int) *s);
            s++;
        }
    *tmp = '\0';
}

