#include "bib.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define BCM2835_PERIPH_BASE     0x20000000
#define BCM2835_GPIO_BASE       ( BCM2835_PERIPH_BASE + 0x200000 )

/*
 * Paging definitions.
 */

#define RPI_PAGE_SIZE           4096
#define RPI_BLOCK_SIZE          4096

uint32_t *volatile gpio_base;
static int mmap_fd;

/*
 * Helper macros for accessing GPIO registers.
 */

#define GPIO_CONF_FUNC_REG( ptr, gpio ) \
    *( (ptr) + ( (gpio) / 10 ) ) 

#define GPIO_CONF_FUNC_INPUT_MASK( gpio ) \
    ( ~( 0x7 << ( ( (gpio) % 10 ) * 3 ) ) ) 

#define GPIO_CONF_FUNC_OUTPUT_MASK( gpio ) \
    ( 0x1 << ( ( (gpio) % 10 ) * 3 ) ) 

#define GPIO_CONF_AS_INPUT( ptr, gpio ) \
    GPIO_CONF_FUNC_REG( ptr, gpio ) &= GPIO_CONF_FUNC_INPUT_MASK( gpio ) 

#define GPIO_CONF_AS_OUTPUT( ptr, gpio ) \
    do { \
        GPIO_CONF_AS_INPUT( ptr, gpio ); \
        GPIO_CONF_FUNC_REG( ptr, gpio ) |= \
            GPIO_CONF_FUNC_OUTPUT_MASK( gpio ); \
    } while ( 0 ) 

#define GPIO_CONF_REG( ptr, addr, gpio ) \
    *( (ptr) + ( ( (addr) / sizeof( uint32_t ) ) + ( (gpio) / 32 ) ) ) 

#define GPIO_SET_REG( ptr, gpio ) \
    GPIO_CONF_REG( ptr, 0x1c, gpio )

#define GPIO_CLR_REG( ptr, gpio ) \
    GPIO_CONF_REG( ptr, 0x28, gpio )

#define GPIO_SET( ptr, gpio ) \
    GPIO_SET_REG( ptr, gpio ) = 1 << ( (gpio) % 32 ) 

#define GPIO_CLR( ptr, gpio ) \
    GPIO_CLR_REG( ptr, gpio ) = 1 << ( (gpio) % 32 )

#define GPIO_VALUE( ptr, gpio ) \
    ( ( GPIO_CONF_REG( ptr, 0x34, gpio ) >> ( (gpio) % 32 ) ) & 0x1 ) 






int gpio_init(){
	void * mmap_result;
    mmap_fd = open ( "/dev/mem", O_RDWR | O_SYNC );
    if ( mmap_fd < 0 ) {
        return -1;
    }
    mmap_result = mmap (
        NULL
      , RPI_BLOCK_SIZE
      , PROT_READ | PROT_WRITE
      , MAP_SHARED
      , mmap_fd
      , BCM2835_GPIO_BASE );

    if ( mmap_result == MAP_FAILED ) {
        close ( mmap_fd );
        return -1;
    }
    gpio_base =  mmap_result;
    return 0;
}


int gpio_setup(int gpio, int direction){
	if( direction == 0){
		GPIO_CONF_AS_INPUT(gpio_base,gpio);
	}
	else if( direction == 1){
		GPIO_CONF_AS_OUTPUT(gpio_base,gpio);
	}
	else{
		return -1;
	}
	return 0;
}


int gpio_read (int gpio,int *val){
	*val = GPIO_VALUE(gpio_base,gpio);
	if( *val != 1 || *val != 0){
		return -1;
	}
	return 0;
}


int gpio_write(int gpio, int val){
	if ( val == 1){
		GPIO_SET(gpio_base,gpio);
	}
	else if( val == 0){
		GPIO_CLR(gpio_base,gpio);
	}
	else{
		return -1;
	}
	return 0;
}
