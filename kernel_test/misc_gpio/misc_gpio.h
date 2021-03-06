#ifndef MISC_GPIO.H
#define MISC_GPIO.H

/*
pinctrl-nexell.h
*/

/**
 * enum pincfg_type - possible pin configuration types supported.
 * @PINCFG_TYPE_FUNC: Function configuration.
 * @PINCFG_TYPE_DAT: Pin value configuration.
 * @PINCFG_TYPE_PULL: Pull up/down configuration.
 * @PINCFG_TYPE_DRV: Drive strength configuration.
 * @PINCFG_TYPE_DIR: pin direction for GPIO mode.
 */
enum pincfg_type {
	PINCFG_TYPE_FUNC,
	PINCFG_TYPE_DAT,onfiguration type.
 */
#define PINCFG_TYPE_MASK		0xFF
#define PINCFG_VALUE_SHIFT		8
#define PINCFG_VALUE_MASK		(0xFF << PINCFG_VALUE_SHIFT)
#define PINCFG_PACK(type, value)	(((value) << PINCFG_VALUE_SHIFT) | type)
#define PINCFG_UNPACK_TYPE(cfg)		((cfg) & PINCFG_TYPE_MASK)
#define PINCFG_UNPACK_VALUE(cfg)	(((cfg) & PINCFG_VALUE_MASK) >> \
						PINCFG_VALUE_SHIFT)
/**
 * enum eint_type - possible external interrupt types.
 * @EINT_TYPE_NONE: bank does not support external interrupts
 * @EINT_TYPE_GPIO: bank supportes external gpio interrupts
 * @EINT_TYPE_WKUP: bank supportes external wakeup interrupts
 *
 * GPIO controller groups all the available pins into banks. The pins
 * in a pin bank can support external gpio interrupts or external wakeup
 * interrupts or no interrupts at all. From a software perspective, the only
 * difference between external gpio and external wakeup interrupts is that
 * the wakeup interrupts can additionally wakeup the system if it is in
 * suspended state.
 */
enum eint_type {
	EINT_TYPE_NONE,
	EINT_TYPE_GPIO,
	EINT_TYPE_WKUP,
};

/* maximum length of a pin in pin descriptor (example: "gpioa-30") */
#define PIN_NAME_LENGTH	10

#define PIN_GROUP(n, p, f)				\
	{						\
		.name		= n,			\
		.pins		= p,			\
		.num_pins	= ARRAY_SIZE(p),	\
		.func		= f			\
	}

#define PMX_FUNC(n, g)					\
	{						\
		.name		= n,			\
		.groups		= g,			\
		.num_groups	= ARRAY_SIZE(g),	\
	}

struct nexell_pinctrl_drv_data;

/**
 * struct nexell_pin_bank: represent a controller pin-bank.
 * @pctl_offset: starting offset of the pin-bank registers.
 * @pin_base: starting pin number of the bank.
 * @nr_pins: number of pins included in this bank.
 * @eint_func: function to set in CON register to configure pin as EINT.
 * @eint_type: type of the external interrupt supported by the bank.
 * @eint_mask: bit mask of pins which support EINT function.
 * @name: name to be prefixed for each pin in this pin bank.
 * @of_node: OF node of the bank.
 * @drvdata: link to controller driver data
 * @irq_domain: IRQ domain of the bank.
 * @gpio_chip: GPIO chip of the bank.
 * @grange: linux gpio pin range supported by this bank.
 * @slock: spinlock protecting bank registers
 */
struct nexell_pin_bank {
	u32		pctl_offset;
	u32		pin_base;
	u8		nr_pins;
	u8		eint_func;
	enum eint_type	eint_type;
	u32		eint_mask;
	void __iomem	*virt_base;
	char		*name;
	void		*soc_priv;
	int		irq;
	struct device_node *of_node;
	struct nexell_pinctrl_drv_data *drvdata;
	struct irq_domain *irq_domain;
	struct gpio_chip gpio_chip;
	struct pinctrl_gpio_range grange;
	struct nexell_irq_chip *irq_chip;
	spinlock_t slock;
};

/**
 * struct nexell_pin_ctrl: represent a pin controller.
 * @pin_banks: list of pin banks included in this controller.
 * @nr_banks: number of pin banks.
 * @base: starting system wide pin number.
 * @nr_pins: number of pins supported by the controller.
 * @eint_gpio_init: platform specific callback to setup the external gpio
 *	interrupts for the controller.
 * @eint_alive_init: platform specific callback to setup the external wakeup
 *	interrupts for the controller.
 * @label: for debug information.
 */
struct nexell_pin_ctrl {
	struct nexell_pin_bank	*pin_banks;
	u32		nr_banks;

	u32		base;
	u32		nr_pins;

