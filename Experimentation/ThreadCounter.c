/*
Jackson Romie

++++FOR FUN+++

This kernel module increments two elements in an array of two integers from 0 to 9000000.
There are two threads - ts0 and ts1 and each is assigned to its corrosponding element in the array.

*/

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#define sum 9000000

struct task_struct *ts0, *ts1;
struct mutex lock;
int timer, start;
int timerfinal[2];
int id0 = 0, id1 = 1;
int tracker0 = 0, tracker1 = 0;
int countVars[2];


int counterup(void* id) {
   printk(KERN_INFO "Thread %d entering counterup function.\n", *((int*)id));
   while(1) {
      mutex_lock(&lock);
      if(countVars[0] + countVars[1] == 2*sum) {
         timerfinal[*((int*)id)] = jiffies;
         printk(KERN_INFO "Thread %d leaving counterup function\n", *((int*)id));
         mutex_unlock(&lock);
         break;
      } else {
         if(countVars[*((int*)id)] < sum) {
            countVars[*((int*)id)]++;
            if(*((int*)id) == 0)
               tracker0++;
            else
               tracker1++;
         }
         mutex_unlock(&lock);
         schedule();
      }
   }
   timer = jiffies;
   return 0;
}

int init_module(void) {
   countVars[0] = 0;
   countVars[1] = 0;
   printk(KERN_INFO "-------------------------------------------\n");
   printk("Counter module installing\n");
   printk(KERN_INFO "countVars = {%d, %d}\n", countVars[0], countVars[1]);
   mutex_init(&lock);
   start = jiffies;
   ts0 = kthread_create(counterup, (void*)&id0, "inc0");
   ts1 = kthread_create(counterup, (void*)&id1, "inc1");

   if(ts0 && ts1) {
      wake_up_process(ts0);
      wake_up_process(ts1);
   } else {
      printk(KERN_EMERG "ERROR\n");
   }

   return 0;
}

void cleanup_module(void) {
   printk(KERN_INFO "countVars = {%d, %d}\n", countVars[0], countVars[1]);
   printk(KERN_INFO "tracker0 = %d\ntracker1 = %d\n", tracker0, tracker1);
   printk(KERN_INFO "Counting took %d jiffies.\n", timer - start);
   printk(KERN_INFO "There are %d jiffies/second.", HZ);
   printk(KERN_INFO "Leaving module\n-------------------------------------------\n+\n+\n+\n");
}