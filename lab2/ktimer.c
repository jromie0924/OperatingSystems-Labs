#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/kthread.h>

MODULE_LICENSE("GPL");

//static struct timer_list my_timer;
static int count;
struct task_struct *ts;
bool flag = false;
int id = 101; /* global */

int function(void *data) {
  int delay = *(int*)data;
  while(count < 10) {
  //while(true) {
    printk(KERN_EMERG "Ding! % d count # %d\n", delay, count++);
   // printk(KERN_EMERG "Jiffies = %lu\n", jiffies);
   // printk(KERN_INFO "jiffies=%lu", jiffies);
    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(HZ);
    if(kthread_should_stop()) {
      flag = true;
      return 0;
    }
  }
  return 0;
}

int init_module( void ) {
   ts = kthread_run(function, (void*)&id, "spawn");
   return 0;
}

void cleanup_module( void ) {

  if(flag) {
    kthread_stop(ts);
  }

  printk("Timer module uninstalling\n");

  return;
}
