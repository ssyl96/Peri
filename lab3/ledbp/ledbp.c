#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/unistd.h>
#include <mach/platform.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Charlie, 2015");
MODULE_DESCRIPTION("Module, aussitot insere, aussitot efface");
static int major;
	
static const int LED0 = 4;
static const int BTN = 18;


struct gpio_s
{
    uint32_t gpfsel[7];
    uint32_t gpset[3];
    uint32_t gpclr[3];
    uint32_t gplev[3];
    uint32_t gpeds[3];
    uint32_t gpren[3];
    uint32_t gpfen[3];
    uint32_t gphen[3];
    uint32_t gplen[3];
    uint32_t gparen[3];
    uint32_t gpafen[3];
    uint32_t gppud[1];
    uint32_t gppudclk[3];
    uint32_t test[1];
}
*gpio_regs = (struct gpio_s *)__io_address(GPIO_BASE);



//fonction qui permet de choisir le mode du gpio (Entrée ou Sortie)
static void gpio_fsel(int pin, int fun) 
{
    uint32_t reg = pin / 10;
    uint32_t bit = (pin % 10) * 3;
    uint32_t mask = 0b111 << bit;
    gpio_regs->gpfsel[reg] = (gpio_regs->gpfsel[reg] & ~mask) | ((fun << bit) & mask);
}

static void gpio_write(int pin, bool val)
{
    if (val)
        gpio_regs->gpset[pin / 32] = (1 << (pin % 32));
    else
        gpio_regs->gpclr[pin / 32] = (1 << (pin % 32));
}

static uint32_t gpio_read(int pin)
{
	return (( gpio_regs->gplev[pin / 32] >> ( (pin) % 32)) & 0b1); 

}



static int open_ledbp(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "open()\n");
    //mettre la LED0 en output
    gpio_fsel(LED0,1);
    gpio_fsel(BTN,0);
    return 0;
}

static ssize_t read_ledbp(struct file *file, char *buf, size_t count, loff_t *ppos) {
    printk(KERN_DEBUG "read()\n");
    uint32_t v = gpio_read(BTN);
    printk(KERN_DEBUG "value: %d\n",v);
    return count;
}

static ssize_t write_ledbp(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    printk(KERN_DEBUG "write()\n");
    gpio_write(LED0,true);
    return count;
}

static int release_ledbp(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "close()\n");
    //éteindre la led lorsqu'on ferme le driver
    gpio_write(LED0,false);
    return 0;
}


struct file_operations fops_ledbp =
{
    .open       = open_ledbp,
    .read       = read_ledbp,
    .write      = write_ledbp,
    .release    = release_ledbp 
};

static int __init mon_module_init(void)
{
    major = register_chrdev(0, "ledbpSS", &fops_ledbp); // 0 est le numéro majeur qu'on laisse choisir par linux
    return 0;
}

static void __exit mon_module_cleanup(void)
{
    unregister_chrdev(major, "ledbpSS");
}

module_init(mon_module_init);
module_exit(mon_module_cleanup);





