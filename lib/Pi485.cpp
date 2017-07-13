#include <jni.h>
#include <stdio.h>
#include "Pi485.h"
#include "Rs485.h"

Rs485 rs485;

JNIEXPORT void JNICALL Java_com_nexten_pi485_Pi485_open(JNIEnv *env, jobject thisObj, jint baudRate, jint gpioDE) {
    //printf("Opening com port...\n");    
    rs485.open(baudRate, gpioDE);
}

JNIEXPORT void JNICALL Java_com_nexten_pi485_Pi485_close(JNIEnv *env, jobject thisObj) {
    //printf("Closing com port...\n");    
    rs485.close();
}

JNIEXPORT void JNICALL Java_com_nexten_pi485_Pi485_clear(JNIEnv *env, jobject thisObj) {
    //printf("Discarding rx data...\n");    
    rs485.clear();
}

JNIEXPORT void JNICALL Java_com_nexten_pi485_Pi485_write(JNIEnv *env, jobject thisObj, jintArray buffer) {
    int length = env->GetArrayLength(buffer);
    int* arr = env->GetIntArrayElements(buffer, NULL);
    unsigned char bufferTx[length];
    
    //printf("length=%d | bufferTx=", length);    
    for (int i = 0; i < length; i++) {
        bufferTx[i] = arr[i];
        //printf("%d;", bufferTx[i]);
    }
    //printf("\n");
       
    rs485.write(bufferTx, length);
}

JNIEXPORT jintArray JNICALL Java_com_nexten_pi485_Pi485_read(JNIEnv *env, jobject thisObj, jint lengthExpected, jint timeout) {    
    unsigned char bufferRx[256];
    int size = rs485.read(bufferRx, lengthExpected, timeout);
    if (size < 1) {
        return NULL;
    }        
    
    jintArray result;
    result = env->NewIntArray(size);
    if (result == NULL) {
        return NULL; // out of memory error thrown 
    }
    
    // fill a temp structure to use to populate the java int array
    jint fill[size];
    for (int i = 0; i < size; i++) {
        fill[i] = bufferRx[i];
    }
    
    // move from the temp structure to the java structure
    env->SetIntArrayRegion(result, 0, size, fill);
    return result;
}