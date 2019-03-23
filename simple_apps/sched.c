/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */



/* 
 * 本程序实现了用户态的任务调度。
 * 适用于X86及X86_64架构。
 * 编译运行方法:
 * [root@localhost ~]# gcc sched.c 
 * [root@localhost ~]# ./a.out
 */

#if defined(__i386) || defined( __x86_64)
#include <stdio.h>
#include <string.h>

#define    MAX_TASK_NUM       (10)
#define    TASK_STACK_SIZE    (4096)

#define    DBG_PRINT(fmt, args...) \
    do \
    { \
        printf("DBG:%s(%d)-%s:\n"fmt"\n", __FILE__,__LINE__,__FUNCTION__,##args); \
    } while (0)

typedef  void * (*task_entry_ptr) (void *);
typedef struct
{
    char               name[64];
    unsigned long      state;
    task_entry_ptr     entry;
    unsigned long      ret;
    unsigned long      sp;
    unsigned long      pc;
    unsigned long      stack[TASK_STACK_SIZE/sizeof(unsigned long)];
} __attribute__((packed)) t_task;

#define    STATE_INVALID    (0)
#define    STATE_SLEEPING   (1)
#define    STATE_RUNNING    (2)
#define    STATE_FINISHED   (3)

t_task g_at_tasks[MAX_TASK_NUM];
int    g_task_cnt = 0;
#define  task_ptr2idx(ptr)  ((t_task *)(ptr)-g_at_tasks)

unsigned long process_main_thread_sp;
t_task        *pt_cur_running_task;
unsigned long prev_task_sp, prev_task_pc;
unsigned long next_task_sp, next_task_pc;

#if defined( __i386)
#define save_context(mem_var_to_save_sp) \
    do \
    { \
        asm volatile("pushfl\n\t" /* save    flags */ \
                     "pushl %%eax\n\t" \
                     "pushl %%edi\n\t" \
                     "pushl %%esi\n\t" \
                     "pushl %%edx\n\t" \
                     "pushl %%ecx\n\t" \
                     "pushl %%ebx\n\t" \
                     "pushl %%ebp\n\t" \
                     "movl %%esp,%[mem_var_to_store_sp]\n\t" \
            : [mem_var_to_store_sp] "=m" (mem_var_to_save_sp)\
                    ); \
    } while (0)


#define restore_context(mem_var_saved_sp) \
    do \
    { \
        asm volatile("movl %[mem_var_contain_sp],%%esp\n\t" \
                     "popl %%ebp\n\t" \
                     "popl %%ebx\n\t" \
                     "popl %%ecx\n\t" \
                     "popl %%edx\n\t" \
                     "popl %%esi\n\t" \
                     "popl %%edi\n\t" \
                     "popl %%eax\n\t" \
                     "popfl\n\t"     \
            :: [mem_var_contain_sp] "m" (mem_var_saved_sp)\
                    ); \
    } while (0)

#elif defined( __x86_64)
#define save_context(mem_var_to_save_sp) \
    do \
    { \
        asm volatile("pushfq\n\t" /* save    flags */ \
                     "pushq %%rax\n\t" \
                     "pushq %%rdi\n\t" \
                     "pushq %%rsi\n\t" \
                     "pushq %%rdx\n\t" \
                     "pushq %%rcx\n\t" \
                     "pushq %%rbx\n\t" \
                     "pushq %%rbp\n\t" \
                     "movq %%rsp,%[mem_var_to_store_sp]\n\t" \
            : [mem_var_to_store_sp] "=m" (mem_var_to_save_sp)\
                ); \
    } while (0)


#define restore_context(mem_var_saved_sp) \
    do \
    { \
        asm volatile("movq %[mem_var_contain_sp],%%rsp\n\t" \
                     "popq %%rbp\n\t" \
                     "popq %%rbx\n\t" \
                     "popq %%rcx\n\t" \
                     "popq %%rdx\n\t" \
                     "popq %%rsi\n\t" \
                     "popq %%rdi\n\t" \
                     "popq %%rax\n\t" \
                     "popfq\n\t"     \
        :: [mem_var_contain_sp] "m" (mem_var_saved_sp)\
                    ); \
    } while (0)
#endif

int i;
int task_scheduler()
{
    unsigned long ret;
#if defined( __i386)
    asm volatile("movl %%eax,%[task_ret]\n\t"
    : [task_ret] "=m" (ret)
    );
#elif defined( __x86_64)
    asm volatile("movq %%rax,%[task_ret]\n\t"
    : [task_ret] "=m" (ret)
    );
#endif

    if (pt_cur_running_task)
    {
        pt_cur_running_task->state=STATE_FINISHED;
        pt_cur_running_task->ret=ret;
        DBG_PRINT("task %s exit with code %lu", pt_cur_running_task->name, pt_cur_running_task->ret);
    }
    
    for (i=0;i<MAX_TASK_NUM;i++)
    {
        pt_cur_running_task = &(g_at_tasks[i]);
        if (pt_cur_running_task->state==STATE_SLEEPING)
        {
            pt_cur_running_task->state=STATE_RUNNING;
            next_task_sp = pt_cur_running_task->sp;
            next_task_pc = pt_cur_running_task->pc;


            /* 准备运行下一个可运行任务 */
            restore_context(next_task_sp);
            asm volatile("jmp  *%[next_pc]\n\t"
                     :: [next_pc] "m" (next_task_pc)
            );
        }
    }


    DBG_PRINT("==no task to run. so we exit");
    restore_context(process_main_thread_sp);
    return 0;
}


#define push_task_stack(sp, data) \
do \
{ \
    sp--; \
    *sp = data; \
} while (0)


int create_task(const char *name, void *task_entry, void *para)
{
    unsigned long *sp;
    t_task *pt_task = &(g_at_tasks[g_task_cnt]);
    strncpy(pt_task->name, name, sizeof(pt_task->name));
    pt_task->entry = task_entry;
    pt_task->sp = (unsigned long)((void *)(pt_task + 1));
    pt_task->state = STATE_SLEEPING;
    sp = (void *)(pt_task->sp);
#if defined( __i386)
    push_task_stack(sp, (unsigned long)para);
#endif
    push_task_stack(sp, (unsigned long)(void *)&task_scheduler);
    pt_task->pc = (unsigned long)task_entry;
    push_task_stack(sp, 0);
    push_task_stack(sp, 0);
#if defined( __i386)
    push_task_stack(sp, 0);
#elif defined( __x86_64)
    push_task_stack(sp, (unsigned long)para);
#endif
    push_task_stack(sp, 0);
    push_task_stack(sp, 0);
    push_task_stack(sp, 0);
    push_task_stack(sp, 0);
    push_task_stack(sp, pt_task->sp); /* push bp at last */
    pt_task->sp = (unsigned long)(void *)sp;

    g_task_cnt++;
    return 0;
}


#if defined( __i386)
#define switch_context(prev, next) \
do \
{ \
    asm volatile("pushfl\n\t"     \
                 "pushl %%eax\n\t" \
                 "pushl %%edi\n\t" \
                 "pushl %%esi\n\t" \
                 "pushl %%edx\n\t" \
                 "pushl %%ecx\n\t" \
                 "pushl %%ebx\n\t" \
                 "pushl %%ebp\n\t" \
                 "movl  %%esp,%[prev_sp]\n\t"  \
                 "movl  $1f,%[prev_pc]\n\t"  \
                 "movl  %[next_sp],%%esp\n\t" \
                 "popl  %%ebp\n\t" /* restore BP   */ \
                 "popl  %%ebx\n\t" \
                 "popl  %%ecx\n\t" \
                 "popl  %%edx\n\t" \
                 "popl  %%esi\n\t" \
                 "popl  %%edi\n\t" \
                 "popl  %%eax\n\t" \
                 "popfl\n\t"     \
                 "jmp  *%[next_pc]\n\t" \
                 "1:\t"\
                 "nop\n\t"     \
          : [prev_sp] "=m" (prev->sp),\
            [prev_pc] "=m" (prev->pc)\
          : [next_sp] "m" (next_task_sp), [next_pc] "m" (next_task_pc) \
          ); \
}while (0)


#elif defined( __x86_64)

#define switch_context(prev, next) \
do \
{ \
    asm volatile("pushfq\n\t"     \
                 "pushq %%rax\n\t" \
                 "pushq %%rdi\n\t" \
                 "pushq %%rsi\n\t" \
                 "pushq %%rdx\n\t" \
                 "pushq %%rcx\n\t" \
                 "pushq %%rbx\n\t" \
                 "pushq %%rbp\n\t" \
                 "movq  %%rsp,%[prev_sp]\n\t"  \
                 "movq  $1f,%[prev_pc]\n\t"  \
                 "movq  %[next_sp],%%rsp\n\t" \
                 "popq  %%rbp\n\t" /* restore BP   */ \
                 "popq  %%rbx\n\t" \
                 "popq  %%rcx\n\t" \
                 "popq  %%rdx\n\t" \
                 "popq  %%rsi\n\t" \
                 "popq  %%rdi\n\t" \
                 "popq  %%rax\n\t" \
                 "popfq\n\t"     \
                 "jmp  *%[next_pc]\n\t" \
                 "1:\t"\
                 "nop\n\t"     \
          : [prev_sp] "=m" (prev->sp),\
            [prev_pc] "=m" (prev->pc)\
          : [next_sp] "m" (next_task_sp), [next_pc] "m" (next_task_pc) \
          ); \
}while (0)
#endif
void schedule()
{
    t_task *prev=pt_cur_running_task, *next=NULL;
    int  cur_task_idx=task_ptr2idx(prev);
    for (i=cur_task_idx+1; i!=cur_task_idx; i=(i+1)%MAX_TASK_NUM)
    {
        next = &(g_at_tasks[i]);
        if (next->state==STATE_SLEEPING)
        {
            pt_cur_running_task->state=STATE_SLEEPING;
            prev_task_sp = pt_cur_running_task->sp;
            prev_task_pc = pt_cur_running_task->pc;

            pt_cur_running_task = next;
            pt_cur_running_task->state=STATE_RUNNING;
            next_task_sp = pt_cur_running_task->sp;
            next_task_pc = pt_cur_running_task->pc;
            break;
        }
    }

    if (i==cur_task_idx) return;

    switch_context(prev, next);
}


int start_sched()
{
    /* 启动调度,首个运行的任务是 father_of_all_task - g_at_tasks[0] */
    save_context(process_main_thread_sp);
    task_scheduler();
    return 0;
}

/* 以上是调度功能的实现，下面是使用示例 */
int task_1_para=1001;
void * usr_task1(void *para)
{
    int i;
DBG_PRINT("==enter, para=%d", *(int *)para);
    for (i=10; i<13; i++)
    {
        DBG_PRINT("==%d", i);
        schedule();
    }
DBG_PRINT("==exit");
    return (void *)100UL;
}

int task_2_para=2001;
void * usr_task2(void *para)
{
    int i;
DBG_PRINT("==enter, para=%d", *(int *)para);
    for (i=20; i<23; i++)
    {
        DBG_PRINT("==%d", i);
        schedule();
    }
DBG_PRINT("==exit");
    return (void *)200UL;
}

int task_3_para=3001;
void * usr_task3(void *para)
{
    int i;
DBG_PRINT("==enter, para=%d", *(int *)para);
    for (i=30; i<33; i++)
    {
        DBG_PRINT("==%d", i);
        schedule();
    }
DBG_PRINT("==exit");
    return (void *)300UL;
}

int root_task_para=1234;
void * father_of_all_task(void *para)
{
DBG_PRINT("==enter, para=%d", *(int *)para);
    create_task("usr_task1", usr_task1, &task_1_para);
    create_task("usr_task2", usr_task2, &task_2_para);
    create_task("usr_task3", usr_task3, &task_3_para);
DBG_PRINT("==exit");
    return (void *)2015UL;
}

int main(int argc, char *argv[])
{
DBG_PRINT("hello");
    create_task("father_of_all_task", father_of_all_task, &root_task_para);
    start_sched();
DBG_PRINT("good bye");
    return 0;
}
#else
int main(int argc, char *argv[])
{
    return 0;
}
#endif