	int		(*base_init)(struct nexell_pinctrl_drv_data *);
	int		(*gpio_irq_init)(struct nexell_pinctrl_drv_data *);
	int		(*alive_irq_init)(struct nexell_pinctrl_drv_data *);
	void		(*suspend)(struct nexell_pinctrl_drv_data *);
	void		(*resume)(struct nexell_pinctrl_drv_data *);
};

/**
 * struct nexell_pinctrl_drv_data: wrapper for holding driver data together.
 * @node: global list node
 * @virt_base: register base address of the controller.
 * @dev: device instance representing the controller.
 * @irq: interrpt number used by the controller to notify gpio interrupts.
 * @ctrl: pin controller instance managed by the driver.
 * @pctl: pin controller descriptor registered with the pinctrl subsystem.
 * @pctl_dev: cookie representing pinctrl device instance.
 * @pin_groups: list of pin groups available to the driver.
 * @nr_groups: number of such pin groups.
 * @pmx_functions: list of pin functions available to the driver.
 * @nr_function: number of such pin functions.
 */
struct nexell_pinctrl_drv_data {
	struct list_head		node;
	void __iomem			*virt_base;
	struct device			*dev;
	int				irq;

	struct nexell_pin_ctrl		*ctrl;
	struct pinctrl_desc		pctl;
	struct pinctrl_dev		*pctl_dev;

	const struct nexell_pin_group	*pin_groups;
	unsigned int			nr_groups;
	const struct nexell_pmx_func	*pmx_functions;
	unsigned int			nr_functions;
};

/**
 * struct nexell_pin_group: represent group of pins of a pinmux function.
 * @name: name of the pin group, used to lookup the group.
 * @pins: the pins included in this group.
 * @num_pins: number of pins included in this group.
 * @func: the function number to be programmed when selected.
 */
struct nexell_pin_group {
	const char		*name;
	const unsigned int	*pins;
	u8			num_pins;
	u8			func;
};

/**
 * struct nexell_pmx_func: represent a pin function.
 * @name: name of the pin function, used to lookup the function.
 * @groups: one or more names of pin groups that provide this function.
 * @num_groups: number of groups included in @groups.
 */
struct nexell_pmx_func {
	const char		*name;
	const char		**groups;
	u8			num_groups;
	u32			val;
};

/* list of all exported SoC specific data */
extern const struct nexell_pin_ctrl s5pxx18_pin_ctrl[];
extern const struct nexell_pin_ctrl nxp5540_pin_ctrl[];


/*
s5pxx18-gpio.h
*/

#define PAD_MD_POS 0
#define PAD_MD_MASK 0xF

#define PAD_FN_POS 4
#define PAD_FN_MASK 0xF

#define PAD_LV_POS 8
#define PAD_LV_MASK 0xF

#define PAD_PU_POS 12
#define PAD_PU_MASK 0xF

#define PAD_ST_POS 16
#define PAD_ST_MASK 0xF

#define PIN_MODE_ALT (0)
#define PIN_MODE_IN (1)
#define PIN_MODE_OUT (2)
#define PIN_MODE_INT (3)

#define PIN_FUNC_ALT0 (0)
#define PIN_FUNC_ALT1 (1)
#define PIN_FUNC_ALT2 (2)
#define PIN_FUNC_ALT3 (3)

#define PIN_LEVEL_LOW (0)		/* if alive, async lowlevel */
#define PIN_LEVEL_HIGH (1)		/* if alive, async highlevel */
#define PIN_LEVEL_FALLINGEDGE (2)	/* if alive, async fallingedge */
#define PIN_LEVEL_RISINGEDGE (3)	/* if alive, async eisingedge */
#define PIN_LEVEL_LOW_SYNC (4)		/* if gpio , not support */
#define PIN_LEVEL_HIGH_SYNC (5)		/* if gpio , not support */
#define PIN_LEVEL_BOTHEDGE (4)		/* if alive, not support */
#define PIN_LEVEL_ALT (6)		/* if pad function is alt, not set */

#define PIN_PULL_DN (0)			/* Do not support Alive-GPIO */
#define PIN_PULL_UP (1)
#define PIN_PULL_OFF (2)

#define PIN_STRENGTH_0 (0)
#define PIN_STRENGTH_1 (1)
#define PIN_STRENGTH_2 (2)
#define PIN_STRENGTH_3 (3)

#define PAD_MODE_ALT ((PIN_MODE_ALT & PAD_MD_MASK) << PAD_MD_POS)
#define PAD_MODE_IN ((PIN_MODE_IN & PAD_MD_MASK) << PAD_MD_POS)
#define PAD_MODE_OUT ((PIN_MODE_OUT & PAD_MD_MASK) << PAD_MD_POS)
#define PAD_MODE_INT ((PIN_MODE_INT & PAD_MD_MASK) << PAD_MD_POS)

