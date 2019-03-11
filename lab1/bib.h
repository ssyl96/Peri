#ifndef H_LIB
#define H_LIB


int gpio_init();
int gpio_setup(int gpio, int direction);
int gpio_read (int gpio,int *val);
int gpio_write(int gpio, int val);

#endif