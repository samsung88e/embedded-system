#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  //copy_to/from_user()
#include <linux/gpio.h>     //GPIO
#include <linux/interrupt.h>
#include <linux/jiffies.h>


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
   unsigned long diff = jiffies - old_jiffie;

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

/*
** This function is the threaded irq handler
*/
static irqreturn_t gpio_interrupt_thread_fn(int irq, void *dev_id) 
{
  led_toggle = (0x01 ^ led_toggle);                             // toggle the old value
  gpio_set_value(GPIO_162, led_toggle);                      // toggle the GPIO_162
  pr_info("Interrupt(Threaded Handler) : GPIO_162 : %d ",gpio_get_value(GPIO_162));
  
  return IRQ_HANDLED;
}


dev_t dev = 0;
static struct class *dev_class;
static struct cdev cdev;
 
static int __init gpio_driver_init(void);
static void __exit gpio_driver_exit(void);
 
 
// Driver functions
static int open(struct inode *inode, struct file *file);
static int release(struct inode *inode, struct file *file);
static ssize_t read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t write(struct file *filp, const char *buf, size_t len, loff_t * off);

//File operation structure 
static struct file_operations fops =
{
    .owner          = THIS_MODULE,
    .read           = read,
    .write          = write,
    .open           = open,
    .release        = release,
};

static int open(struct inode *inode, struct file *file)
{
  pr_info("Device File Opened\n");
  return 0;
}

static int release(struct inode *inode, struct file *file)
{
  pr_info("Device File Closed\n");
  return 0;
}


// This function will be called when we read the Device file
static ssize_t read(struct file *filp, 
                char __user *buf, size_t len, loff_t *off)
{
  uint8_t gpio_state = 0;
  
  //reading GPIO value
  gpio_state = gpio_get_value(GPIO_162);
  
  //write to user
  len = 1;
  if(copy_to_user(buf, &gpio_state, len) > 0) 
  {
    pr_err("ERROR: copy_to_user error\n");
  }
  pr_info("Read function : GPIO_21 = %d \n", gpio_state);
  
  return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t write(struct file *filp, 
                const char __user *buf, size_t len, loff_t *off)
{
  uint8_t rec_buf[10] = {0};
  
  if( copy_from_user( rec_buf, buf, len ) > 0) {
    pr_err("ERROR: copy_from_user error\n");
  }
  
  pr_info("Write Function : GPIO_21 Set = %c\n", rec_buf[0]);
  
  if (rec_buf[0]=='1') {
    //set the GPIO value to HIGH
    gpio_set_value(GPIO_162, 1);
  } else if (rec_buf[0]=='0') {
    //set the GPIO value to LOW
    gpio_set_value(GPIO_162, 0);
  } else {
    pr_err("Unknown command : Please provide either 1 or 0 \n");
  }
  
  return len;
}

/*
** Module Init function
*/
static int __init gpio_driver_init(void)
{
  /*Allocating Major number*/
  if((alloc_chrdev_region(&dev, 0, 1, "Dev")) <0){
    pr_err("Cannot allocate major number\n");
    goto r_unreg;
  }
  pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

  /*Creating cdev structure*/
  cdev_init(&cdev,&fops);

  /*Adding character device to the system*/
  if((cdev_add(&cdev,dev,1)) < 0){
    pr_err("Cannot add the device to the system\n");
    goto r_del;
  }

  /*Creating struct class*/
  if((dev_class = class_create(THIS_MODULE,"class")) == NULL){
    pr_err("Cannot create the struct class\n");
    goto r_class;
  }

  /*Creating device*/
  if((device_create(dev_class,NULL,dev,NULL,"my_device")) == NULL){
    pr_err( "Cannot create the Device \n");
    goto r_device;
  }
  
  //Output GPIO configuration    gpio_free(GPIO_163);
  //Checking the GPIO is valid or not
  if(gpio_is_valid(GPIO_162) == false){
    pr_err("GPIO %d is not valid\n", GPIO_162);
    goto r_device;
  }
  
  //Requesting the GPIO
  if(gpio_request(GPIO_162,"GPIO_162_OUT") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_162);
    goto r_gpio_out;
  }
  
  //configure the GPIO as output
  gpio_direction_output(GPIO_162, 1);
  
  //Input GPIO configuratioin
  //Checking the GPIO is valid or not
  if(gpio_is_valid(GPIO_163) == false){
    pr_err("GPIO %d is not valid\n", GPIO_163);
    goto r_gpio_in;
  }
  
  //Requesting the GPIO
  if(gpio_request(GPIO_163,"GPIO_163_IN") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_163);
    goto r_gpio_in;
  }
  
  //configure the GPIO as input
  gpio_direction_input(GPIO_163);
  
  /*
  ** I have commented the below few lines, as gpio_set_debounce is not supported 
  ** in the Raspberry pi. So we are using EN_DEBOUNCE to handle this in this driver.
  */ 
#ifndef EN_DEBOUNCE
  //Debounce the button with a delay of 200ms
  if(gpio_set_debounce(GPIO_163, 200) < 0){
    pr_err("ERROR: gpio_set_debounce - %d\n", GPIO_163);
    //goto r_gpio_in;
  }
#endif
  
  //Get the IRQ number for our GPIO
  GPIO_irqNumber = gpio_to_irq(GPIO_163);
  pr_info("GPIO_irqNumber = %d\n", GPIO_irqNumber);
  
  if (request_threaded_irq( GPIO_irqNumber,             //IRQ number
                            (void *)gpio_irq_handler,   //IRQ handler (Top half)
                            gpio_interrupt_thread_fn,   //IRQ Thread handler (Bottom half)
                            IRQF_TRIGGER_RISING,        //Handler will be called in raising edge
                            "my_device",                //used to identify the device name using this IRQ
                            NULL))                      //device id for shared IRQ
  {
    pr_err("my_device: cannot register IRQ ");
    goto r_gpio_in;
  }
 
  pr_info("Device Driver Insert...Done!!!\n");
  return 0;

r_gpio_in:
    gpio_free(GPIO_163);
r_gpio_out:
    gpio_free(GPIO_162);
r_device:
    device_destroy(dev_class,dev);
r_class:
    class_destroy(dev_class);
r_del:
    cdev_del(&cdev);
r_unreg:
    unregister_chrdev_region(dev,1);
  
  return -1;
}

/*
** Module exit function
*/ 
static void __exit gpio_driver_exit(void)
{
    free_irq(GPIO_irqNumber,NULL);
    gpio_free(GPIO_163);
    gpio_free(GPIO_162);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove\n");
}
 
module_init(gpio_driver_init);
module_exit(gpio_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Teddy KIM");
MODULE_DESCRIPTION("Threaded IRQ (GPIO Interrupt) ");
MODULE_VERSION("1.0");