#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/acpi.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
// use BIF and BST
// BST info on page 577 of ACPI spec sheet

#define delay 1000
#define delayCritical 30000

struct task_struct *thread, *reporter;
int tid = 0, tid2 = 1;
int capacity, powerUnit;
bool flag = false, signal = false;
struct semaphore lock;

int doit(void* id) {
	if(!down_interruptible(&lock)) {
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
					if(signal) signal = false;
					if(!down_interruptible(&lock)) {
						printk(KERN_INFO "Battery charging; Charge=%d; info#=%d\n", charge, charge_discharge);
					}
				} else {
					if(charge <= 5000 && charge >= 3500) {
						if(signal) signal = false;
						if(!down_interruptible(&lock)) {
							printk(KERN_INFO "Battery discharging; RUNNING LOW; Remaning=%d; info#=%d\n", charge, charge_discharge);
						}
					}
					else if(charge <= 3500) {
						up(&lock);
						signal = true;
						printk(KERN_INFO "Battery discharging; VEREY LOW; Remaning=%d; info#=%d\n", charge, charge_discharge);
					} else {
						printk(KERN_INFO "Battery discharging; Remaning=%d; info#=%d\n", charge, charge_discharge);
					}
				}
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
	}
	return 0;
}

int criticalMsg(void* data) {
	while(1) {
		if(signal) {
			if(!down_interruptible(&lock)) {
				printk(KERN_EMERG "BATTERY LEVEL CRITICALLY LOW");
			}
		} else {
			up(&lock);
		}
		if(kthread_should_stop()) {
			flag = true;
		}
		msleep(delayCritical);
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

	sema_init(&lock, 1);

	thread = kthread_run(doit, (void*)&tid, "tid1");
	msleep(500);
	reporter = kthread_run(criticalMsg, (void*)&tid2, "tid2");

	/*if(thread) {
		wake_up_process(thread);
	} else {
		printk(KERN_EMERG "ERROR\n");
	}*/

	msleep(500);

	kfree(result);
	
	return 0;
}

void cleanup_module() {
	if(thread) {
		if(flag) {
			kthread_stop(thread);
			kthread_stop(reporter);
		}
		kfree(thread);
		kfree(reporter);

	}
	//kfree(thread);
	printk(KERN_INFO "MODULE REMOVED\n");
}