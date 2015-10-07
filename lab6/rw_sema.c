#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/semaphore.h>

MODULE_LICENSE("GPL");

int arr[100];
struct rw_semaphore rw_lock;

int init_module(void) {

}