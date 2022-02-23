#include "gpio_misc_driver.h"

#define ALIVE_BASE	160

/* prototypes */
/* misc driver functions */
static int misc_gpio_open(struct inode *inode, struct file *file);
static int misc_gpio_close(struct inode *inode, struct file *file);
static ssize_t misc_gpio_read(struct file *file, char __user *buf, size_t len,
loff_t *off);
static ssize_t misc_gpio_write(struct file *file, const char *buf, size_t len,
loff_t *off);
static long misc_gpio_ioctl(struct file *file, unsigned int cmd,
unsigned long arg);

/* platform driver functions */
static int platform_probe(struct platform_device *pdev);
static int platform_remove(struct platform_device *pdev);

/* file operation structure */
static const struct file_operations misc_fops = {
	.write          = misc_gpio_write,
	.read           = misc_gpio_read,
	.open           = misc_gpio_open,
	.unlocked_ioctl = misc_gpio_ioctl,
	.release        = misc_gpio_close,
};

/* misc driver fuction implementations */
static int misc_gpio_open(struct inode *inode, struct file *file)
{	pr_info("%s\n", __func__);	return 0; }
static int misc_gpio_close(struct inode *inodep, struct file *file)
{	pr_info("%s\n", __func__);	return 0; }
static ssize_t misc_gpio_read(struct file *filp, char __user *buf,
			   size_t count, loff_t *f_pos)
{	pr_info("%s\n", __func__);	return 0; }
static ssize_t misc_gpio_write(struct file *file, const char __user *buf,
			   size_t len, loff_t *ppos)
{	pr_info("%s\n", __func__);	return len; }

static long     misc_gpio_ioctl(struct file *file, unsigned int cmd,
			   unsigned long arg)
{	pr_info("%s\n", __func__);	return 0; }

/* interrupt functions */
enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
	pr_info("Callback Function: %d\n", timer->is_rel);
	hrtimer_forward_now(timer, ktime_set(10, 0));
	return HRTIMER_RESTART;
}

static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
	pr_info("Interrupt [%d] in top half\n", irq);
	return IRQ_WAKE_THREAD;
}

static irqreturn_t gpio_interrupt_thread_fn(int irq, void *dev_id)
{
	pr_info("Interrupt [%d] in bottom half\n", irq);
	return IRQ_HANDLED;
}

/* platform data format */
struct driver_data {
	struct hrtimer htimer;
	ktime_t ktime;

	const char *name;
	u32 addr;

	int id;
	struct device *dev;
	struct miscdevice mdev;

	struct nx_alive_gpio_regs *palive_gpio;
};

