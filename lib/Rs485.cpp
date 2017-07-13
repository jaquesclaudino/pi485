/* 
 * File:   Rs485.cpp
 * Author: Jaques Claudino
 * 
 * Created on Jul 12, 2017, 8:30 PM
 */

#include "Rs485.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/time.h>

// Access from ARM Running Linux

//#define BCM2708_PERI_BASE 0x20000000
#define BCM2708_PERI_BASE   0x3F000000
#define UART_BASE   (BCM2708_PERI_BASE + 0x201000)
#define GPIO_BASE   (BCM2708_PERI_BASE + 0x200000)

#define PAGE_SIZE   (4*1024)
#define BLOCK_SIZE  (4*1024)

#define DR      (0x00 >> 2)
#define RSRECR  (0x04 >> 2)
#define FR      (0x18 >> 2)
#define ILPR    (0x20 >> 2)
#define IBRD    (0x24 >> 2)
#define FBRD    (0x28 >> 2)
#define LCRH    (0x2C >> 2)
#define CR      (0x30 >> 2)
#define IFLS    (0x34 >> 2)
#define IMSC    (0x38 >> 2)
#define RIS     (0x3C >> 2)
#define MIS     (0x40 >> 2)
#define ICR     (0x44 >> 2)
#define DMACR   (0x48 >> 2)
#define ITCR    (0x80 >> 2)
#define ITIP    (0x84 >> 2)
#define ITOP    (0x88 >> 2)
#define TDR     (0x8C >> 2)

#define FR_TXFE 0x80
#define FR_RXFF 0x40
#define FR_TXFF 0x20
#define FR_RXFE 0x10
#define FR_BUSY 0x08
#define FR_DCD  0x04
#define FR_DSR  0x02
#define FR_CTS  0x01

#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

long int currentTimeMillis() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (((long int)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

void msleep(unsigned int millis) {
    usleep(1000*millis);
}

volatile unsigned *map_peripheral(unsigned int address) {
    void *map;
    int mem_fd;

    if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
        printf("can't open /dev/mem \n");
        exit(-1);
    }

    map = mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, address);

    close(mem_fd);

    if (map == MAP_FAILED) {
        printf("mmap error %d\n", (int) map); //errno also set!
        exit(-1);
    }

    return (volatile unsigned *) map;
}

speed_t getBaudrateSpeed(unsigned int baudRate) {
    switch (baudRate) {
        case 1200: return B1200;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200; 
        default: 
            printf("Baud rate %d not supported. Using 9600 bps.\n", baudRate);
            return B9600;
    }
}

void Rs485::open(unsigned int baudRate, unsigned int gpioDE) {
    this->gpioDE = gpioDE;
    
    uart = map_peripheral(UART_BASE);
    gpio = map_peripheral(GPIO_BASE);

    fd = ::open("/dev/ttyAMA0", O_RDWR | O_NONBLOCK);
    if (!fd) {
        printf("Error opening /dev/ttyAMA0\n");
        exit(10);
    }

    //CONFIGURE THE UART
    //The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
    //CSIZE:- CS5, CS6, CS7, CS8
    //CLOCAL - Ignore modem status lines
    //CREAD - Enable receiver
    //IGNPAR = Ignore characters with parity errors
    //ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comm
    //PARENB - Parity enable
    //PARODD - Odd parity (else even)
    struct termios options; 
    tcgetattr(fd, &options);
    options.c_cflag = getBaudrateSpeed(baudRate) | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;  
    options.c_lflag = 0;  
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &options);

    // gpio used as output to control the RS485 DE (driver enable) pin:
    INP_GPIO(gpioDE);
    OUT_GPIO(gpioDE);
    GPIO_CLR = 1 << gpioDE;
}

void Rs485::close() {
    if (!fd) {
        printf("uart closed\n");
        return;
    }
    ::close(fd);
    fd = 0;
}

void Rs485::clear() {
    if (!fd) {
        printf("uart closed\n");
        return;
    }    
    
    unsigned char bufferRx[256];
    int available;
    
    //discard all rx data:
    do {
        available = ::read(fd, bufferRx, 255);
    } while (available == 255);
}

void Rs485::write(unsigned char* bufferTx, unsigned int length) {
    if (!fd) {
        printf("uart closed\n");
        return;
    }
    
    // tx mode:
    GPIO_SET = 1 << gpioDE;
    
    int count = ::write(fd, bufferTx, length);
    if (count < 0) {
        printf("tx error\n");
    }
        
    fsync(fd);
    while (!(uart[FR] & FR_TXFE)) {
        continue;
    }
    while ((uart[FR] & FR_BUSY)) {
        continue;
    }
    
    // rx mode:
    GPIO_CLR = 1 << gpioDE;
}

unsigned int Rs485::read(unsigned char* bufferRx, unsigned int lengthExpected, unsigned int millisTimeout) {
    if (!fd) {
        printf("uart closed\n");
        return 0;
    }
    unsigned char bufferTmp[256];
    unsigned int millisCount = 0;
    unsigned int indexRx = 0;    
    
    while (millisCount < millisTimeout) {
        int available = ::read(fd, bufferTmp, 255);
        if (available > 0) {
            //printf("read available %d \n", available);
            for (unsigned int i = 0; i < available; i++) {
                bufferRx[indexRx++] = bufferTmp[i];
                if (indexRx == lengthExpected) {
                    return lengthExpected;
                }
            }
        } else if ((lengthExpected == -1 && indexRx > 0) || (lengthExpected != -1 && indexRx >= lengthExpected)) {            
            //printf("read break\n");
            break;
        } else {
            //printf("read delay\n");
            msleep(1);
            millisCount++;
        }
    }
    return indexRx;
}
