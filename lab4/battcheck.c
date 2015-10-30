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

#define delay 1000

struct task_struct *thread;
int tid = 0;
int capacity, powerUnit;
bool flag = false;

int doit(void* id) {
	int charge_discharge, charge;
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

			if(charge_discharge == 2) {
				printk(KERN_INFO "Battery charging; Percent Remaning=%d\n", charge);
			} else {
				printk(KERN_INFO "Battery discharging; Percent Remaning=%d\n", charge);
			}
		//	kernel_fpu_begin();
		//	y = (int)(charge/1.0);
		//	kernel_fpu_end();
			//printk(KERN_INFO "discharging=%d charge=%d remaining=%d\n", charge_discharge, charge, y);
			
			kfree(result);
		}
		else {
			printk(KERN_EMERG "ERROR\n");
		}
		if(kthread_should_stop()) {
			flag = true;
		}
		msleep(delay);
	}
	return 0;
}

int init_module(void) {

	acpi_status info;
	acpi_handle handle;
	union acpi_object* result;
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

	info = acpi_get_handle(NULL, (acpi_string)"\\_SB_.PCI0.BAT0", &handle);
	info = acpi_evaluate_object(handle, "_BIF", NULL, &buffer);
	result = buffer.pointer;
	if(result) {
		powerUnit = result->package.elements[0].integer.value;
		capacity = result->package.elements[1].integer.value;

		printk(KERN_INFO "power unit=%d capacity=%d\n", powerUnit, capacity);

	} else {
		printk(KERN_EMERG "ERROR GETTING BATTERY INFORMATION\n");
	}

	thread = kthread_create(doit, (void*)&tid, "tid");
//	t1 = kthread_create(incrementer, (void*)&id1, "inc1");

	if(thread) {
		wake_up_process(thread);
	} else {
		printk(KERN_EMERG "ERROR\n");
	}

	msleep(500);

	//doit();
	return 0;
}

void cleanup_module() {
	if(flag) {
		kthread_stop(thread);
	}

	printk(KERN_INFO "MODULE REMOVED\n");
}