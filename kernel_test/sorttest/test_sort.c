/**
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sort.h>		// sort()
#include <linux/slab.h>		// kmalloc_array(), kfree()

#define TEST_LEN 10

int cmpint(const void *a, const void *b)
{
	if (*(const int *)a < *(const int *)b)
		return -1;
	else if (*(const int *)a > *(const int *)b)
		return 1;
	else
		return 0;
}

static int __init test_sort_init(void)
{
	int *a;
	int i;
	int r;
	int err;

	printk("test_sort: test_sort_init(): starting...\n");

	err = -ENOMEM;
	a = kmalloc_array(TEST_LEN, sizeof(*a), GFP_KERNEL);
	if (!a)
		return err;

	pr_info("sort before data[%d]: ", TEST_LEN);
	for (i = 0; i < TEST_LEN; i++) {
		r = (r * 725861) % 6599;
		a[i] = r;

		pr_info("%d ", a[i]);
	}

	sort(a, TEST_LEN, sizeof(*a), cmpint, NULL);

	for (i = 0; i < TEST_LEN - 1; i++) {
		if (a[i] > a[i + 1]) {
			pr_err("test has failed\n");
			
			err = -EINVAL;
			goto exit;
		}
	}

	pr_info("test passed\n");

	err = 0;
exit:
	kfree(a);
	return 0;

}

static void __exit test_sort_exit(void) 
{
	printk("linux-modules: lib/test_sort.c: test_sort_exit().\n");
}

module_init(test_sort_init);
module_exit(test_sort_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hyunwoo Kim");
MODULE_DESCRIPTION("Test sort function.");
MODULE_VERSION("0.1.0");