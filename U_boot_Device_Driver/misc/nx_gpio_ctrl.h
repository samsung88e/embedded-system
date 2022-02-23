/*
 * (C) Copyright 2022 Nexell
 *
 * Hyunwoo, Kim <teddy0530@nexell.co.kr>
 *
 * *
 */

#ifndef __NX_GPIO_CTRL_H
#define __NX_GPIO_CTRL_H

#define NX_GPIO_IOCTL_DIRECTION_IN  0x0000
#define NX_GPIO_IOCTL_DIRECTION_OUT 0x0001
#define NX_GPIO_DIR_IN              0x0000 
#define NX_GPIO_DIR_OUT             0x0001

int nx_gpio_ctrl_read(struct udevice *dev, 
    int offset, void *buf, int size);
int nx_gpio_ctrl_write(struct udevice *dev,
    int offset, void *buf, int size);
int nx_gpio_ctrl_ioctl(struct udevice *dev,
    unsigned long request, void *buf);
int nx_gpio_ctrl_probe(struct udevice *dev);
int nx_gpio_ctrl_ofdata_to_platdata(struct udevice *dev);

#endif

