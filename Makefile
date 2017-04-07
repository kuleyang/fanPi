obj-m += fanPi.o

all: piFanTest.o
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
	cc -o fanpi piFanTest.o 

piFanTest.o: piFanTest.c
	cc -c piFanTest.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	rm -f *.o *.ko fanpi
