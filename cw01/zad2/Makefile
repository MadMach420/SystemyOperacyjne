all: main_static main_shared main_dynamic

main_static: main.c static_buffer_lib
	gcc -Wall -o main_static main.c -L. -lbuffer

main_shared: main.c shared_buffer_lib
	export LD_LIBRARY_PATH=$(pwd)
	gcc -Wall -o main_shared main.c -L. -lbuffer

main_dynamic: main_dynamic.c shared_buffer_lib
	gcc -Wall -o main_dynamic main_dynamic.c

static_buffer_lib: buffer.c
	gcc -Wall -c buffer.c
	ar rcs libbuffer.a buffer.o

shared_buffer_lib: buffer.c
	gcc -Wall -fPIC -c buffer.c
	gcc -Wall -shared -fPIC buffer.c -o libbuffer.so

clean:
	rm *o *a main_static main_shared main_dynamic
