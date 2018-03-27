include d:\masm32\include\masm32rt.inc

.xmm
.code
__@function0:
enter 0, 1
leave
ret 0

start:
call __@function0
exit
end start