#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/spinlock.h>

// 1초마다 로그를 남기는 타이머 예제

#define MODULE_NAME "TIMER"
#define DELAY (1 * HZ)

struct timer_data {
        int value;
        spinlock_t lock;
        struct timer_list timer;
        bool isActive;
};

struct timer_data my_data = {};

void timer_callback(struct timer_list *timer) {
        struct timer_data *data = from_timer(data, timer, timer);

        data->value++;
        printk(KERN_INFO "[%s] value is = %d\n", __func__, data->value);
        spin_lock(&data->lock);
        if (data->isActive)
                mod_timer(timer, jiffies + DELAY);
        spin_unlock(&data->lock);
}

int __init timer_init(void) {
        printk("[%s] creating timer...\n", __func__);

        /* initialization */
        my_data.isActive = true;
        spin_lock_init(&my_data.lock);
        timer_setup(&my_data.timer, timer_callback, 0);

        /* register timer */
        mod_timer(&my_data.timer, jiffies + DELAY);
        return 0;
}

void __exit timer_exit(void) {
        int ret;

        spin_lock(&my_data.lock);
        my_data.isActive = false;
        ret = del_timer(&my_data.timer);
        spin_unlock(&my_data.lock);

        printk("[%s] deleting timer..., ret = %d\n", __func__, ret);
}

module_init(timer_init);
module_exit(timer_exit);

MODULE_LICENSE("GPL");