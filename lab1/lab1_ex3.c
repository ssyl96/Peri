/*
 * This file is a template for the first Lab in the SESI-PERI module.
 *
 * It draws inspiration from Dom & Gert code on ELinux.org.
 *
 * Authors:
 *   Julien Peeters <julien.peeters@lip6.fr>
 */

/*
 * Headers that are required for printf and mmap.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>


int period1 = 2000;
int period2 = 1000;
/*
 * Base of peripherals and base of GPIO controller.
 */

#define BCM2835_PERIPH_BASE     0x20000000
#define BCM2835_GPIO_BASE       ( BCM2835_PERIPH_BASE + 0x200000 )

/*
 * Paging definitions.
 */

#define RPI_PAGE_SIZE           4096
#define RPI_BLOCK_SIZE          4096

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







/*
 * Setup the access to memory-mapped I/O.
 */

static int mmap_fd;

int
setup_gpio_mmap ( uint32_t volatile ** ptr )
{
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

    *ptr = ( uint32_t volatile * ) mmap_result;

    return 0;
}

void
teardown_gpio_mmap ( void * ptr )
{
    munmap ( ptr, RPI_BLOCK_SIZE );
}

void
delay ( unsigned int milisec )
{
    struct timespec ts, dummy;
    ts.tv_sec  = ( time_t ) milisec / 1000;
    ts.tv_nsec = ( long ) ( milisec % 1000 ) * 1000000;
    nanosleep ( &ts, &dummy );
}

#define min( a, b ) ( a < b ? a : b )
#define max( a, b ) ( a < b ? b : a )

/*
 * Main program.
 */

#define GPIO_LED0   4
#define GPIO_LED1   17


#define GPIO_BTN0   18


struct led{
    uint32_t volatile *gpio;
    int *period;
    int l;
};
//fonction pour allumer la led
void *blink (void *args){
    struct led *led = args;
      

    while(1){
       
        GPIO_SET(led->gpio,led->l);
        printf("periode : %d\n",*(led->period));
        delay(*(led->period)/2);
        GPIO_CLR(led->gpio,led->l);
        delay(*(led->period)/2);

    }
}
//fonction pour lire le bouton 
void *read_btn(void *args){
	int val_prec = 1;
	int val_nouv = 1;
	//variable pour savoir si on doit augmenter la fréquence ou le diminuer
	int button_on = 0;
	uint32_t *volatile gpio_base = args;
	while(1){
		delay(20);
		val_nouv = GPIO_VALUE(gpio_base,GPIO_BTN0);
		if( val_nouv != val_prec){
			//le bouton est enfoncé
			if(val_nouv == 0){
				//on doit diminuer la fréquance
				if( button_on == 0){
					button_on = 1;
					period1 /=2;
					period2 /=2;
				}
				//doubler la fréquence
				else{
					button_on = 0;
					period1 *=2;
					period2 *=2;
				}
			}
		}
		//mise  à jour 
		val_prec == val_nouv;

	}
}
int
main ( int argc, char **argv )
{
    int                 result;
    int                 period, half_period;
    uint32_t volatile * gpio_base = 0;
    struct led led1, led0;
    
    pthread_t pthread1,pthread2, pthread3;


    /* Retreive the mapped GPIO memory. */
    result = setup_gpio_mmap ( &gpio_base );



    if ( result < 0 ) {
        printf ( "-- error: cannot setup mapped GPIO.\n" );
        exit ( 1 );
    }

    period = 1000; /* default = 1Hz */
    if ( argc > 1 ) {
        period = atoi ( argv[1] );
    }
    half_period = period / 2;
    
    led1.period = &period1;
    led0.period = &period2;

    led0.gpio = gpio_base;
    led1.gpio = gpio_base;

    led0.l = GPIO_LED0;
    led1.l = GPIO_LED1;

    /* Setup GPIO OF LED TO OUTPUT */
    GPIO_CONF_AS_OUTPUT ( gpio_base, GPIO_LED0 );
    GPIO_CONF_AS_OUTPUT ( gpio_base, GPIO_LED1 );

    /* Setup GPIO OF BUTTON TOP INPUT */
    GPIO_CONF_AS_INPUT(gpio_base,GPIO_BTN0);
    printf ( "-- info: start blinking.\n" );


    pthread_create(&pthread1,NULL,blink,&led1);
    pthread_create(&pthread2,NULL,blink,&led0);
    pthread_create(&pthread3,NULL,read_btn,gpio_base);

    pthread_join(pthread1,NULL);
    pthread_join(pthread2,NULL);
    pthread_join(pthread3,NULL);



    return 0;
}
