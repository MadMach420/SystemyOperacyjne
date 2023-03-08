static_buffer_lib: buffer.c
	gcc -Wall -c buffer.c
	ar rcs libbuffer.a buffer.o

shared_buffer_lib: buffer.c
	gcc -Wall -fPIC -c buffer.c
	gcc -Wall -shared -fPIC buffer.c -o libbuffer.so

clean:
	rm *o *a main
