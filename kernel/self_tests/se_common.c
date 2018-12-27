/* SPDX-License-Identifier: BSD-2-Clause */

#include <tilck/common/basic_defs.h>
#include <tilck/common/string_util.h>
#include <tilck/common/utils.h>

#include <tilck/kernel/irq.h>
#include <tilck/kernel/kmalloc.h>
#include <tilck/kernel/paging.h>
#include <tilck/kernel/debug_utils.h>

#include <tilck/kernel/hal.h>
#include <tilck/kernel/tasklet.h>
#include <tilck/kernel/sync.h>
#include <tilck/kernel/fault_resumable.h>
#include <tilck/kernel/timer.h>
#include <tilck/kernel/self_tests/self_tests.h>
#include <tilck/kernel/cmdline.h>
#include <tilck/kernel/gcov.h>


void regular_self_test_end(void)
{
   if (dump_coverage)
      gcov_dump_coverage();

   printk("DEBUG QEMU turn off machine\n");
   debug_qemu_turn_off_machine();
}

void simple_test_kthread(void *arg)
{
   u32 i;
#if !defined(NDEBUG) && !defined(RELEASE)
   uptr esp;
   uptr saved_esp = get_curr_stack_ptr();
#endif

   printk("[kthread] This is a kernel thread, arg = %p\n", arg);

   for (i = 0; i < 256*MB; i++) {

#if !defined(NDEBUG) && !defined(RELEASE)

      /*
       * This VERY IMPORTANT check ensures us that in NO WAY functions like
       * save_current_task_state() and kernel_context_switch() changed value
       * of the stack pointer. Unfortunately, we cannot reliably do this check
       * in RELEASE (= optimized) builds because the compiler plays with the
       * stack pointer and 'esp' and 'saved_esp' differ by a constant value.
       */
      esp = get_curr_stack_ptr();

      if (esp != saved_esp)
         panic("esp: %p != saved_esp: %p [curr-saved: %d], i = %u",
               esp, saved_esp, esp - saved_esp, i);

#endif

      if (!(i % (64*MB))) {
         printk("[kthread] i = %i\n", i/MB);
      }
   }

   printk("[kthread] completed\n");

   if ((uptr)arg == 1) {
      regular_self_test_end();
   }
}

void selftest_kthread_med(void)
{
   if (!kthread_create(simple_test_kthread, (void *)1))
      panic("Unable to create the simple test kthread");
}

void selftest_kernel_sleep_short()
{
   const u64 wait_ticks = TIMER_HZ;
   u64 before = get_ticks();

   kernel_sleep(wait_ticks);

   u64 after = get_ticks();
   u64 elapsed = after - before;

   printk("[sleeping_kthread] elapsed ticks: %llu (expected: %llu)\n",
          elapsed, wait_ticks);

   VERIFY((elapsed - wait_ticks) <= 2);

   regular_self_test_end();
}

void selftest_join_med()
{
   printk("[selftest join] create the simple thread\n");

   task_info *ti = kthread_create(simple_test_kthread, (void *)0xAA0011FF);

   printk("[selftest join] join()\n");

   join_kernel_thread(ti->tid);

   printk("[selftest join] kernel thread exited\n");
   regular_self_test_end();
}

/*
 * Special selftest that cannot be run by the system test runner.
 * It has to be run manually by passing to the kernel -s panic_manual.
 */
void selftest_panic_manual(void)
{
   printk("[panic selftest] In a while, I'll panic\n");

   for (int i = 0; i < 500*1000*1000; i++) {
      asmVolatile("nop");
   }

   panic("test panic");
}
