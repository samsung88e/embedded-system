/*
 * Control GPIO pins on the fly
 *
 * Copyright (c) 2008-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <dm.h>
#include <asm/gpio.h>
#include <nx_gpio_ctrl.h>
#include <nx_i2c_gpio.h>
#include <misc.h>

/* GPIO control command */
enum gpio_cmd {
	GPIO_SET,
	GPIO_GET,
	GPIO_TOGGLE,
};

/* GPIO Control Application */
static int do_nx_gpio_ctrl(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret, i = 0;
	struct udevice *dev = 0;
	int dev_idx = 0;
	int value, sec = 0;
	int found = 0;
	int gpio_ctrl_device_idx = 0;
    enum gpio_cmd sub_cmd = 0;
    const char* str_cmd = NULL;

	if(argc < 2)
	{
		return CMD_RET_USAGE;
	}
    else
    {
        str_cmd = argv[1];
        dev_idx = simple_strtol(argv[2], NULL, 10);

        for(i = 0, ret = uclass_first_device(UCLASS_MISC, &dev); dev; i++)
        {
            if(0 == strncmp("nx_gpio_ctrl", dev->name, 12))
            {
                if(dev_idx == gpio_ctrl_device_idx)
                {
                    found = 1;
                    break;
                }
                gpio_ctrl_device_idx++;
            }
            ret = uclass_next_device(&dev);
        }

        if(found)
        {   
            /* parse the behavior */
            switch (*str_cmd) 
            {
                case 's': sub_cmd = GPIO_SET;    break;
                case 'g': sub_cmd = GPIO_GET;    break;
                case 't': sub_cmd = GPIO_TOGGLE; break;
                default:  return CMD_RET_USAGE;
            }

            /* get */
            if(sub_cmd == GPIO_GET)
            {  
                if(argc != 3)
                {
                    printf("ngc get command args error\n");
                    return CMD_RET_USAGE;
                }

                char direction = NX_GPIO_DIR_IN;
                char data;

                nx_gpio_ctrl_ioctl(dev, NX_GPIO_IOCTL_DIRECTION_IN, &direction);
                misc_read(dev, 0, &data, sizeof(data));
                printf("read value : %d\n", data); // test
            }

            /* set */
            else if(sub_cmd == GPIO_SET)
            {
                if(argc != 4)
                {
                    printf("ngc set command args error\n");
                    return CMD_RET_USAGE;
                }
                char direction = NX_GPIO_DIR_OUT;
                char value;

                nx_gpio_ctrl_ioctl(dev, NX_GPIO_IOCTL_DIRECTION_OUT, &direction);
                value = simple_strtol(argv[3], NULL, 10);
                char data = value ? 1 : 0;

                misc_write(dev, 0, &data, sizeof(data));
                printf("write value : %d\n", data); // test
            }

            /* toggle */
            else if(sub_cmd == GPIO_TOGGLE)
            {
                if(argc != 4)
                {
                    printf("ngc toggle command args error\n");
                    return CMD_RET_USAGE;
                }
                sec = simple_strtol(argv[3], NULL, 10); // sec#
                int repeat = 5;
                char data;

                for(int i = 0; i < repeat; i++)
                {
                    char direction = NX_GPIO_DIR_OUT;
                    nx_gpio_ctrl_ioctl(dev, NX_GPIO_IOCTL_DIRECTION_OUT, &direction);
                    data = data ? 0 : 1;    
                    misc_write(dev, 0, &data, sizeof(data));
                    printf("write value : %d\n", data);

                    udelay(sec*10000000);

                    direction = NX_GPIO_DIR_IN;
                    nx_gpio_ctrl_ioctl(dev, NX_GPIO_IOCTL_DIRECTION_IN, &direction);
                    misc_read(dev, 0, &data, sizeof(data));
                    printf("read value : %d\n", data);
                    printf("---------%ds--------\n", i+1);
                }
            }
            else
            {
                return CMD_RET_USAGE;
            }
        }
    }
    return 0;
}

U_BOOT_CMD(ngc, 4, 1, do_nx_gpio_ctrl,
	"Control Nexell's banked gpio pins", // usage
    "\ncommand = toggle, set, get\n"     // help
    "> ngc toggle pin# sec#\n"             
    "> ngc set pin# value(0 or 1)#\n"    
    "> ngc get pin#\n"                  
);