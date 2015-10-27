#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/acpi.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
// use BIF and BST

#define delay 5000

struct task_struct *thread;
int tid = 0;

int doit(void* id) {
	int charge_discharge, charge, y;
	while(1) {
		acpi_status status;
		acpi_handle handle;
		union acpi_object* result;
		struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

		status = acpi_get_handle(NULL, (acpi_string)"\\_SB_.PCI0.BAT0", &handle);
		status = acpi_evaluate_object(handle, "_BST", NULL, &buffer);
		result = buffer.pointer;
		if(result) {
			charge_discharge = result->package.elements[0].integer.value;
			charge = result->package.elements[2].integer.value;
		//	kernel_fpu_begin();
			y = (int)(charge/1.0);
		//	kernel_fpu_end();
			printk(KERN_INFO "Info from thread ID %d: discharging=%d charge=%d remaining=%d\n", *((int*)id), charge_discharge, charge, y);
			kfree(result);
		}
		msleep(delay);
	}
	// queue_delayed_work(queue, &dwork, HZ);
	return 0;
}

int init_module(void) {
//	queue = create_workqueue("queue");
//	INIT_DELAYED_WORK((struct delayed_work*)&dwork, doit);
//	queue_delayed_work(queue, &dwork, HZ);

	thread = kthread_create(doit, (void*)&tid, "tid");
//	t1 = kthread_create(incrementer, (void*)&id1, "inc1");

	if(thread) {
		wake_up_process(thread);
	} else {
		printk(KERN_EMERG "ERROR\n");
	}

	//doit();
	return 0;
}

void cleanup_module() {
	printk(KERN_INFO "MODULE REMOVED\n");
}