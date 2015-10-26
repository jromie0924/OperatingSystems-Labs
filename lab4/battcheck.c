#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/acpi.h>

MODULE_LICENSE("GPL");
// use BIF and BST

struct workqueue_struct *queue;
struct delayed_work dwork;

void doit(void) {
	acpi_status status;
	acpi_handle handle;
	union acpi_object* result;
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

	int charge_discharge, charge, y;

	 status = acpi_get_handle(NULL, (acpi_string)"\\_SB_.PCI0.BAT0", &handle);

	 status = acpi_evaluate_object(handle, "_BST", NULL, &buffer);
	 result = buffer.pointer;
	 if(result) {
	 	charge_discharge = result->package.elements[0].integer.value;
	 	charge = result->package.elements[2].integer.value;
	 //	kernel_fpu_begin();
	 	y = (int)(charge/1.0);
	 //	kernel_fpu_end();
	 	printk(KERN_INFO "discharging=%d charge=%d remaining=%d\n", charge_discharge, charge, y);
	 	kfree(result);
	 }
	 queue_delayed_work(queue, &dwork, HZ);
}

int init_module(void) {
	/*acpi_status info;
	acpi_handle ihandle;
	union acpi_object* result;
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

	int capacity, capLow;
	//string model;
	info = acpi_get_handle(NULL, (acpi_string)"\\_SB_.PCI0.BAT0", &ihandle);
	info = acpi_evaluate_object(ihandle, "BIF", NULL, &buffer);
	result = buffer.pointer;
	if(result) {
		capacity = result->package.elements[1].integer.value;
		capLow = result->package.elements[6].integer.value;
	//	model = result->package.elements[9].toString();

		printk(KERN_INFO "Battery Capacity=%d Capacityof Low=%d\n", capacity, capLow);
		kfree(result);
	}
	else {
		printk(KERN_EMERG "ERROR GETTING BATTERY INFO\n\n");
	}*/

	queue = create_workqueue("queue");
	INIT_DELAYED_WORK((struct delayed_work*)&dwork, doit;
	queue_delayed_work(queue, &dwork, HZ);
	return 0;
}

void cleanup_module() {
	if(delayed_work_pending(&dwork)) {
		cancel_delayed_work(&dwork);
	}
	flush_workqueue(queue);
	destroy_workqueue(queue);
	printk(KERN_INFO "MODULE REMOVED\n");
}