#define PAD_FUNC_ALT0 ((PIN_FUNC_ALT0 & PAD_FN_MASK) << PAD_FN_POS)
#define PAD_FUNC_ALT1 ((PIN_FUNC_ALT1 & PAD_FN_MASK) << PAD_FN_POS)
#define PAD_FUNC_ALT2 ((PIN_FUNC_ALT2 & PAD_FN_MASK) << PAD_FN_POS)
#define PAD_FUNC_ALT3 ((PIN_FUNC_ALT3 & PAD_FN_MASK) << PAD_FN_POS)

#define PAD_LEVEL_LOW                                                          \
	((PIN_LEVEL_LOW & PAD_LV_MASK)                                         \
	 << PAD_LV_POS) /* if alive, async lowlevel */
#define PAD_LEVEL_HIGH                                                         \
	((PIN_LEVEL_HIGH & PAD_LV_MASK)                                        \
	 << PAD_LV_POS) /* if alive, async highlevel */
#define PAD_LEVEL_FALLINGEDGE                                                  \
	((PIN_LEVEL_FALLINGEDGE & PAD_LV_MASK)                                 \
	 << PAD_LV_POS) /* if alive, async fallingedge */
#define PAD_LEVEL_RISINGEDGE                                                   \
	((PIN_LEVEL_RISINGEDGE & PAD_LV_MASK)                                  \
	 << PAD_LV_POS) /* if alive, async eisingedge */
#define PAD_LEVEL_LOW_SYNC                                                     \
	((PIN_LEVEL_LOW_SYNC & PAD_LV_MASK)                                    \
	 << PAD_LV_POS) /* if gpio , not support */
#define PAD_LEVEL_HIGH_SYNC                                                    \
	((PIN_LEVEL_HIGH_SYNC & PAD_LV_MASK)                                   \
	 << PAD_LV_POS) /* if gpio , not support */
#define PAD_LEVEL_BOTHEDGE                                                     \
	((PIN_LEVEL_BOTHEDGE & PAD_LV_MASK)                                    \
	 << PAD_LV_POS) /* if alive, not support */
#define PAD_LEVEL_ALT                                                          \
	((PIN_LEVEL_ALT & PAD_LV_MASK)                                         \
	 << PAD_LV_POS) /* if pad function is alt, not set */

#define PAD_PULL_DN                                                            \
	((PIN_PULL_DN & PAD_PU_MASK)                                           \
	 << PAD_PU_POS) /* Do not support Alive-GPIO */
#define PAD_PULL_UP ((PIN_PULL_UP & PAD_PU_MASK) << PAD_PU_POS)
#define PAD_PULL_OFF ((PIN_PULL_OFF & PAD_PU_MASK) << PAD_PU_POS)

#define PAD_STRENGTH_0 ((PIN_STRENGTH_0 & PAD_ST_MASK) << PAD_ST_POS)
#define PAD_STRENGTH_1 ((PIN_STRENGTH_1 & PAD_ST_MASK) << PAD_ST_POS)
#define PAD_STRENGTH_2 ((PIN_STRENGTH_2 & PAD_ST_MASK) << PAD_ST_POS)
#define PAD_STRENGTH_3 ((PIN_STRENGTH_3 & PAD_ST_MASK) << PAD_ST_POS)

#define PAD_GET_GROUP(pin) ((pin >> 0x5) & 0x07) /* Divide 32 */
#define PAD_GET_BITNO(pin) (pin & 0x1F)
#define PAD_GET_FUNC(pin) ((pin >> PAD_FN_POS) & PAD_FN_MASK)
#define PAD_GET_MODE(pin) ((pin >> PAD_MD_POS) & PAD_MD_MASK)
#define PAD_GET_LEVEL(pin) ((pin >> PAD_LV_POS) & PAD_LV_MASK)
#define PAD_GET_PULLUP(pin) ((pin >> PAD_PU_POS) & PAD_PU_MASK)
#define PAD_GET_STRENGTH(pin) ((pin >> PAD_ST_POS) & PAD_ST_MASK)


/*
 * gpio descriptor
 */
#define IO_ALT_0 (0)
#define IO_ALT_1 (1)
#define IO_ALT_2 (2)
#define IO_ALT_3 (3)

#if defined(CONFIG_PINCTRL_S5PXX18)

#define PAD_GPIO_A (0 * 32)
#define PAD_GPIO_B (1 * 32)
#define PAD_GPIO_C (2 * 32)
#define PAD_GPIO_D (3 * 32)
#define PAD_GPIO_E (4 * 32)
#define PAD_GPIO_ALV (5 * 32)

/* s5pxx18 GPIO function number */

#define ALT_NO_GPIO_A                                                          \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,    \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0,                                                  \
	}

#define ALT_NO_GPIO_B                                                          \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,    \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_2, IO_ALT_2, IO_ALT_1, IO_ALT_2, IO_ALT_1,          \
		    IO_ALT_2, IO_ALT_1, IO_ALT_2, IO_ALT_1, IO_ALT_1,          \
		    IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1,          \
		    IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1,          \
		    IO_ALT_1,                                                  \
	}

