;
;
; assembler implementation of normalize for speed
;
; PUBLIC DOMAIN
;
;


public NORMALIZE

.model large
.code


NORMALIZE proc far

	push bp
	mov bp,sp

	push di
	push si
	push ds


start:  les di,[bp+6]
	push es
	pop ds
	mov si,di

l1:     lodsb
	cmp al,8dh
	je l1
	cmp al,0ah
	je l1
	or al,0
	jz l3
	stosb
	jmp l1

l3:     stosb

	pop ds
	pop si
	pop di

	pop bp
	ret 4

NORMALIZE endp

end