/* platform_probe */
static int platform_probe(struct platform_device *pdev)
{
	struct driver_data *plat_data;

	struct miscdevice misc_gpio_driver = {
		.minor = MISC_DYNAMIC_MINOR,
		.fops = &misc_fops,
	};

	const char *name;
	u32 addr;
	struct nx_alive_gpio_regs *palive;
	struct device *dev = &pdev->dev;
	ktime_t ktime;

	int err;
	int id;
	int irq;

	if (!dev->of_node) {
		dev_err(dev, "device tree node error\n");
		return -ENODEV;
	}

	plat_data = devm_kzalloc(dev, sizeof(*plat_data), GFP_KERNEL);

	err = of_property_read_string(pdev->dev.of_node, "misc-name", &name);
	if (err) {
		pr_err("ERROR : name error!\n");
		return err;
	}
	pr_info("DTS Node name    : %s\n", pdev->name);
	pr_info("Misc Device name : %s\n", name);

	err = of_property_read_u32(pdev->dev.of_node, "misc-addr", &addr);
	if (err) {
		pr_err("ERROR : address error!\n");
		return err;
	}
	pr_info("Device Base addr : %08X\n", addr);

	err = of_property_read_u32(pdev->dev.of_node, "misc-id", &id);
	if (err) {
		pr_err("ERROR : id error!\n");
		return err;
	}
	pr_info("Misc Device pin  : %d\n", id);

	plat_data->id = id;
	plat_data->name = name;
	plat_data->dev = dev;
	plat_data->ktime = ktime;
	palive = (struct nx_alive_gpio_regs *)(addr);
	plat_data->palive_gpio = palive;

	alive_gpio_direction_input(plat_data->palive_gpio, plat_data->id);
	mdelay(10);

	irq = gpio_to_irq(ALIVE_BASE + plat_data->id);
	pr_info("IRQ number   : %d\n", irq);

	if (request_threaded_irq(irq,
	(void *)gpio_irq_handler,
	gpio_interrupt_thread_fn,
	IRQF_TRIGGER_RISING |
	IRQF_TRIGGER_FALLING,
	plat_data->name, NULL)) {
		pr_err("my_device: cannot register IRQ %d\n", irq);
	}

	plat_data->ktime = ktime_set(0, 0);
	mdelay(10);
	hrtimer_init(&plat_data->htimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	plat_data->htimer.is_rel = plat_data->id;

	plat_data->htimer.function = &timer_callback;
	hrtimer_start(&plat_data->htimer, plat_data->ktime, HRTIMER_MODE_REL);

	misc_gpio_driver.name = plat_data->name;
	plat_data->mdev = misc_gpio_driver;
	err = misc_register(&plat_data->mdev);

	if (err) {
		pr_err("misc_register failed\n");
		return err;
	}

	platform_set_drvdata(pdev, plat_data);

	return 0;
};

/* platform_remove */
static int platform_remove(struct platform_device *pdev)
{
	int irq;
	struct driver_data *plat_data;

	plat_data = platform_get_drvdata(pdev);
	hrtimer_cancel(&plat_data->htimer);
	irq = gpio_to_irq(ALIVE_BASE + plat_data->id);
	free_irq(irq, NULL);
	misc_deregister(&plat_data->mdev);

	pr_info("%s\n", __func__);
	return 0;
}

static const struct of_device_id misc_gpio_match[] = {
	{.compatible = "nexell,misc_gpio"},
	{},
};
MODULE_DEVICE_TABLE(of, misc_gpio_match);

static struct platform_driver platform_gpio_driver = {
	.probe = platform_probe,
	.remove = platform_remove,
	.driver = {
		.name = "misc_gpio_driver",
		.of_match_table = misc_gpio_match,
	},
};

/* misc_gpio_driver_init */
static int __init misc_gpio_driver_init(void)
{
	int error;

	error = platform_driver_register(&platform_gpio_driver);
	if (error) {
		pr_err("plat_register failed\n");
		return error;
	}
	return 0;
}

/* misc_gpio_driver_exit */
static void __exit misc_gpio_driver_exit(void)
{
	platform_driver_unregister(&platform_gpio_driver);
	pr_info("%s\n", __func__);
}

/* driver helper fuctions */
u32 misc_getbit(u32 base, unsigned pin)
{
	void *pvaddr;
	unsigned int value;
	unsigned int mask;

	pvaddr = ioremap((u32)base, 32);
	value = ioread32(pvaddr);

	mask = 1UL << pin;
	value = (value & mask) >> pin;

	return value;
}

/* driver helper fuctions */
void misc_setbit(u32 base, unsigned pin)
{
	void *pvaddr;
	unsigned int value;
	unsigned int mask;

	pvaddr = ioremap((u32)base, 32);
	value = ioread32(pvaddr);

	mask = 1UL << pin;
	value = (value | mask);

	iowrite32(value, pvaddr);
}

/* driver helper fuctions */
void misc_clrbit(u32 base, unsigned pin)
{
	void *pvaddr;
	unsigned int value;
	unsigned int mask;

	pvaddr = ioremap((u32)base, 32);
	value = ioread32(pvaddr);

	mask = 1UL << pin;
	value = value & ~mask;

	iowrite32(value, pvaddr);
}

u32 alive_gpio_get_value(struct nx_alive_gpio_regs const *base, unsigned pin)
{
	u32 value;

	value = misc_getbit((u32)&(base->pad), pin);
	return value;
}

void alive_gpio_direction_output(struct nx_alive_gpio_regs const *base,
unsigned pin, int val)
{
	if (val)
		misc_setbit((u32)(&base->data), pin);
	else
		misc_setbit((u32)(&base->pad_reset), pin);

	misc_setbit((u32)(&base->outputenb), pin);
}

void alive_gpio_direction_input(struct nx_alive_gpio_regs const *base,
unsigned pin)
{
	misc_setbit((u32)(&base->outputenb_reset), pin);
}

void alive_gpio_set_value(struct nx_alive_gpio_regs const *base,
unsigned pin, int val)
{
	if (val)
		misc_setbit((u32)(&base->data), pin);
	else
		misc_clrbit((u32)(&base->pad_reset), pin);
}

module_init(misc_gpio_driver_init);
module_exit(misc_gpio_driver_exit);

MODULE_AUTHOR("Hyunwoo, Kim <teddy0530@nexell.co.kr>");
MODULE_DESCRIPTION("Nexell Gpio Misc driver");
MODULE_LICENSE("GPL v2");
