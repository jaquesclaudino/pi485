/* 
 * File:   Rs485.h
 * Author: Jaques Claudino
 *
 * Created on Jul 12, 2017, 8:30 PM
 */

#ifndef RS485_H
#define RS485_H

class Rs485 {
public:
    void open(unsigned int baudRate, unsigned int gpioDE);
    void close();
    void clear(); //discard rx data
    void write(unsigned char* bufferTx, unsigned int length);
    unsigned int read(unsigned char* bufferTx, unsigned int lengthExpected, unsigned int timeout);
            
private:  
    unsigned int gpioDE; //rs 485 driver enable pin
    
    volatile unsigned *uart;
    volatile unsigned *gpio;
    int fd;
};

#endif /* RS485_H */

