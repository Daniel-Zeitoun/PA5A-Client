.code

NopAsm PROC
	nop
	ret
NopAsm ENDP

Getx64PebAsm PROC
	mov rax, GS:[60h]
	ret
Getx64PebAsm ENDP

Getx86PebAsm PROC
	mov eax, FS:[30h]
	ret
Getx86PebAsm ENDP

IsBeingDebuggedAsm PROC
	call Getx64PebAsm
	movzx eax, byte ptr [rax+2]
	ret
IsBeingDebuggedAsm ENDP

end