#pragma inline


int pascal IsQuote(char *line)
{


   asm mov esi,dword ptr[ebp+8]

   asm mov cx,7       /* No more than 7 characters */
   asm cld            /* We move forward           */

again:
   asm lodsb         /* Get a char in al           */
   asm or al,al      /* Is it a 0? End of string?  */
   asm jz nogo       /* Yes, return 0              */
   asm cmp al,60     /* Is it a '<' ?              */
   asm je nogo       /* Yes, return 0              */
   asm cmp al,62     /* Is it '>' ?                */
   asm loopne again  /* no, Load next char         */
   asm jne nogo      /* 7 chars scanned, return 0  */
   asm mov eax,1      /* Lets return 1              */
   asm jmp getout    /* And get out                */

nogo:
   asm xor eax,eax     /* Return 0                   */

getout:

}

