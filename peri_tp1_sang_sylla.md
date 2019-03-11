# [TP1 PERI] Sidiki Sylla, Loic Sang 
## Hello World! Raspberry PI 1
Nous avons écrit un hello world, et en utilisant le fichier makefile, en le faisant exécuter sur le raspberry PI.

## 2.2. Contrôle de GPIO en sortie 
1. Nous avons réussi à allumer une led de la raspberry PI en ajoutant une ligne de code : GPIO_CLR( gpio_base, GPIO_LED0 ) dans la boucle while.
2. Nous sommes bloqués pendant un moment pour allumer les deux leds à des fréquences différentes en utilisant des threads, mais au final, il s'agit d'une inattention car les deux instruction pour allumer et éteindre la led s'enchainent(pas de délai) et par conséquence, les leds clignotent toujours à la même fréquence.
Voici quelques lignes de code importantes:
 ```c
 /* On a défini une structure qui regroupe  
 *  tous les arguments de la fonction blink ()
 *  lorsqu'on utilise les threads.
 */
struct led{
    uint32_t volatile *gpio;
    int period;
    int l;
};


/*Elle permet de faire clignoter la LED
 * à une fréquence qui se trouve dans la variable args*/
void *blink (void *args){
    struct led *led = args;     
    while(1){
        GPIO_SET(led->gpio,led->l);
        printf("periode : %d\n",led->period);
        delay(led->period/2);
        GPIO_CLR(led->gpio,led->l);
        delay(led->period/2);
    }
}

 ```

## 2.4. Lecture de la valeur d'une entrée GPIO
Nous avons configuré le GPIO 18 entré avec la macro GPIO_CONF_AS_INPUT, puis en utilisant la macro GPIO_VALUE puis lire la valeur du bouton. Cependant, le bouton sur le raspberry 24 attribué à nous, ne marche pas très bien. Nous avons alors testé notre programme sur un autre (le numéro 25).
Pour faire clignoter les leds plus rapidement, nous avons mis les deux variables périodes (period1,period2) en variable globale.
On crée un thread pour chaque GPIO (les deux leds et le bouton), le thread de bouton lit la valeur du bouton, augmente la fréquence( multipliée par 2 la variable période) pour un appui ou diminuer la fréquence si la fréquence est doublée précédemment. Comme nous le montre ci-dessous:
```c
//Ces deux variables sont déclarées en globales

int period1 = 2000;
int period2 = 1000;
...
//Une attention particulière à la variable *period, pour pouvoir faire communiquer les threads entre elles
struct led{
    uint32_t volatile *gpio;
    int *period; 
    int l;
};
...
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
```

## 2.5 Réalisation d'une petite librairie
Pour pouvoir écrire la library libgpio.a, nous avons commencé par reprendre toutes les macros fournies dans le fichier de départ.
Ensuite, l'écriture des fonctions consiste à réutiliser ces macros. Comme le fichier bib.c nous  montre ci-dessous.

```c
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
```
Pour compiler ce fichier bib.c en une bibliothèque, on utilise le cross-compilateur avec l'option -c qui produit un fichier bib.o , puis utiliser la commande bcm2708hardfp-ar -rcs libgpio.a bib.o.
On crée un fichier test.c pour tester les fonctions, pour compiler il faut utiliser le cross-compilateur avec libgpio.a lorsqu'on lance le programme dans le même répertoire.


##3 Arduino
1.  Pour faire clignoter la LED 13, on a la fonction pinMode(LED_BUILTIN, OUTPUT) appelée dans la fonction setup() avec LED_BUILTIN fixée par défaut au gpio 13 qui est la led et OUTPUT pour configurer ce gpio comme sortie.
Dans la fonction loop(). Avec la fonction digitalWrite(LED_BUILTIN, param) pour allumer et éteindre la led en fonction du param LOW pour éteinte et HIGH pour allumée et un delay d’une seconde entre chaque état.


2. 3.  On remarque sur notre breadboard la photorésistance est connectée sur le port analogique A1.
En salle sesi on considère (via le serial port de l’arduino) que la lumière qu’on perçoit est au tour de 204 - 231 et lorsque la photorésistante reçoit un flash on est autour de 512 on print sur le serial port qu’on a reçut un flash.
```c
int val = 1;
int new_val = 1;
// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  pinMode(13,OUTPUT);
  val = analogRead(A1); // initialize with the current light 
}

// the loop routine runs over and over again forever:
void loop() {
  new_val = analogRead(A1);  // read the analog in value on gpio A1
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  // print the results to the Serial Monitor:
  Serial.println(new_val);
  if (new_val <val){ 
    digitalWrite(13,LOW); // sets the digital pin 13 on
  }
  if (new_val >val)
  {
    digitalWrite(13,HIGH);
    Serial.println("reçoit un flash");
    
  }
  
  delay(1000);
}

```

# Code de l'exercice 2.4. Lecture de la valeur d'une entrée GPIO
```c
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
```

# Code de l'exercice 2.5 Réalisation d'une petite librairie
* bib.h
```c
#ifndef H_LIB
#define H_LIB


int gpio_init();
int gpio_setup(int gpio, int direction);
int gpio_read (int gpio,int *val);
int gpio_write(int gpio, int val);

#endif
```

* bib.c
```c
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

```

## Code de la partie Arduino

```c
/*

ReadAnalogVoltage

Reads an analog input on pin 0, converts it to voltage, and prints the result to the serial monitor.

Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

This example code is in the public domain.

*/

int val = 1;

int new_val = 1;

// the setup routine runs once when you press reset:

void setup() {

// initialize serial communication at 9600 bits per second:

Serial.begin(9600);

pinMode(13,OUTPUT);

val = analogRead(A1); // initialize with the current light

}

  

// the loop routine runs over and over again forever:

void loop() {

new_val = analogRead(A1); // read the analog in value on gpio A1

// Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):

// print the results to the Serial Monitor:

Serial.println(new_val);

if (new_val <val){

digitalWrite(13,LOW); // sets the digital pin 13 on

}

if (new_val >val)

{

digitalWrite(13,HIGH);

Serial.println("reçoit un flash");

}

delay(1000);

}
```






