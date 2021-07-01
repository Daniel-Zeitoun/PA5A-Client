
.code

nopAsm proc
	nop
	ret
nopAsm endp

getPeb proc
	mov   rax,qword ptr gs:[60h]
	movzx eax,byte ptr [rax+2]
	ret
getPeb endp

end