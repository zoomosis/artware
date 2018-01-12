;***************************************************************************
;*                                                                         *
;*  Squish Developers Kit Source, Version 2.00                             *
;*  Copyright 1989-1994 by SCI Communications.  All rights reserved.       *
;*                                                                         *
;*  USE OF THIS FILE IS SUBJECT TO THE RESTRICTIONS CONTAINED IN THE       *
;*  SQUISH DEVELOPERS KIT LICENSING AGREEMENT IN SQDEV.PRN.  IF YOU DO NOT *
;*  FIND THE TEXT OF THIS AGREEMENT IN THE AFOREMENTIONED FILE, OR IF YOU  *
;*  DO NOT HAVE THIS FILE, YOU SHOULD IMMEDIATELY CONTACT THE AUTHOR AT    *
;*  ONE OF THE ADDRESSES LISTED BELOW.  IN NO EVENT SHOULD YOU PROCEED TO  *
;*  USE THIS FILE WITHOUT HAVING ACCEPTED THE TERMS OF THE SQUISH          *
;*  DEVELOPERS KIT LICENSING AGREEMENT, OR SUCH OTHER AGREEMENT AS YOU ARE *
;*  ABLE TO REACH WITH THE AUTHOR.                                         *
;*                                                                         *
;*  You can contact the author at one of the address listed below:         *
;*                                                                         *
;*  Scott Dudley       FidoNet     1:249/106                               *
;*  777 Downing St.    Internet    sjd@f106.n249.z1.fidonet.org            *
;*  Kingston, Ont.     CompuServe  >INTERNET:sjd@f106.n249.z1.fidonet.org  *
;*  Canada  K7M 5N3    BBS         1-613-634-3058, V.32bis                 *
;*                                                                         *
;***************************************************************************
; Find first file in set (used by FFIND only - do not call directly)

IFDEF __FLAT__

        .386p
        .model small, pascal
        .code

        PUBLIC __DFINDFIRST, __DFINDNEXT

__DFINDFIRST proc filename:dword, attr:dword, dta:dword
        mov     edx, dta                        ; Load offset of DTA
        mov     ah, 01ah                        ; Set DTA
        int     21h                             ; Do call

        mov     ecx, attr                        ; Attribute for search
        mov     edx, filename                   ; File to search for
        mov     ah, 04eh                        ; FindFirst
        int     21h

        jc      done                            ; If carry set, error occured
        xor     eax,eax                         ; Otherwise, RC=0.
done:   ret
__DFINDFIRST endp


; Find next file in set (used by FFIND only - do not call directly)

__DFINDNEXT proc dta:dword
        mov     edx, dta                        ; Load offset of DTA
        mov     ah, 01ah                        ; Set DTA
        int     21h                             ; Do call

        mov     ah, 04fh                        ; FindNext
        int     21h

        jc      donen                           ; If carry set, error occured
        xor     eax,eax                         ; Otherwise, RC=0.
donen:  ret
__DFINDNEXT endp

ELSE  ; !__FLAT__

.model   large, pascal
.code

PUBLIC __DFINDFIRST, __DFINDNEXT


__DFINDFIRST proc filename:dword, attr:word, dta:dword
        push    ds                              ; Save DS

        lds     dx, dta
;        mov     dx, word ptr [dta]              ; Load dta into ds:dx
;        mov     ax, word ptr [dta+2]
;        mov     ds, ax

        mov     ah, 01ah                        ; Set DTA
        int     21h                             ; Do call
        pop     ds                              ; Restore DS

        push    ds

        lds     dx, filename
;        mov     dx, word ptr [filename]         ; Load dta into ds:dx
;        mov     ax, word ptr [filename+2]
;        mov     ds, ax

        mov     cx, attr                        ; Attribute for search
        mov     ah, 04eh                        ; FindFirst
        int     21h
        pop     ds

        jc      done                            ; If carry set, error occured
        xor     ax,ax                           ; Otherwise, RC=0.
done:   ret
__DFINDFIRST endp


; Find next file in set (used by FFIND only - do not call directly)

__DFINDNEXT proc dta:dword
        push    ds                              ; Save DS

        lds     dx, dta
;        mov     dx, word ptr [dta]              ; Load dta into ds:dx
;        mov     ax, word ptr [dta+2]
;        mov     ds, ax

        mov     ah, 01ah                        ; Set DTA
        int     21h                             ; Do call
        pop     ds                              ; Restore DS

        mov     ah, 04fh                        ; FindNext
        int     21h

        jc      donen                           ; If carry set, error occured
        xor     ax,ax                           ; Otherwise, RC=0.
donen:  ret
__DFINDNEXT endp


ENDIF

end

