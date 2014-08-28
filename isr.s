.global asm_excehandler
.global asm_inthandler
.global asm_timer,asm_kb
.global asm_sys_call,sys_print,sys_fork,asm_fork,child_ret_from_fork
.global load_idtr
.global load_gdtr

.align 2
asm_excehandler:
  sti
  pushw %es
  pushw %ds
  pushal
  movl %esp,%eax
  pushl %eax
  movw %ss,%ax
  movw %ax,%ds
  movw %ax,%es
  call excehandler
  popl %eax
  popal
  popw %ds
  popw %es
  add 4,%esp     #需要这句
  iretl


.align 2
asm_inthandler:
  pushw %es
  pushw %ds
  pushal
  movl %esp,%eax
  pushl %eax
  movw %ss,%ax
  movw %ax,%ds
  movw %ax,%es
  call inthandler
  
  popl %eax
  popal
  popw %ds
  popw %es
  iretl

.align 2 
asm_timer:
  pushw %es
  pushw %ds
  pushal
  movl %esp,%eax
  pushl %eax
  movw %ss,%ax
  movw %ax,%ds
  movw %ax,%es
  call do_timer
  
  popl %eax
  popal
  popw %ds
  popw %es
  iretl
  
.align 2  
asm_kb:
  pushw %es
  pushw %ds
  pushal
  movl %esp,%eax
  pushl %eax
  movw %ss,%ax
  movw %ax,%ds
  movw %ax,%es
  call do_kb
  
  popl %eax
  popal
  popw %ds
  popw %es
  iretl
 
.align 2 
sys_print:
	pushl	%esi			# bg color
	pushl	%edi			# fg color
	pushl	%ebx			# character to be printed
	cli
	call	print_c
	sti
	addl	$12,	%esp
	ret
.align 2	
child_ret_from_fork:
	popal
	popl %es
	popl %ds
	
	popl %eax    #eip
	popl %eax    #cs
	popl %ebx    #eflags
	popl %ecx    #esp
	popl %edx    #ss
	
	pushl %edx    #ss
	pushl %ecx    #esp
	pushl %ebx    #eflags
	pushl %eax    #cs
	pushl 1f
	jmp .
	iretl
1:
	jmp .
	#movw $0x000f,%cx
	#movw %cx,%fs
	#movw %cx,%gs
	
	#movl 8(%esp),%eax
	#movl %eax,syscall_ret
	#jmp .
	#movl $0,%eax
	#iretl
	
.align 2
sys_fork:
	#pushw %gs			#中断时没有入栈的寄存器入栈
	#pushl %esi
	#pushl %edi
	#pushl %ebp
	#pushl %edx
	call copy_process 	#调用C函数
	#addl $18,%esp 		#丢弃这里所有压栈内容。
	ret

.align 2	
asm_sys_call:
	sti
	pushw %ds
	pushw %es
	pushal			#该句有个pushl %eax,而eax寄存器要保留系统调用的返回值
	
	pushal
	movw %ss,%ax
	movw %ax,%ds
	movw %ax,%es
	call syscall_entry
	movl %eax,syscall_ret
	addl $32,%esp
	
	popal
	movl syscall_ret,%eax	#保留系统调用的返回值
	popw %es
	popw %ds
	iretl
	
.align 2	
asm_fork:
	sti
	pushl %ds
	pushl %es
	
	pushal
	movw %ss,%ax
	movw %ax,%ds
	movw %ax,%es
	call copy_process
	movl %eax,syscall_ret
	addl $32,%esp
	
	movl syscall_ret,%eax	#保留系统调用的返回值
	popl %es
	popl %ds
	iretl
	
	
.align 2 
load_gdtr:		#; void load_gdtr(int limit, int addr);
  mov 4(%esp) ,%ax
  mov %ax,6(%esp)
  lgdt 6(%esp)
  ret
  
.align 2 
load_idtr:		#; void load_idtr(int limit, int addr);
  mov 4(%esp) ,%ax
  mov %ax,6(%esp)
  lidt 6(%esp)
  ret