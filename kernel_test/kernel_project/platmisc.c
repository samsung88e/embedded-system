

#include <linux/types.h>
#include "platmisc.h"

int misc_getbit(struct nx_alive_gpio_regs const *base, unsigned pin)
{
    // void __iomem *reg_addr;
    void *pvaddr;
    unsigned int value;
    unsigned int mask;

    pvaddr = ioremap((u32)base, 32); /* 주소 가져오기 */
    value = ioread32(pvaddr); /* 레지스터 값 가져오기 */

    mask = 1UL << pin;
    value = (value & mask) >> pin;

    // iounmap(reg_addr);

    return value;
}

int misc_setbit(struct nx_alive_gpio_regs const *base, unsigned pin)
{
    void __iomem *reg_addr;
    unsigned int value;
    unsigned int mask;

    reg_addr = ioremap(base, 32); 
    value = ioread32(reg_addr);

    mask = 1UL << pin;
    value = (value | mask);

    iowrite32(value, reg_addr);
    iounmap(reg_addr);

    return 0;
}    

int misc_clrbit(struct nx_alive_gpio_regs const *base, unsigned pin)
{
    void __iomem *reg_addr;
    unsigned int value;
    unsigned int mask;

    reg_addr = ioremap(base, 32);
    value = ioread32(reg_addr);

    mask = 1UL << pin;
    value = value & ~mask;

    iowrite32(value, reg_addr);
    iounmap(reg_addr);

    return 0;
}

int alive_gpio_get_value(struct nx_alive_gpio_regs const *base, unsigned pin)
{
	unsigned int value;
	value = misc_getbit(&base->pad, pin);
	return value;
}

int alive_gpio_direction_output(struct nx_alive_gpio_regs const *base, unsigned pin,
				     int val)
{
	if (val) 
		misc_setbit(&base->data, pin);
	else     
		misc_setbit(&base->pad_reset, pin);

	misc_setbit(&base->outputenb, pin);

	return 0;
}

int alive_gpio_direction_input(struct nx_alive_gpio_regs const *base, unsigned pin)
{
	misc_setbit(&base->outputenb_reset, pin);

	return 0;
}

int alive_gpio_set_value(struct nx_alive_gpio_regs const *base, unsigned pin, int val)
{
	if (val)    
		misc_setbit(&base->data, pin);
	else        
		misc_clrbit(&base->pad_reset, pin);

	return 0;
}
