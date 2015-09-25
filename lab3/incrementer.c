#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/mutex.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define length 1000000

int arr[length];
int stat[length];
int idx = 0;
int sum = 0;
int delay;
int id1 = 1, id2 = 2;
int cs1 = 0, cs2 = 0;
struct task_struct *t1, *t2;
struct mutex lock;


int incrementer(void *ptr) {
	printk(KERN_EMERG "Consumer TID %d entering incrementer() at time %ld\n", *(int*)ptr, jiffies);

	while(1) {
		mutex_lock(&lock);
		if(idx >= length) {
			printk(KERN_EMERG "Thread TID %d leaving incrementer() leaving at time %ld\n", *(int*)ptr, jiffies);
			mutex_unlock(&lock);
			break;
		}
		if(*(int*)ptr == id1) {
			cs1++;
		} else {
			cs2++;
		}
		arr[idx++] = 1;
		delay = 0;
		mutex_unlock(&lock);
		schedule();
	}

	return 0;
}

int init_module(void) {
	int k = 0;
	while(k < length){
		arr[k] = 0;
		stat[k] = 0;
		k++;
	}

	printk(KERN_EMERG "\n\n\n");

	mutex_init(&lock);

	t1 = kthread_create(incrementer, (void*)&id1, "inc1");
	t2 = kthread_create(incrementer, (void*)&id2, "inc2");
	if(t1 && t2) {
		wake_up_process(t1);
		wake_up_process(t2);
	} else {
		printk(KERN_EMERG "Error\n");
	}

	msleep(500);

	return 0;
}

void cleanup_module(void) {
	int a = 0;
	while(a < length) {
		stat[arr[a++]]++;
	}
	sum = cs1 + cs2;
	printk(KERN_EMERG "Value of stat[1] = %d\n", stat[1]);
	printk(KERN_EMERG "Value of stat[0] = %d\n", stat[0]);
	printk(KERN_EMERG "cs1 = %d\ncs2 = %d\n", cs1, cs2);
	printk(KERN_EMERG "cs1 + cs2 = %d\n", sum);
	printk(KERN_EMERG "Leaving module...\n");
}
