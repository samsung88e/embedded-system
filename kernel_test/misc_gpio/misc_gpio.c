/**
 * @file misc_gpio.c
 * @author Hyunwoo Kim (teddy0530@coasia.com)
 * @brief misc gpio control
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 * @dts
dwc2otg@c0040000 {
			pinctrl-names = "default";
			pinctrl-0 = <&otg_pwron>;
			gpios = <&gpio_d 21 0>;
			nouse_idcon = <1>;
			status = "okay";
		};

*/
#define PHYS_BASE_GPIOA		(0xC001A000)
#define PHYS_BASE_GPIOB		(0xC001B000)
#define PHYS_BASE_GPIOC		(0xC001C000)
#define PHYS_BASE_GPIOD		(0xC001D000)
#define PHYS_BASE_GPIOE		(0xC001E000)
#define PHYS_BASE_ALIVE		(0xC0010800)

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>     //GPIO
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  //copy_to/from_user()
#include <linux/time.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/types.h>
#include <linux/platform_device.h>



// Control Register has AliveGPIO in/out mode enable, pull-up, and AliveGPIOPADOut.

//Timer Variable
// hrtimer = 
#define TIMEOUT_NSEC   ( 1000000000L )      //1 second in nano seconds
#define TIMEOUT_SEC    ( 4 )                //4 seconds
static struct hrtimer etx_hr_timer;
static unsigned int count = 0;

//Timer Callback function. This will be called when timer expires
enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
    pr_info("Timer Callback function Called [%d]\n",count++);
    hrtimer_forward_now(timer, ktime_set(TIMEOUT_SEC, TIMEOUT_NSEC));
    return HRTIMER_RESTART;
}

#define EN_DEBOUNCE
#ifdef EN_DEBOUNCE

extern unsigned long volatile jiffies;
unsigned long old_jiffie = 0;
#endif

//LED is connected to this GPIO
#define GPIO_162 (162)
#define GPIO_163 (163)


//GPIO_163 value toggle
unsigned int led_toggle = 0; 

//This used for storing the IRQ number for the GPIO
unsigned int GPIO_irqNumber;


//Interrupt handler for GPIO 25. This will be called whenever there is a raising edge detected. 
static irqreturn_t gpio_irq_handler(int irq, void *dev_id) 
{
  
#ifdef EN_DEBOUNCE
   unsigned long diff = jiffies/HZ - old_jiffie/HZ;

    // HZ=1000 = 1msec = 1초 = 1000HZ
   pr_info("HZ = %u, diff = %lu 초, jiffies/HZ 초 = %lu - old_jiffie/HZ 초 = %lu\n", HZ, diff, jiffies/HZ, old_jiffie/HZ);

   if (diff < 20)
   {
    printk(KERN_NOTICE "Interrupt [%d] for device %s was triggered, jiffies=%lu, diff=%lu, direction: %d \n",
          irq, (char *) dev_id, jiffies, diff, gpio_get_value(GPIO_162));

    printk(KERN_NOTICE "Interrupt [%d] for device %s was triggered, jiffies=%lu, diff=%lu, direction: %d \n",
          irq, (char *) dev_id, jiffies, diff, gpio_get_value(GPIO_163));

    return IRQ_HANDLED;
   }
  
  old_jiffie = jiffies;
#endif  
  
  pr_info("Interrupt(IRQ Handler)\n");
  
  /*
  ** If you don't want to call the thread fun, then you can just return
  ** IRQ_HANDLED. If you return IRQ_WAKE_THREAD, then thread fun will be called.
  */
  return IRQ_WAKE_THREAD;
}


// This function is the threaded irq handler
static irqreturn_t gpio_interrupt_thread_fn(int irq, void *dev_id) 
{
  led_toggle = (0x01 ^ led_toggle);                             // toggle the old value
  gpio_set_value(GPIO_162, led_toggle);                      // toggle the GPIO_162
  pr_info("Interrupt(Threaded Handler) : GPIO_162 : %d ", gpio_get_value(GPIO_162));
  
  return IRQ_HANDLED;
}
 
// Driver functions
static int misc_gpio_open(struct inode *inode, struct file *file);
static int misc_gpio_close(struct inode *inode, struct file *file);
static ssize_t misc_gpio_read(struct file *file, char __user *buf, size_t len,loff_t * off);
static ssize_t misc_gpio_write(struct file *file, const char *buf, size_t len, loff_t * off);

static int misc_gpio_open(struct inode *inode, struct file *file)
{   printk("%s\n", __FUNCTION__); return 0; }

static int misc_gpio_close(struct inode *inodep, struct file *file)
{   printk("%s\n", __FUNCTION__);   return 0; }

