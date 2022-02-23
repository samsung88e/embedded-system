#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>
#include <linux/time.h>
#include <linux/timer.h>

static struct hrtimer timer;
ktime_t kt;
struct timespec64 oldtc;

static enum hrtimer_restart hrtimer_hander(struct hrtimer *timer)
{
	struct timespec64 tc;
    printk("I am in hrtimer hander : %lu... \r\n",jiffies);

	ktime_get_real_ts64(&tc); //Get the new current system time
	
	printk("interval: %ld - %ld = %ld us\r\n",tc.tv_nsec/1000,oldtc.tv_nsec/1000,tc.tv_nsec/1000 - oldtc.tv_nsec/1000);
	oldtc = tc;
    hrtimer_forward(timer,timer->base->get_time(),kt);
    return HRTIMER_RESTART;
}

static int __init test_init(void)
{
    printk("---------test start-----------\r\n");
    
	ktime_get_real_ts64(&oldtc);  //Get the current system time
    kt = ktime_set(1,0);// 1ns = 1 초
    hrtimer_init(&timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
    hrtimer_start(&timer,kt,HRTIMER_MODE_REL);
    timer.function = hrtimer_hander;
    return 0;
}

static void __exit test_exit(void)
{
    hrtimer_cancel(&timer);
    printk("------------test over---------------\r\n");
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");