#define ALT_NO_GPIO_C                                                          \
	{                                                                      \
		IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1,    \
		    IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1,          \
		    IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1,          \
		    IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1,          \
		    IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1,          \
		    IO_ALT_1, IO_ALT_1, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0,                                                  \
	}

#define ALT_NO_GPIO_D                                                          \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,    \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0,                                                  \
	}

#define ALT_NO_GPIO_E                                                          \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,    \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_1,          \
		    IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1, IO_ALT_1,          \
		    IO_ALT_1,                                                  \
	}

#define ALT_NO_ALIVE                                                           \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,    \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,          \
		    IO_ALT_0,                                                  \
	}

#define NUMBER_OF_GPIO_MODULE 5

/* GPIO Module's Register List */
struct nx_gpio_reg_set {
	/* 0x00	: Output Register */
	u32 GPIOxOUT;
	/* 0x04	: Output Enable Register */
	u32 GPIOxOUTENB;
	/* 0x08	: Event Detect Mode Register */
	u32 GPIOxDETMODE[2];
	/* 0x10	: Interrupt Enable Register */
	u32 GPIOxINTENB;
	/* 0x14	: Event Detect Register */
	u32 GPIOxDET;
	/* 0x18	: PAD Status Register */
	u32 GPIOxPAD;
	/* 0x1C	: Pull Up Enable Register */
	u32 GPIOxPUENB;
	/* 0x20	: Alternate Function Select Register */
	u32 GPIOxALTFN[2];
	/* 0x28	: Event Detect Mode extended Register */
	u32 GPIOxDETMODEEX;
	/* 0x2B	: */
	u32 __Reserved[4];
	/* 0x3C	: IntPend Detect Enable Register */
	u32 GPIOxDETENB;

	/* 0x40	: Slew Register */
	u32 GPIOx_SLEW;
	/* 0x44	: Slew set On/Off Register */
	u32 GPIOx_SLEW_DISABLE_DEFAULT;
	/* 0x48	: drive strength LSB Register */
	u32 GPIOx_DRV1;
	/* 0x4C	: drive strength LSB set On/Off Register */
	u32 GPIOx_DRV1_DISABLE_DEFAULT;
	/* 0x50	: drive strength MSB Register */
	u32 GPIOx_DRV0;
	/* 0x54	: drive strength MSB set On/Off Register */
	u32 GPIOx_DRV0_DISABLE_DEFAULT;
	/* 0x58	: Pull UP/DOWN Selection Register */
	u32 GPIOx_PULLSEL;
	/* 0x5C	: Pull UP/DOWN Selection On/Off Register */
	u32 GPIOx_PULLSEL_DISABLE_DEFAULT;
	/* 0x60	: Pull Enable/Disable Register */
	u32 GPIOx_PULLENB;
	/* 0x64	: Pull Enable/Disable selection On/Off Register */
	u32 GPIOx_PULLENB_DISABLE_DEFAULT;
	/* 0x68 */
	u32 GPIOx_InputMuxSelect0;
	/* 0x6C */
	u32 GPIOx_InputMuxSelect1;
};

#define NR_GPIO_MODULE	5		/* number of gpio module */
#define NR_ALIVE	6		/* number of alive pin */
#define ALIVE_INDEX	(NR_GPIO_MODULE)

#elif defined(CONFIG_PINCTRL_NXP5540)

#define PAD_GPIO_A (0 * 32)
#define PAD_GPIO_B (1 * 32)
#define PAD_GPIO_C (2 * 32)
#define PAD_GPIO_D (3 * 32)
#define PAD_GPIO_E (4 * 32)
#define PAD_GPIO_F (5 * 32)
#define PAD_GPIO_G (6 * 32)
#define PAD_GPIO_H (7 * 32)
#define PAD_GPIO_ALV (8 * 32)

#define ALT_NO_GPIO_A                                                          \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0,                                            \
	}

#define ALT_NO_GPIO_B                                                          \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0,                                            \
	}

#define ALT_NO_GPIO_C                                                          \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0,                                            \
	}

#define ALT_NO_GPIO_D                                                          \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0,                                            \
	}

#define ALT_NO_GPIO_E                                                          \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0,                                            \
	}

#define ALT_NO_GPIO_F                                                          \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0,                                            \
	}

#define ALT_NO_GPIO_G                                                          \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0,                                            \
	}

/* XXX: ball map has only H0, H1 */
#define ALT_NO_GPIO_H                                                          \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0,                                            \
	}

/* XXX: ball map has only 0 ~ 14 */
#define ALT_NO_ALIVE                                                           \
	{                                                                      \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0, IO_ALT_0,              \
		IO_ALT_0, IO_ALT_0,                                            \
	}

#define NUMBER_OF_GPIO_MODULE 8

