package com.nexten.pi485;

import java.util.Arrays;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author Jaques Claudino Jul 12, 2017
 */
public class Pi485 {

    private static final Logger LOG = Logger.getLogger(Pi485.class.getName());
    
    public Pi485() throws Exception {
        try {
            LOG.log(Level.INFO, "Library pi485 loading...");
            
            // Load native library libpi485.so (Unix) or pi485.dll (Windows):
            System.loadLibrary("pi485"); 

            LOG.log(Level.INFO, "Library pi485 loaded successfully");
            
        } catch (Error ex) {
            String msg = "Error loading pi485 library";
            LOG.log(Level.SEVERE, msg, ex);
            throw new Exception(msg);
        }
    }
    
    public native void open(int baudRate, int gpioDE);
    public native void close();
    public native void clear();
    public native void write(int[] buffer);
    public native int[] read(int lengthExpected, int timeout);
    public native void setPiVersion(int piVersion);
    public native int getPiVersion();
    public native String getLibVersion();

    /**
     * Method used to test from command line.
     * @param args buffer used to transmit.
     * @throws java.lang.Exception
     */
    public static void main(String[] args) throws Exception {
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
        try {
            pi485.open(baudRate, gpioDE);
            pi485.clear(); // discard old rx data
            pi485.write(bufferTx);

            long start = System.currentTimeMillis();
            int bufferRx[] = pi485.read(lengthExpected, timeout);
            System.out.printf("bufferRx=%s | %d ms\n", Arrays.toString(bufferRx), System.currentTimeMillis() - start);
        } finally {
            pi485.close();
        }
    }

}