static ssize_t misc_gpio_read(struct file *filp, char __user *buf,
                    size_t count, loff_t *f_pos)
{   printk("%s\n", __FUNCTION__); return 0; }

static ssize_t misc_gpio_write(struct file *file, const char __user *buf,
               size_t len, loff_t *ppos)
{   printk("%s\n", __FUNCTION__); return len; }

//File operation structure 
static const struct file_operations fops = 
{
    .owner          = THIS_MODULE,
    .write          = misc_gpio_write,
    .read           = misc_gpio_read,
    .open           = misc_gpio_open,
    .release        = misc_gpio_close,
};

struct my_drv_data
{
    struct platform_device *pdev; 
    int my_gpio;
    int my_irq;
};

// // platdata format
// struct nx_gpio_ctrl_platdata {
// 	int type;
// 	int num;
// 	int pad_func;
// 	struct gpio_desc gpio;
// };

// my_gpio_driver 
//     {
//         compatible = "test, my_gpio_driver";
//         pinctrl-names = "default";
//         pinctrl-0 = <&my_pin_gpio>;
//         TEST, my_pin_gpio = <&alive_0 2 0>;
//         status = "okay";
//     }
static int misc_gpio_probe(struct platform_device *pdev)
{
    // struct my_drv_data *pdata = NULL;
    // int ret = 0;

    // if(!g_pdrv)
    // {
    //     enum of_gpio_flags flags;

    //     pdata = devm_kzalloc(&pdev->dev, sizeof(struct my_drv_data), GFP_KERNEL);
    //     if(!pdata)
    //         return -ENOMEM;
    // }

    // pdata->my_gpio = of_get_named_gpio_flags(pdev->dev.of_node, "test, my_pin_gpio", 0, &flags);

    // if(gpio_is_valid(pdata->my_gpio))
    // {
    //     ret = gpio_direction_output(pdata->my_gpio, 1);
    // }
    // else
    //     pdata->my_gpio = -1;

    // pdata->pdev = pdev;
    // g_pdrv = pdata;
    // platform_set_drvdata(pdev, pdata);

    return 0;
}

static int misc_gpio_remove(struct platform_device *pdev)
{
    printk("%s\n", __FUNCTION__);
    return 0;
}

// static int misc_gpio_suspend(struct platform_device *pdev)
// {
//     printk("%s\n", __FUNCTION__);
//     return 0;
// }

// static int misc_gpio_resume(struct platform_device *pdev)
// {
//     printk("%s\n", __FUNCTION__);
//     return 0;
// }


// 방향 set out Enable
// ALIVE GPIO PAD Out Enable Set Register (ALIVEGPIOPADOUTENBSETREG)
// Address : C001_0878h

// 방향 set out
// ALIVE GPIO PAD Out Set Register (ALIVEGPIOPADOUTSETREG)
// Address : C001_0890h

// 방향 get
// ALIVE GPIO PAD Out Enable Read Register (ALIVEGPIOPADOUTENBREADREG)

// get_direction(unsigned int io)
// unsigned int bit = io & 0x1F
// int dir = -1;
// dir = nx_alive_get_output_enable(bit) ? 1 : 0;


// #define ALIVEGPIOPADOUTENB 0xC001087C
// static int gpio_ioctrl(struct udevice *dev, unsigned long request, void *buf)
// {
//   int ret;
//   struct gpio_ctrl_platdata *plat = dev_get_platdata(dev);
//   unsigned int pin = plat -> num;
//   unsigned int mask = (1UL << pin);
//   unsigned int output;

//   // get direction
//   output = readl(ALIVEGPIOPADOUTENB) & mask;

//    if (output)
//      pr_info("[%s] get direction : (%s)\n", __FUNCTION__, "output");
//    else
//      pr_info("[%s] get direction : (%s)\n", __FUNCTION__, "input");

//   return 0;
// }

//Misc device structure
struct miscdevice misc_gpio_device = 
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = "misc_gpio_driver",
    .fops = &fops,
};

static struct of_device_id misc_gpio_match[] = 
{
    {.compatible = "test, my_gpio_driver"},
    {}
};
MODULE_DEVICE_TABLE(of, misc_gpio_match);