/* GPIO Module's Register List */
struct nx_gpio_reg_set {
	/* 0x00	: Output Register */
	u32 GPIOxOUT;
	/* 0x04	: Output Enable Register */
	u32 GPIOxOUTENB;
	/* 0x08	: Event Detect Mode Register */
	u32 GPIOxDETMODE[2];
	/* 0x10	: Interrupt Enable Register */
	u32 GPIOxINTENB;
	/* 0x14	: Event Detect Register */
	u32 GPIOxDET;
	/* 0x18	: PAD Status Register */
	u32 GPIOxPAD;
	/* 0x1C */
	u32 __Reserved0;
	/* 0x20	: Alternate Function Select Register */
	u32 GPIOxALTFN[2];
	/* 0x28	: Event Detect Mode extended Register */
	u32 GPIOxDETMODEEX;
	/* 0x2B	: */
	u32 __Reserved[4];
	/* 0x3C	: IntPend Detect Enable Register */
	u32 GPIOxDETENB;

	/* 0x40	: Slew Register */
	u32 GPIOx_SLEW;
	/* 0x44	: Slew set On/Off Register */
	u32 GPIOx_SLEW_DISABLE_DEFAULT;
	/* 0x48	: drive strength LSB Register */
	u32 GPIOx_DRV1;
	/* 0x4C	: drive strength LSB set On/Off Register */
	u32 GPIOx_DRV1_DISABLE_DEFAULT;
	/* 0x50	: drive strength MSB Register */
	u32 GPIOx_DRV0;
	/* 0x54	: drive strength MSB set On/Off Register */
	u32 GPIOx_DRV0_DISABLE_DEFAULT;
	/* 0x58	: Pull UP/DOWN Selection Register */
	u32 GPIOx_PULLSEL;
	/* 0x5C	: Pull UP/DOWN Selection On/Off Register */
	u32 GPIOx_PULLSEL_DISABLE_DEFAULT;
	/* 0x60	: Pull Enable/Disable Register */
	u32 GPIOx_PULLENB;
	/* 0x64	: Pull Enable/Disable selection On/Off Register */
	u32 GPIOx_PULLENB_DISABLE_DEFAULT;
	/* 0x68, 0x6C : */
	u32 GPIOx_GPIOx_INPUT_MUX_SELECT[2];
	/* 0x70 : Secure Marking Register */
	u32 GPIOx_SECURE_MARKING;
	/* 0x74 : Input Enable/Disable Register */
	u32 GPIOx_INPUTENB;
	/* 0x78 : Input Enable/Disable set On/Off Register */
	u32 GPIOx_INPUTENB_DISABLE_DEFAULT;
};

#define NR_GPIO_MODULE	8		/* number of gpio module */
#define NR_ALIVE	15		/* number of alive pin */
#define ALIVE_INDEX	(NR_GPIO_MODULE)

#else
#error "Must specify PAD config!!"

#endif


#define GPIO_NUM_PER_BANK (32)

#define nx_gpio_intmode_lowlevel 1
#define nx_gpio_intmode_highlevel 2
#define nx_gpio_intmode_fallingedge 2
#define nx_gpio_intmode_risingedge 3
#define nx_gpio_intmode_bothedge 4

#define nx_gpio_padfunc_0 0
#define nx_gpio_padfunc_1 1
#define nx_gpio_padfunc_2 2
#define nx_gpio_padfunc_3 3

#define nx_gpio_drvstrength_0 0
#define nx_gpio_drvstrength_1 1
#define nx_gpio_drvstrength_2 2
#define nx_gpio_drvstrength_3 3

#define nx_gpio_pull_down 0
#define nx_gpio_pull_up 1
#define nx_gpio_pull_off 2


struct module_init_data {
	struct list_head node;
	void *bank_base;
	int bank_type;		/* 0: none, 1: gpio, 2: alive */
};

/*
 * nx_alive
 */

/* ALIVE GPIO Detect Mode */
#define nx_alive_detect_mode_async_lowlevel 0
#define nx_alive_detect_mode_async_highlevel 1
#define nx_alive_detect_mode_sync_fallingedge 2
#define nx_alive_detect_mode_sync_risingedge 3
#define nx_alive_detect_mode_sync_lowlevel 4
#define nx_alive_detect_mode_sync_highlevel 5

struct nx_alive_reg_set {
	/* 0x00 : Alive Power Gating Register */
	u32 ALIVEPWRGATEREG;
	/* 0x04 : Alive GPIO ASync Detect Mode Reset Register0 */
	u32 ALIVEGPIOASYNCDETECTMODERSTREG0;
	/* 0x08 : Alive GPIO ASync Detect Mode Set Register0 */
	u32 ALIVEGPIOASYNCDETECTMODESETREG0;
	/* 0x0C : Alive GPIO Low ASync Detect Mode Read Register */
	u32 ALIVEGPIOLOWASYNCDETECTMODEREADREG;

