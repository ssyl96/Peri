# changer les valeurs par défaut de RPI et DIR
RPI?=25
DIR?=sylla-sang
APP=lcd_user

CFLAGS=-Wall -Wfatal-errors -O2
LDFLAGS=-static -L$(LIB) -lgpio

KERNELDIR       = /dsk/l1/misc/linux-rpi-3.18.y
CROSSDIR        = /users/enseig/franck/peri
CROSS_COMPILE   = $(CROSSDIR)/arm-bcm2708hardfp-linux-gnueabi/bin/bcm2708hardfp-

all:  $(APP).x upload

$(APP).x: $(APP).c
	$(CROSS_COMPILE)gcc -o $@ $< -I$(INC) -O2 -static

upload: 
	scp -P50$(RPI) $(APP).x pi@peri:$(DIR)
clean:
	rm -f *.o *.x *~
