/*
 * (C) Copyright 2020 CoAsiaNexell Co., Ltd.
 *
 * SeongO, Park <ray@coasia.com>
 *
 * * SPDX-License-Identifier:      GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <misc.h>
#include <mapmem.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/nx_gpio.h>
#include <nx_i2c_gpio.h>
#include <nx_gpio_ctrl.h>

DECLARE_GLOBAL_DATA_PTR;

// platdata format
struct nx_gpio_ctrl_platdata {
	int type;
	int num;
	int pad_func;
	struct gpio_desc gpio;
};

// gpio read
static int nx_gpio_ctrl_read(struct udevice *dev, int offset, void *buf, int size)
{
	int ret;
	unsigned char *data = buf;
	struct nx_gpio_ctrl_platdata *plat = dev_get_platdata(dev);
	ret = dm_gpio_set_dir_flags(&plat->gpio, GPIOD_IS_IN);

	if(ret)
	{
		printf("[%s] dm_gpio_set_dir_flags() failed!!!\n", __FUNCTION__);
		return -EINVAL;
	}
	ret = dm_gpio_get_value( &plat->gpio );
	data[0] = ret;
	return 0;
}

// gpio write
static int nx_gpio_ctrl_write(struct udevice *dev, int offset, void *buf, int size)
{
	int ret;
	unsigned char *data = buf;
	struct nx_gpio_ctrl_platdata *plat = dev_get_platdata(dev);
	ret = dm_gpio_set_dir_flags(&plat->gpio, GPIOD_IS_OUT);
	if( ret != 0 )
	{
		printf("[%s] dm_gpio_set_dir_flags() failed!!!\n", __FUNCTION__);
		return -EINVAL;
	}
	ret = dm_gpio_set_value( &plat->gpio, data[0]);
	if( ret != 0 )
	{
		printf("[%s] dm_gpio_set_value() failed!!!\n", __FUNCTION__);
		return -EINVAL;
	}
	return 0;
}

//
int nx_gpio_ctrl_ioctl(struct udevice *dev, unsigned long request, void *buf)
{
	struct nx_gpio_ctrl_platdata *plat = dev_get_platdata(dev);

    switch (request)
    {
        case NX_GPIO_IOCTL_DIRECTION_OUT:
        {
            char dir = ((char*)buf)[0];
            if(dir == 0)
                printf("--direction error\n");
            else
                printf("--output mode--\n");
        }
        break;

        case NX_GPIO_IOCTL_DIRECTION_IN:
        {
            char dir = ((char*)buf)[0];
            if(dir == 0)
                printf("--input mode--\n");
            else
                printf("direction error\n");
        }
        break;
        
        default:
            return -EINVAL;
    }
}
/* s5p4418-daudio-ref.dts*/

// nx_gpio_ctrl@0{
//     compatible = "nexell,nx_gpio_ctrl";
//     gpio = <&gpio_alv 0 1>; // LCD ON/OFF Key
//     pad_func = <1>;
// };
static const struct udevice_id nx_gpio_ctrl_ids[] = {
	{ .compatible = "nexell,nx_gpio_ctrl" },
	{}
};

int nx_gpio_ctrl_probe(struct udevice *dev)
{
	struct nx_gpio_ctrl_platdata *plat = dev_get_platdata(dev);
	nx_gpio_set_pad_function(plat->type, plat->num, plat->pad_func);
	return 0;
}

int nx_gpio_ctrl_ofdata_to_platdata(struct udevice *dev)
{
	struct nx_gpio_ctrl_platdata *plat = dev_get_platdata(dev);
	int node, ret, num;
	ret = gpio_request_by_name(dev, "gpio", 0, &plat->gpio, GPIOD_IS_IN);
	if( ret < 0 )
	{
		printf("[%s] failed to get gpios information(node=%d, of_offset=%d)\n", __func__, node, dev->of_offset);
		return -EINVAL;
	}
	num = gpio_get_number(&plat->gpio);

	if( num < 0 )
	{
		printf("[%s] failed to gpio_get_number()\n", __func__);
		return -EINVAL;
	}
	plat->pad_func = fdtdec_get_int(gd->fdt_blob, dev->of_offset, "pad_func", 5 );

	/* convert num to nexell gpio type and number */
	plat->type = num / nx_gpio_max_bit;
	plat->num = num % nx_gpio_max_bit;

	printf("platdata : type(%d), num(%d), pad_func(%d)\n", 
		   plat->type, plat->num, plat->pad_func );

	return 0;
}

static const struct misc_ops nx_gpio_ctrl_ops = {
	.read = nx_gpio_ctrl_read,
	.write = nx_gpio_ctrl_write,
	.ioctl = nx_gpio_ctrl_ioctl
};

U_BOOT_DRIVER(nx_gpio_ctrl) = {
	.name = "nx_gpio_ctrl",
	.id = UCLASS_MISC,
	.probe = nx_gpio_ctrl_probe,
	.of_match = nx_gpio_ctrl_ids,
	.ofdata_to_platdata = nx_gpio_ctrl_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct nx_gpio_ctrl_platdata),
	.ops = &nx_gpio_ctrl_ops,
};