	/* 0x10 : Alive GPIO ASync Detect Mode Reset Register1 */
	u32 ALIVEGPIOASYNCDETECTMODERSTREG1;
	/* 0x14 : Alive GPIO ASync Detect Mode Set Register1 */
	u32 ALIVEGPIOASYNCDETECTMODESETREG1;
	/* 0x18 : Alive GPIO High ASync Detect Mode Read Register */
	u32 ALIVEGPIOHIGHASYNCDETECTMODEREADREG;

	/* 0x1C : Alive GPIO Detect Mode Reset Register0 */
	u32 ALIVEGPIODETECTMODERSTREG0;
	/* 0x20 : Alive GPIO Detect Mode Set Register0 */
	u32 ALIVEGPIODETECTMODESETREG0;
	/* 0x24 : Alive GPIO Falling Edge Detect Mode Read Register */
	u32 ALIVEGPIOFALLDETECTMODEREADREG;

	/* 0x28 : Alive GPIO Detect Mode Reset Register1 */
	u32 ALIVEGPIODETECTMODERSTREG1;
	/* 0x2C : Alive GPIO Detect Mode Set Register1 */
	u32 ALIVEGPIODETECTMODESETREG1;
	/* 0x30 : Alive GPIO Rising Edge Detect Mode Read Register */
	u32 ALIVEGPIORISEDETECTMODEREADREG;

	/* 0x34 : Alive GPIO Detect Mode Reset Register2 */
	u32 ALIVEGPIODETECTMODERSTREG2;
	/* 0x38 : Alive GPIO Detect Mode Set Register2 */
	u32 ALIVEGPIODETECTMODESETREG2;
	/* 0x3C : Alive GPIO Low Level Detect Mode Read Register */
	u32 ALIVEGPIOLOWDETECTMODEREADREG;

	/* 0x40 : Alive GPIO Detect Mode Reset Register3 */
	u32 ALIVEGPIODETECTMODERSTREG3;
	/* 0x44 : Alive GPIO Detect Mode Set Register3 */
	u32 ALIVEGPIODETECTMODESETREG3;
	/* 0x48 : Alive GPIO High Level Detect Mode Read Register */
	u32 ALIVEGPIOHIGHDETECTMODEREADREG;

	/* 0x4C : Alive GPIO Detect Enable Reset Register */
	u32 ALIVEGPIODETECTENBRSTREG;
	/* 0x50 : Alive GPIO Detect Enable Set Register */
	u32 ALIVEGPIODETECTENBSETREG;
	/* 0x54 : Alive GPIO Detect Enable Read Register */
	u32 ALIVEGPIODETECTENBREADREG;

	/* 0x58 : Alive GPIO Interrupt Enable Reset Register */
	u32 ALIVEGPIOINTENBRSTREG;
	/* 0x5C : Alive GPIO Interrupt Enable Set Register */
	u32 ALIVEGPIOINTENBSETREG;
	/* 0x60 : Alive GPIO Interrupt Enable Read Register */
	u32 ALIVEGPIOINTENBREADREG;

	/* 0x64 : Alive GPIO Detect Pending Register */
	u32 ALIVEGPIODETECTPENDREG;

	/* 0x68 : Alive Scratch Reset Register */
	u32 ALIVESCRATCHRSTREG;
	/* 0x6C : Alive Scratch Set Register */
	u32 ALIVESCRATCHSETREG;
	/* 0x70 : Alive Scratch Read Register */
	u32 ALIVESCRATCHREADREG;

	/* 0x74 : Alive GPIO PAD Out Enable Reset Register */
	u32 ALIVEGPIOPADOUTENBRSTREG;
	/* 0x78 : Alive GPIO PAD Out Enable Set Register */
	u32 ALIVEGPIOPADOUTENBSETREG;
	/* 0x7C : Alive GPIO PAD Out Enable Read Register */
	u32 ALIVEGPIOPADOUTENBREADREG;

	/* 0x80 : Alive GPIO PAD Pullup Reset Register */
	u32 ALIVEGPIOPADPULLUPRSTREG;
	/* 0x84 : Alive GPIO PAD Pullup Set Register */
	u32 ALIVEGPIOPADPULLUPSETREG;
	/* 0x88 : Alive GPIO PAD Pullup Read Register */
	u32 ALIVEGPIOPADPULLUPREADREG;

	/* 0x8C : Alive GPIO PAD Out Reset Register */
	u32 ALIVEGPIOPADOUTRSTREG;
	/* 0x90 : Alive GPIO PAD Out Set Register */
	u32 ALIVEGPIOPADOUTSETREG;
	/* 0x94 : Alive GPIO PAD Out Read Register */
	u32 ALIVEGPIOPADOUTREADREG;

	/* 0x98 : VDD Control Reset Register */
	u32 VDDCTRLRSTREG;
	/* 0x9C : VDD Control Set Register */
	u32 VDDCTRLSETREG;
	/* 0xA0 : VDD Control Read Register */
	u32 VDDCTRLREADREG;

