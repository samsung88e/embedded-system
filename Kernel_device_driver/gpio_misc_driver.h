#ifndef GPIO_MISC_DIRVER_H
#define GPIO_MISC_DIRVER_H

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <asm/gpio.h>
#include <linux/of.h>
#include <linux/io.h>
#include <asm/io.h>
#include <linux/time.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

struct nx_gpio_regs
{
	u32	data;		/* Data register */
	u32	outputenb;	/* Output Enable register */
	u32	detmode[2];	/* Detect Mode Register */
	u32	intenb;		/* Interrupt Enable Register */
	u32	det;		/* Event Detect Register */
	u32	pad;		/* Pad Status Register */
};

struct nx_alive_gpio_regs 
{
	u32	pwrgate;	/* Power Gating Register */
	u32	reserved0[28];	/* Reserved0 */
	u32	outputenb_reset;/* Alive GPIO Output Enable Reset Register */
	u32	outputenb;	/* Alive GPIO Output Enable Register */
	u32	outputenb_read; /* Alive GPIO Output Read Register */
	u32	reserved1[3];	/* Reserved1 */
	u32	pad_reset;	/* Alive GPIO Output Reset Register */
	u32	data;		/* Alive GPIO Output Register */
	u32	pad_read;	/* Alive GPIO Pad Read Register */
	u32	reserved2[33];	/* Reserved2 */
	u32	pad;		/* Alive GPIO Input Value Register */
};

u32 misc_getbit(u32 base, unsigned pin);
void misc_setbit(u32 base, unsigned pin);
void misc_clrbit(u32 base, unsigned pin);

u32 alive_gpio_get_value(struct nx_alive_gpio_regs const *base, unsigned pin);
void alive_gpio_direction_output(struct nx_alive_gpio_regs const *base, unsigned pin, int val);
void alive_gpio_direction_input(struct nx_alive_gpio_regs const *base, unsigned pin);
void alive_gpio_set_value(struct nx_alive_gpio_regs const *base, unsigned pin, int val);

#endif