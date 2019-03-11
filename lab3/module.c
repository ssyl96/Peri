#include <linux/module.h>
#include <linux/init.h>

#define NBMAX_LED 32
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Charlie, 2015");
MODULE_DESCRIPTION("Module, aussitot insere, aussitot efface");

static int btn;
module_param(btn, int, 0);
MODULE_PARM_DESC(btn, "numéro du port du bouton");

static int leds[NBMAX_LED];
static int nbled;
module_param_array(leds, int, &nbled, 0);
MODULE_PARM_DESC(LEDS, "tableau des numéros de port LED");

static int __init mon_module_init(void)
{	
	int i;
    printk(KERN_DEBUG "Hello World !\n");
    printk(KERN_DEBUG "btn=%d !\n", btn);
    for (i=0; i < nbled; i++)
       printk(KERN_DEBUG "LED %d = %d\n", i ,leds[i]);
    return 0;
}
static void __exit mon_module_cleanup(void)
{
   printk(KERN_DEBUG "Goodbye World!\n");
}

module_init(mon_module_init);
module_exit(mon_module_cleanup);