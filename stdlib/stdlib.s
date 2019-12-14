; this implements some libc like routines to run ascc-compiled
; assembly snippets in agssim.
;
; it's meant to be concatenated with assembly files generated
; by ascc (check make-executable.py).
;
; this is not compatible with the AGS "stdlib" in agsdefns.sh
; therefore you need to override -i systemhdr directory to point
; to some non-existing location, or to some dir with an empty
; agsdefns.sh in it.
;
; agssim execution starts top down, so we jump to a C-ABI like
; entry point called _start, which has the purpose to set things
; up for the call to main(), and exit after it returned.
;
jmpi _start
strlen$1:
; load arg1 (ptr) into mar
        ptrstack 8
        memread4 mar
; save start addr
	push mar
label_strlen_loop1:
	memread1 ax
	jzi label_strlen_break1
	addi mar, 1
	jmpi label_strlen_loop1
label_strlen_break1:
	mr ax, mar
	pop mar
	sub ax, mar
	ret
memcpy$3:
; return addr
	pop ax
; ptrstack 8 : dst
; ptrstack 12 : src
; ptrstack 16 : len
; dst
	pop bx
; src
	pop cx
; len
	pop dx
	addi sp, 16
; save dst, it is required as return value
	push bx
	add dx, bx ; dx: dst+len
label_memcpy_loop1:
	mr ax, bx
	gte ax, dx
	jnzi label_memcpy_break1
	mr mar, cx
	memread1 ax
	mr mar, bx
	memwrite1 ax
	addi bx, 1
	addi cx, 1
	jmpi label_memcpy_loop1
label_memcpy_break1:
	pop ax
	ret
strcpy$2: ; arg1: dest, arg2: src
	ptrstack 12
	memread4 ax
	push ax
	li ax strlen$1
	call ax
	pop bx
; copy trailing zero byte too
	addi ax, 1
	push ax
	ptrstack 12
	memread4 ax
	push bx
	push ax
	li ax, memcpy$3
	call ax
	subi sp, 12
	ret
print$1:
	ptrstack 8
	memread4 cx
	push cx
	li ax, strlen$1
	call ax
	pop cx
	mr dx, ax
	li ax, 1
	li bx, 1
	callscr ax
	ret
puts$1:
	ptrstack 8
	memread4 ax
	push ax
	li ax, print$1
	call ax
	ptrstack 4
	memcpy 4, 10
	push mar
	li ax, print$1
	call ax
	subi sp, 8
	ret
_start:
	mr mar, op
	memread4 ax
	addi mar, 4
;	memread4 mar
	push mar
	push ax
	li ax, main
	call ax
	mr bx, ax
	li ax, 60
	callscr ax
