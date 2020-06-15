# $@ matches the target
# $^ matches all dependency
# $< matches the first dependency
# $* matches the target filename without the extension

all : clean libpi485.so

libpi485.so : Pi485.o Rs485.o
	gcc -shared -o $@ $^

# $@ matches the target, $< matches the first dependancy
Pi485.o : Pi485.cpp Pi485.h
#	gcc -I"C:\java\jdk1.7.0_79\include" -I"C:\java\jdk1.7.0_79\include\win32" -c $< -o $@
#	gcc -I"/usr/java/latest/include" -I"/usr/java/latest/include/linux" -c $< -o $@
#	gcc -I"/usr/lib/jvm/jdk-8-oracle-arm-vfp-hflt/include" -I"/usr/lib/jvm/jdk-8-oracle-arm-vfp-hflt/include/linux" -c $< -o $@
	gcc -I"/opt/jdk1.8.0_241/include" -I"/opt/jdk1.8.0_241/include/linux" -c $< -o $@

Pi485.h : 
#	/usr/java/latest/bin/javah -classpath ../target/classes -o $@ com.nexten.pi485.Pi485
	javah -classpath ../target/classes -o $@ com.nexten.pi485.Pi485

Rs485.o : Rs485.cpp
	gcc -c $< -o $@

clean :
	rm -f Pi485.h *.o *.dll *.so *~
