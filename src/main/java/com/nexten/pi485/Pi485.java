package com.nexten.pi485;

import java.util.Arrays;

/**
 *
 * @author Jaques Claudino Jul 12, 2017
 */
public class Pi485 {
    
    /**
     * Load native library at runtime: pi485.dll (Windows) or libpi485.so (Unixes)
     */
    static {
        System.loadLibrary("pi485");
    }
    
    private native void open(int baudRate, int gpioDE);
    private native void write(int[] buffer);
    private native int[] read(int lengthExpected, int timeout);

    /**
     * Method used to test from command line.
     * @param args buffer used to transmit.
     */
    public static void main(String[] args) {
        if (args.length < 5) {
            System.out.println("Sintaxe: sudo java...Pi485 baudRate gpioDE lengthExpected timeout bufferTx...");
            System.out.println("Example: sudo java -classpath ./pi485-0.0.1.jar -Djava.library.path=./ com.nexten.pi485.Pi485 19200 12 -1 1000 12 1 0 4 223");
            return;
        }

        int index = 0;
        int baudRate = Integer.parseInt(args[index++]);
        int gpioDE = Integer.parseInt(args[index++]);
        int lengthExpected = Integer.parseInt(args[index++]);
        int timeout = Integer.parseInt(args[index++]);
        
        int[] bufferTx = new int[args.length - index];
        for (int i = 0; i < bufferTx.length; i++) {
            bufferTx[i] = Integer.parseInt(args[index++].trim());
        }
        System.out.printf("bufferTx=%s\n", Arrays.toString(bufferTx));
        
        Pi485 pi485 = new Pi485();        
        pi485.open(baudRate, gpioDE);
        pi485.write(bufferTx);
        
        long start = System.currentTimeMillis();
        int bufferRx[] = pi485.read(lengthExpected, timeout);
        System.out.printf("bufferRx=%s | %d ms\n", Arrays.toString(bufferRx), System.currentTimeMillis() - start);
    }

}
