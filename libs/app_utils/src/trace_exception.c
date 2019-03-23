/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>
#include <dlfcn.h>
#include "debug.h"
#include "misc_utils.h"
#include "symbol_utils.h"

#if defined(__i386) || defined( __x86_64)
#if defined(__i386)
#define    UL_FMT_STR    "0x%08lx"
#define    REG_BP_NO     REG_EBP
#define    REG_PC_NO     REG_EIP
#define    REG_SP_NO     REG_ESP
static const char *reg_names[] = 
{
  "GS",
  "FS",
  "ES",
  "DS",
  "EDI",
  "ESI",
  "EBP",
  "ESP",
  "EBX",
  "EDX",
  "ECX",
  "EAX",
  "TRAPNO",
  "ERR",
  "EIP",
  "CS",
  "EFL",
  "UESP",
  "SS",
};
#elif defined( __x86_64)
#define    UL_FMT_STR    "0x%016lx"
#define    REG_BP_NO     REG_RBP
#define    REG_PC_NO     REG_RIP
#define    REG_SP_NO     REG_RSP
static const char *reg_names[] = 
{
  "R8",
  "R9",
  "R10",
  "R11",
  "R12",
  "R13",
  "R14",
  "R15",
  "RDI",
  "RSI",
  "RBP",
  "RBX",
  "RDX",
  "RAX",
  "RCX",
  "RSP",
  "RIP",
  "EFL",
  "CSGSFS",
  "ERR",
  "TRAPNO",
  "OLDMASK",
  "CR2",
};
#endif

static void print_gregs(ucontext_t *pt_ucontext)
{
    int i;
    
    printf("value of general regs:\n");
    for(i = 0; i < NGREG; i++)
    {
        if (i % 4 == 0) printf("\n");
        printf("%-8s=" UL_FMT_STR "    "
            , reg_names[i], (unsigned long)(pt_ucontext->uc_mcontext.gregs[i]));
    }
    printf("\n\n\n");

}

static void print_call_links(unsigned long *reg_bp, unsigned long reg_pc)
{
    Dl_info dlinfo;
   
    printf("function call links:\n");
    
    while (reg_bp!=NULL)
    {
    //DBG_PRINT("%p", reg_bp);
       if(!addr2symbol_info((void *)reg_pc, &dlinfo))
            break;

        printf(UL_FMT_STR" - %s+%u (%s)\n",
                    reg_pc,
                    dlinfo.dli_sname,
                    (unsigned)((void *)reg_pc - dlinfo.dli_saddr),
                    dlinfo.dli_fname);

        if(dlinfo.dli_sname && !strcmp(dlinfo.dli_sname, "main"))
            break;

        reg_pc = reg_bp[1];
        reg_bp = (void *)(*reg_bp);    
    }

printf("\n\n\n");

}

static void print_stack_contents(void *sp)
{
    printf("stack contents:\n");
    print_mem(sp, 512);
printf("\n\n\n");
}

static void print_detailed_sig_info(int sig_no, siginfo_t *pt_siginfo, void *p_ucontext)
{
    ucontext_t *pt_ucontext = p_ucontext;
    unsigned long *reg_bp = (void *)(pt_ucontext->uc_mcontext.gregs[REG_BP_NO]);
    unsigned long  reg_pc = pt_ucontext->uc_mcontext.gregs[REG_PC_NO];
    void *reg_sp = (void *)(pt_ucontext->uc_mcontext.gregs[REG_SP_NO]);

    printf("pc=" UL_FMT_STR " access invalid mem addr=" UL_FMT_STR "\n\n\n"
           ,reg_pc
           ,(unsigned long)(pt_siginfo->si_addr));
    
    print_gregs(pt_ucontext);
    print_call_links(reg_bp, reg_pc);
    print_stack_contents(reg_sp);

}
#else
static void print_detailed_sig_info(int sig_no, siginfo_t *pt_siginfo, void *p_ucontext)
{

}
#endif

static void  exceptionmy_handler(int sig_no, siginfo_t *pt_siginfo, void *p_ucontext)
{
    char prog_path[1024] = {0};
    const char *sig_name="unknown";

    get_self_path(prog_path, sizeof(prog_path));
    switch (sig_no)
    {
        case SIGSEGV:
            sig_name = "SIGSEGV";
            break;

        case SIGILL:
            sig_name = "SIGILL";
            break;
            
        case SIGABRT:
            sig_name = "SIGABRT";
            break;
    }


    printf("\n\n\n"
        "***********************\n"
        "*******EXCEPTION*******\n"
        "***********************\n"
        "[program]:%s\n\n"
        , prog_path);  
    
    printf("received signal %d (%s).\n"
        "The process will exit.\n\n"
        , sig_no
        , sig_name);


    print_detailed_sig_info(sig_no, pt_siginfo, p_ucontext);
    
    exit(1);

}

void __attribute__((constructor, used)) trace_exception_init()
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = exceptionmy_handler;
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &action, NULL);
    sigaction(SIGILL, &action, NULL);
    sigaction(SIGABRT, &action, NULL);
}





