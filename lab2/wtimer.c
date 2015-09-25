#include <linux/module.h>
#include <linux/workqueue.h>
//#include <linux/slab.h>

MODULE_LICENSE("GPL");

struct workqueue_struct *queue;
struct delayed_work dwork;
int count = 0;

void function(struct work_struct *work) {
	if(count < 10) {
		printk(KERN_EMERG "Ding yo\n");
		count++;
		queue_delayed_work(queue, &dwork, HZ);
	}
	else
		return;
}

int init_module( void ) {
	queue = create_workqueue("queue");
	//INIT_DELAYED_WORK(&dwork, function);
	INIT_DELAYED_WORK((struct delayed_work*)&dwork, function);
	queue_delayed_work(queue, &dwork, HZ);
	return 0;
}

void cleanup_module(void) {
	if(delayed_work_pending(&dwork)) {
		cancel_delayed_work(&dwork);
	}
	flush_workqueue(queue);
	destroy_workqueue(queue);
	printk(KERN_INFO "MODULE REMOVED\n");
}