	/* 0x0A4 : ALIVE CLEAR WAKEUP STATUS REGISTER */
	u32 CLEARWAKEUPSTATUS;
	/* 0x0A8 : ALIVE SLEEP WAKEUP STATUS REGISTER */
	u32 WAKEUPSTATUS;

	/* 0x0AC : ALIVE SCRATCH RESET REGISTER1 */
	u32 ALIVESCRATCHRST1;
	/* 0x0B0 : ALIVE SCRATCH SET REGISTER1 */
	u32 ALIVESCRATCHSET1;
	/* 0x0B4 : ALIVE SCRATCH READ REGISTER1 */
	u32 ALIVESCRATCHVALUE1;

	/* 0x0B8 : ALIVE SCRATCH RESET REGISTER2 */
	u32 ALIVESCRATCHRST2;
	/* 0x0BC : ALIVE SCRATCH SET REGISTER2 */
	u32 ALIVESCRATCHSET2;
	/* 0x0C0 : ALIVE SCRATCH READ REGISTER2 */
	u32 ALIVESCRATCHVALUE2;

	/* 0x0C4 : ALIVE SCRATCH RESET REGISTER3 */
	u32 ALIVESCRATCHRST3;
	/* 0x0C8 : ALIVE SCRATCH SET REGISTER3 */
	u32 ALIVESCRATCHSET3;
	/* 0x0CC : ALIVE SCRATCH READ REGISTER3 */
	u32 ALIVESCRATCHVALUE3;

	/* 0x0D0 : ALIVE SCRATCH RESET REGISTER4 */
	u32 ALIVESCRATCHRST4;
	/* 0x0D4 : ALIVE SCRATCH SET REGISTER4 */
	u32 ALIVESCRATCHSET4;
	/* 0x0D8 : ALIVE SCRATCH READ REGISTER4 */
	u32 ALIVESCRATCHVALUE4;

	/* 0x0DC : ALIVE SCRATCH RESET REGISTER5 */
	u32 ALIVESCRATCHRST5;
	/* 0x0E0 : ALIVE SCRATCH SET REGISTER */
	u32 ALIVESCRATCHSET5;
	/* 0x0E4 : ALIVE SCRATCH READ REGISTER5 */
	u32 ALIVESCRATCHVALUE5;

	/* 0x0E8 : ALIVE SCRATCH RESET REGISTER6 */
	u32 ALIVESCRATCHRST6;
	/* 0x0EC : ALIVE SCRATCH SET REGISTER6 */
	u32 ALIVESCRATCHSET6;
	/* 0x0F0 : ALIVE SCRATCH READ REGISTER6 */
	u32 ALIVESCRATCHVALUE6;

	/* 0x0F4 : ALIVE SCRATCH RESET REGISTER7 */
	u32 ALIVESCRATCHRST7;
	/* 0x0F8 : ALIVE SCRATCH SET REGISTER7 */
	u32 ALIVESCRATCHSET7;
	/* 0x0FC : ALIVE SCRATCH READ REGISTER7 */
	u32 ALIVESCRATCHVALUE7;

	/* 0x100 : ALIVE SCRATCH RESET REGISTER8 */
	u32 ALIVESCRATCHRST8;
	/* 0x104 : ALIVE SCRATCH SET REGISTER8 */
	u32 ALIVESCRATCHSET8;
	/* 0x108 : ALIVE SCRATCH READ REGISTER8 */
	u32 ALIVESCRATCHVALUE8;

	/* 0x10C : VDD OFF DELAY RESET REGISTER */
	u32 VDDOFFCNTVALUERST;
	/* 0x110 : VDD OFF DELAY SET REGISTER */
	u32 VDDOFFCNTVALUESET;

	/* 0x114 : VDD OFF DELAY VALUE REGISTER */
	u32 VDDOFFCNTVALUE;
	/* 0x118 : VDD OFF DELAY TIMER REGISTER */
	u32 VDDOFFCNTTIMER;

	/* 0x11C : ALIVE GPIO INPUT VALUE READ REGISTER */
	u32 ALIVEGPIOINPUTVALUE;
};

/*
 * nx_soc functions
 */
extern void nx_soc_gpio_set_io_func(unsigned int io, unsigned int func);
extern int nx_soc_gpio_get_altnum(unsigned int io);
extern unsigned int nx_soc_gpio_get_io_func(unsigned int io);
extern void nx_soc_gpio_set_io_dir(unsigned int io, int out);
extern int nx_soc_gpio_get_io_dir(unsigned int io);
extern void nx_soc_gpio_set_io_pull(unsigned int io, int val);
extern int nx_soc_gpio_get_io_pull(unsigned int io);
extern void nx_soc_gpio_set_io_drv(int gpio, int mode);
extern int nx_soc_gpio_get_io_drv(int gpio);
extern void nx_soc_gpio_set_out_value(unsigned int io, int high);
extern int nx_soc_gpio_get_out_value(unsigned int io);
extern int nx_soc_gpio_get_in_value(unsigned int io);
extern void nx_soc_gpio_set_int_enable(unsigned int io, int on);
extern int nx_soc_gpio_get_int_enable(unsigned int io);
extern void nx_soc_gpio_set_int_mode(unsigned int io, unsigned int mode);
extern int nx_soc_gpio_get_int_mode(unsigned int io);
extern int nx_soc_gpio_get_int_pend(unsigned int io);
extern void nx_soc_gpio_clr_int_pend(unsigned int io);

