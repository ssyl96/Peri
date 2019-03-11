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
#include "bib.h"



int
main ( int argc, char **argv )
{
    gpio_init();
    gpio_setup(4,1);
   	gpio_write(4,1);
  
    

    return 0;
}
