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

#if defined(__i386)
#define    UL_FMT_STR    "0x%08x"
#define    REG_BP_NO     REG_EBP
#define    REG_PC_NO     REG_EIP
#elif defined( __x86_64)
#define    UL_FMT_STR    "0x%016x"
#define    REG_BP_NO     REG_RBP
#define    REG_PC_NO     REG_RIP
#else
#error unsupported arch
#endif

void print_arch_name()
{
#if defined(__i386)
    printf("arch is i386\n");
#elif defined( __x86_64)
    printf("arch is x86_64\n");
#else
#error unsupported arch
#endif

}

void print_gregs(ucontext_t *pt_ucontext)
{
    int i;
    
    printf("value of general regs:\n");
    for(i = 0; i < NGREG; i++)
    {
        if (i % 4 == 0) printf("\n");
        printf("reg[%02d]=" UL_FMT_STR "    ", i, pt_ucontext->uc_mcontext.gregs[i]);
    }
    printf("\n");

}

void print_call_links(unsigned long *reg_bp, unsigned long reg_pc)
{
    Dl_info dlinfo;
   
    printf("function call links:\n");
    
    while (reg_bp!=NULL)
    {
       if(!dladdr((void *)reg_pc, &dlinfo))
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



}

static void  my_sig_handler(int sig_no, siginfo_t *pt_siginfo, void *p_ucontext)
{
    ucontext_t *pt_ucontext = p_ucontext;
    unsigned long *reg_bp = (void *)(pt_ucontext->uc_mcontext.gregs[REG_BP_NO]);
    unsigned long  reg_pc = pt_ucontext->uc_mcontext.gregs[REG_PC_NO];

    if (sig_no != SIGSEGV)
    {
        printf("\n\nreceived signal %d\n", sig_no);
        return;

    }

    printf("\n\nreceived signal SIGSEGV. program will exit.\n\n");

    print_arch_name();

    printf("pc=" UL_FMT_STR " access invalid mem addr=" UL_FMT_STR "\n"
           ,reg_pc
           ,(unsigned long)(pt_siginfo->si_addr));
    
    print_gregs(pt_ucontext);
    print_call_links(reg_bp, reg_pc);

    
    exit(1);

}

void trace_exception_init()
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = my_sig_handler;
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &action, NULL);
}