extern void nx_soc_alive_set_det_enable(unsigned int io, int on);
extern int nx_soc_alive_get_det_enable(unsigned int io);
extern void nx_soc_alive_set_det_mode(unsigned int io, unsigned int mode,
				      int on);
extern int nx_soc_alive_get_det_mode(unsigned int io, unsigned int mode);
extern int nx_soc_alive_get_int_pend(unsigned int io);
extern void nx_soc_alive_clr_int_pend(unsigned int io);

extern u32 nx_alive_get_wakeup_status(void);
extern void nx_alive_clear_wakeup_status(void);



/*
pinctrl-s5pxx18.h
*/

#define SOC_PIN_BANK_EINTN(pins, reg, id)		\
	{						\
		.pctl_offset	= reg,			\
		.nr_pins	= pins,			\
		.eint_type	= EINT_TYPE_NONE,	\
		.name		= id			\
	}

// #define SOC_PIN_BANK_EINTG(pins, reg, id)	\
// 	{							\
// 		.pctl_offset	= reg,				\
// 		.nr_pins	= pins,				\
// 		.eint_type	= EINT_TYPE_GPIO,		\
// 		.name		= id				\
// 	}

// #define SOC_PIN_BANK_EINTW(pins, reg, id)	\
// 	{							\
// 		.pctl_offset	= reg,				\
// 		.nr_pins	= pins,				\
// 		.eint_type	= EINT_TYPE_WKUP,		\
// 		.name		= id				\
// 	}


#define GPIO_INT_OUT (0x04)   /* out enable */
#define GPIO_INT_MODE0 (0x08) /* 0x08,0x0C */
#define GPIO_INT_MODE1 (0x28) /* detect mode ex */
#define GPIO_INT_ENB (0x10)
#define GPIO_INT_STATUS (0x14)
#define GPIO_INT_ALT (0x20) /* 0x20,0x24 */
#define GPIO_INT_DET (0x3C)

#define ALIVE_MOD_RESET (0x04) /* detect mode reset */
#define ALIVE_MOD_SET (0x08)   /* detect mode set */
#define ALIVE_MOD_READ (0x0C)  /* detect mode read */
#define ALIVE_DET_RESET (0x4C)
#define ALIVE_DET_SET (0x50)
#define ALIVE_DET_READ (0x54)
#define ALIVE_INT_RESET (0x58)    /* interrupt reset : disable */
#define ALIVE_INT_SET (0x5C)      /* interrupt set   : enable */
#define ALIVE_INT_SET_READ (0x60) /* interrupt set read */
#define ALIVE_INT_STATUS (0x64)   /* interrupt detect pending and clear */
#define ALIVE_OUT_RESET (0x74)
#define ALIVE_OUT_SET (0x78)
#define ALIVE_OUT_READ (0x7C)

/* alive interrupt detect type */
#define NX_ALIVE_DETECTMODE_ASYNC_LOWLEVEL 0
#define NX_ALIVE_DETECTMODE_ASYNC_HIGHLEVEL 1
#define NX_ALIVE_DETECTMODE_SYNC_FALLINGEDGE 2
#define NX_ALIVE_DETECTMODE_SYNC_RISINGEDGE 3
#define NX_ALIVE_DETECTMODE_SYNC_LOWLEVEL 4
#define NX_ALIVE_DETECTMODE_SYNC_HIGHLEVEL 5

/* gpio interrupt detect type */
#define NX_GPIO_INTMODE_LOWLEVEL 0
#define NX_GPIO_INTMODE_HIGHLEVEL 1
#define NX_GPIO_INTMODE_FALLINGEDGE 2
#define NX_GPIO_INTMODE_RISINGEDGE 3
#define NX_GPIO_INTMODE_BOTHEDGE 4

#define NX_GPIO_PADFUNC_0 0
#define NX_GPIO_PADFUNC_1 1
#define NX_GPIO_PADFUNC_2 2
#define NX_GPIO_PADFUNC_3 3

#define NX_GPIO_DRVSTRENGTH_0 0
#define NX_GPIO_DRVSTRENGTH_1 1
#define NX_GPIO_DRVSTRENGTH_2 2
#define NX_GPIO_DRVSTRENGTH_3 3

#define NX_GPIO_PULL_DOWN 0
#define NX_GPIO_PULL_UP 1
#define NX_GPIO_PULL_OFF 2

#ifdef CONFIG_ARM64
#define ARM_DMB() dmb(sy)
#else
#define ARM_DMB() dmb()
#endif


#endif
