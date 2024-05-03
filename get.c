#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <ucontext.h>

int n, m;

int foo (int p, int q);


int 
main(int argc, char **argv)
{
    
    ucontext_t con;
    int i, j;
    unsigned int ret;
    

    printf("main: address of main is 0x%x\n", (unsigned int)&main);
    printf("main: address of foo is 0x%x\n", (unsigned int)&foo);

    
    printf("main: address of the argument argc is 0x%x\n", (unsigned int) &argc);
    printf("main: address of the global variable n is 0x%x\n", (unsigned int) &n);
    printf("main: address of the global variable m is 0x%x\n", (unsigned int) &m);
    printf("main: address of the local variable i is 0x%x\n", (unsigned int) &i);
    printf("main: address of the local variable j is 0x%x\n", (unsigned int) &j);
    printf("main: address of the local variable ret is 0x%x\n", (unsigned int) &ret);

    ret = getcontext(&con); // save the current cpu state into context structure con
    
    printf("main: EIP (instruction pointer) saved in context structure con is 0x%x\n",  (unsigned int)  con.uc_mcontext.gregs[REG_EIP]);

    printf("main: ESP (stack pointer) saved in context structure con is 0x%x\n",  (unsigned int)  con.uc_mcontext.gregs[REG_ESP]);

    printf("main: UESP saved in context structure con is 0x%x\n",  (unsigned int)  con.uc_mcontext.gregs[REG_UESP]);
    // maybe we don't need to use REG_UESP in this project.

    printf("main: The uc_stack.ss_sp field value in context structure is 0x%x\n", (unsigned int)con.uc_stack.ss_sp);
    // maybe we don't need to use uc_stack in this project.

    // call a function
    foo(10, 20); // call foo(), which is getting and printing the context.
    
    // after running the program, think about and check the printed addresses (virtual memory addresses).
    
    return (0);
}



int foo(int p, int q) 
{   int ret;
    ucontext_t con;
    
    printf("foo: address of the argument p is 0x%x\n", (unsigned int) &p);
    printf("foo: address of the argument q is 0x%x\n", (unsigned int) &q);
    printf("foo: address of the local variable ret is 0x%x\n", (unsigned int) &ret);

    
    ret = getcontext(&con); // save the current cpu state into context structure con
    
    printf("foo: EIP (instruction pointer) saved in context structure con is 0x%x\n",  (unsigned int)  con.uc_mcontext.gregs[REG_EIP]);
    printf("foo: ESP (stack pointer) saved in context structure con is 0x%x\n",  (unsigned int)  con.uc_mcontext.gregs[REG_ESP]);
    printf("foo: UESP saved in context structure con is 0x%x\n",  (unsigned int)  con.uc_mcontext.gregs[REG_UESP]);
    printf("main: The uc_stack.ss_sp field value in context structure is 0x%x\n", (unsigned int)con.uc_stack.ss_sp);
    return (0);
}