static struct platform_driver platform_gpio_driver =
{
    .probe = misc_gpio_probe,
    .remove = misc_gpio_remove,
    // .suspend = misc_gpio_suspend,
    // .resume = misc_gpio_resume,
    .driver = 
    {
        .name = "misc_gpio_driver",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(misc_gpio_match),
    },
};
/*
** Misc Init function
*/
static int __init misc_gpio_init(void)
{
    ktime_t ktime;
    int error;

    error = misc_register(&misc_gpio_device);
    if (error) 
    {
        pr_err("misc driver register failed!!!\n");
        return error;
    }

    error = platform_driver_register(&platform_gpio_driver);

    if (error) 
    {
        pr_err("platform driver register failed!!!\n");
        return error;
    }

    //Output GPIO configuration    gpio_free(GPIO_163);
    //Checking the GPIO is valid or not
    if(gpio_is_valid(GPIO_162) == false)
    {
        gpio_free(GPIO_162);
    }

    //configure the GPIO as output
    gpio_direction_output(GPIO_162, 1);

    //Input GPIO configuratioin
    //Checking the GPIO is valid or not
    if(gpio_is_valid(GPIO_163) == false)
    {
        pr_err("GPIO %d is not valid\n", GPIO_163);
        gpio_free(GPIO_163);
    }

    //Requesting the GPIO
    if(gpio_request(GPIO_163,"GPIO_163_IN") < 0)
    {
        pr_err("ERROR: GPIO %d request\n", GPIO_163);
        gpio_free(GPIO_163);
    }
    
    //configure the GPIO as input
    gpio_direction_input(GPIO_163);

    #ifndef EN_DEBOUNCE
    //Debounce the button with a delay of 200ms
        if(gpio_set_debounce(GPIO_163, 200) < 0)
        {
            pr_err("ERROR: gpio_set_debounce - %d\n", GPIO_163);
            //gpio_free(GPIO_163);
        }

    #endif

    //Get the IRQ number for our GPIO
    GPIO_irqNumber = gpio_to_irq(GPIO_163);
    pr_info("GPIO_irqNumber = %d\n", GPIO_irqNumber);

    if (request_threaded_irq( GPIO_irqNumber,             //IRQ number
                    (void *)gpio_irq_handler,   //IRQ handler (Top half)
                    gpio_interrupt_thread_fn,   //IRQ Thread handler (Bottom half)
                    IRQF_TRIGGER_RISING,        //Handler will be called in raising edge
                    "misc_gpio_driver",         //used to identify the device name using this IRQ
                    NULL))                      //device id for shared IRQ
    {
        pr_err("my_device: cannot register IRQ ");
        gpio_free(GPIO_163);
    }

    ktime = ktime_set(5, 0); // 5초 call back setting
    hrtimer_init(&etx_hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    etx_hr_timer.function = &timer_callback;
    hrtimer_start( &etx_hr_timer, ktime, HRTIMER_MODE_REL);
 

    printk("%s\n", __FUNCTION__);
    
    return 0;
}

static void __exit misc_gpio_exit(void)
{
    hrtimer_cancel(&etx_hr_timer);
    free_irq(GPIO_irqNumber, NULL);
    gpio_free(GPIO_163);
    gpio_free(GPIO_162);
    misc_deregister(&misc_gpio_device);
    platform_driver_unregister(&platform_gpio_driver);
    
    printk("%s\n", __FUNCTION__);
}
 
module_init(misc_gpio_init);
module_exit(misc_gpio_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Teddy");
MODULE_DESCRIPTION("A Misc device driver for processing registered GPIO with toggle option using thread interrupt.");
MODULE_VERSION("1.0");


/**
 * 
[ 3938.314000] misc_gpio_init
[ 3938.318000] GPIO_irqNumber = 157
[ 3938.319000] misc_register init done!!!

 * 
 */


    // plat_data = platform_get_drvdata(pdev); // test
    // printk(KERN_ALERT "test gpio name: %s\n", plat_data->name);
///////////////////////////////////////////////////////////////////////////////

    // alive_gpio_direction_output(palive, 2, 1);
    // val = misc_getbit(palive, 2);
    // printk(KERN_ALERT "after output direction 1 pin value: %d\n", val);

    // printk(KERN_ALERT "output set 1\n");
    // alive_gpio_direction_output(palive, 2, 1);
    // alive_gpio_set_value(palive, 2, 1);
    // mdelay(10);
    
    // val = alive_gpio_get_value(palive, 2);
    // printk(KERN_ALERT "after output direction 1 pin value: %d\n", val);
    

    // printk(KERN_ALERT "output set 0\n");
    // alive_gpio_direction_output(palive, 2, 0);
    // alive_gpio_set_value(palive, 2, 0);
    // mdelay(10);

    // val = alive_gpio_get_value(palive, 2);
    // printk(KERN_ALERT "after output direction 0 pin value: %d\n", val);


    // alive_gpio_set_value(palive, 2, 0);              // -> LED on routine
    // val = alive_gpio_get_value(palive, 2);
    // printk(KERN_ALERT "set pin value: %d\n", val); 

    // alive_gpio_direction_input(palive, 2);          -> LED off routine  1
    // alive_gpio_set_value(palive, 1);                -> LED off routine  2
    /////////////////////////////////////////////////////////////////////////////////