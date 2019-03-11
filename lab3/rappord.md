# [TP3 PERI] Sidiki Sylla, Loic Sang 
##Étape 1 : création et test d'un module noyau
## Questions
* La fonction suivante est exécuté lorsqu'on insère le module dans le noyau.
```c
static int  __init mon_module_init(void) 
```
* La fonction suivante est exécuté lorsqu'on enlève le module dans le noyau.
```c
static void __exit mon_module_cleanup(void)
```

## Travail à faire
* La premiere commande sudo insmod ./module.ko insère le module dans le noyau.
* lsmod permet d'afficher les modules chargés dans le noyau, nous devons retrouver le module inséré auparavant.
* Avec dmesg on voit affiché sur "Hello World<votre nom> !", cela veut dire que la fonction d'insertion a été bien exécutée dans le noyau.
* sudo rmmod module permet de supprimer le module chargé auparavant.
* Ainsi, en faisant dmesg nous voyons le message "Goodbye World!" cela veut dire que la fonction de suppresion a été bien exécutée dans le noyau.

#Étape 2 : ajout des paramètres au module
## Questions
```c
static int btn;
module_param(btn, int, 0);
```
La variable btn est initialisée à 0, lorsqu'on exécute: sudo insmod ./module.ko btn=18, nous vérifions cette valeur dans le terminal avec dmesg.


```c
static int leds[NBMAX_LED];
static int nbled;
module_param_array(leds, int, &nbled, 0);

static int __init mon_module_init(void)
{
    printk(KERN_DEBUG "Hello World !\n");
    for (int i=0; i < nbled; i++)
       printk(KERN_DEBUG "LED %d = %d\n", i,leds[i]);
    return 0;
}
``` 
Il faut remplace l'argument nbled par leds dans la fonction module_param_array.
Lorsqu'on exécute: sudo insmod ./module.ko leds=4,17 ; leds[0] = 4 et leds[1] = 17.

#Étape 3 : création d'un driver qui ne fait rien, mais qui le fait dans le noyau 
## Questions
* Pour vérifier que le device a été bien créé dans le raspberry pi, on tape dans le terminal: cat /proc/devices, et on cherche le device( ledbp) est dans ce fichier. Dans notre cas, on voit que le major de notre device est 246.
* echo "rien" > /dev/ledbp cette commande ouvre le fichier /dev/ledbp et écrire "rien" puis ferme le fichier.
* dd bs=1 count=1 < /dev/ledbp, on ouvre le fichier /dev/ledbp et on redirige ce flux vers la commande dd.

* Les scripts insdev et rmdev nous permettront respectivement d'automatiser le chargement et le déchargement des modules dans le noyau en fonction des paramètres qu'on donne aux scripts.

#Étape 4 : accès aux GPIO depuis les fonctions du pilote
* `gpio_fsel(int pin, int fun)` permet de configurer le gpio en entrée ou sortie selon la valeur de fun.
* `gpio_write(int pin,bool val)` permet de allumer ou éteindre la led selon la valeur de val.
* ` gpio_read(int pin )` retourne la valeur du bouton. 
Pour lire la valeur du bouton, nous devons utiliser le registre GPLEV0. La valeur du bouton est stocké dans le pin(18), on décale le 18e bit du registre vers le bit le plus faible puis nous faisons un & logique avec 1 pour récupérer la valeur du bouton.
Voici le code ci-dessous:

```c
static uint32_t gpio_read(int pin)
{
	return (( gpio_regs->gplev[pin / 32] >> ( (pin) % 32)) & 0b1); 

}
```
* Lorsqu'on ouvre le driver ledbp, on doit configurer la led en input. Donc, la fonction `gpio_fsel` est invoquée dans la fonction d'ouverture du module. On suit la meme logique pour les fonctions `gpio_read` et `gpio_write`. 
Par ailleurs, la fonction `kmalloc` donne des erreurs dans le noyau. Nous avons opté pour une autre méthode afin de faire clignoter la led.
```c
static int open_ledbp(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "open()\n");
    gpio_fsel(LED0,1);//Output pour la led
    gpio_fsel(BTN,0);//Input pour le bouton
    return 0;
}

static ssize_t read_ledbp(struct file *file, char *buf, size_t count, loff_t *ppos) {
    printk(KERN_DEBUG "read()\n");
    uint32_t value =   gpio_read(BTN);
    /*
		Vérifier la valeur du button. Si elle est appuyée alors on mettra le '1' dans le buffer (ce que l'utilisateur lira).
		sinon l'utilisateur lira '0'.
    */
    if (value == 0) 
    {
    	buf[0] = '0';
    }
    else {
    	buf[0] = '1';
    }
    return count;
}

static ssize_t write_ledbp(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    printk(KERN_DEBUG "write()\n");
    printk(KERN_DEBUG " buf :%c\n",buf[0]);
    /*
    Si l'utilisateur écrit '1' alors on allume la led sinon on l'éteint.
    */
    if(buf[0] == '1'){
    	gpio_write(LED0,true);
	}
	else{
		gpio_write(LED0,false);
	}
    return count;
}
```

# Code complet
```c
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/unistd.h>
#include <mach/platform.h>
#include <linux/slab.h>

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


static void gpio_fsel(int pin, int fun) //fonction qui permet de choisir le mode du gpio (Entrée ou Sortie)
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
    gpio_fsel(LED0,1);//Output pour la led
    gpio_fsel(BTN,0);//Input pour le bouton
    return 0;
}

static ssize_t read_ledbp(struct file *file, char *buf, size_t count, loff_t *ppos) {
    printk(KERN_DEBUG "read()\n");
    uint32_t value =   gpio_read(BTN);
    if (value == 0)
    {
    	buf[0] = '0';
    }
    else {
    	buf[0] = '1';
    }
    return count;
}

static ssize_t write_ledbp(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    printk(KERN_DEBUG "write()\n");
    printk(KERN_DEBUG " buf :%c\n",buf[0]);
    if(buf[0] == '1'){
    	gpio_write(LED0,true);
	}
	else{
		gpio_write(LED0,false);
	}
    return count;
}

static int release_ledbp(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "close()\n");
    gpio_write(LED0,false);
    //kfree(file->private_data), file->private_data = NULL;
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
```