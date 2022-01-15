IFDEF RAX

	.code
	GetPebAsm PROC
		mov rax, GS:[60h]
		ret
	GetPebAsm ENDP

	IsBeingDebuggedAsm PROC
		call GetPebAsm
		movzx eax, byte ptr [rax+2]
		ret
	IsBeingDebuggedAsm ENDP

ELSE

	.MODEL FLAT, C
	.code

	GetPebAsm PROC
		ASSUME  FS:NOTHING
		mov eax, FS:[30h]
		ret
	GetPebAsm ENDP

	IsBeingDebuggedAsm PROC
		call GetPebAsm
		movzx eax, byte ptr [eax+2]
		ret
	IsBeingDebuggedAsm ENDP

ENDIF

end
