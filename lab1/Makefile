NAME = ''
RPI ?= 25

CROSS_COMPILE ?= bcm2708hardfp-

CFLAGS=-Wall -Wfatal-errors -O2
LDFLAGS=-static -L. -lgpio -lpthread


help:
	@echo ""
	@echo "make [RPI=num] target"
	@echo ""
	@echo "      target :	help	this message"
	@echo " 		all	clean all and recompile"
	@echo " 		dep	compile if required"
	@echo " 		clean	clean all except sources"
	@echo " 		copy	compile then copy to card $(RPI) in directory $(NAME)"
	@echo ""

all: distclean 

dep : lab1_ex3.x

copy: lab1_ex3.x
	scp -P 50$(RPI) $^ pi@peri:$(NAME)


%.x: %.c
	$(CROSS_COMPILE)gcc -o $@ $^ $(LDFLAGS)

libgpio.a: bib.o
	$(CROSS_COMPILE)ar -rcs $@ $^

%.o: %.c
	$(CROSS_COMPILE)gcc -o $@ -c $(CFLAGS) $<

clean:
	rm -f *.o *.x *~ *.a
