all: test

test: mains create_tests
	./main_static < input > results_static.txt
	./main_shared < input > results_shared.txt
	./main_dynamic < input > results_dynamic.txt

create_tests:
	rm -f input
	touch input
	echo "init 1024" >> input
	for f in * ; do if [ "$$f" != "input" ]; then echo "count ./$$f" >> input; fi; done
	for f in * ; do if [ "$$f" != "input" ]; then echo "show 0" >> input; echo "delete 0" >> input; fi; done
	echo "destroy" >> input
	echo "quit" >> input

mains: main_static main_shared main_dynamic

main_static: main.c static_buffer_lib
	gcc -Wall -o main_static main.c -L. -lbuffer

main_shared: main.c shared_buffer_lib
	gcc -Wall -o main_shared main.c -L. -lbuffer
	sudo cp libbuffer.so /usr/lib

main_dynamic: main_dynamic.c shared_buffer_lib
	gcc -Wall -o main_dynamic main_dynamic.c

static_buffer_lib: buffer.c
	gcc -Wall -c buffer.c
	ar rcs libbuffer.a buffer.o

shared_buffer_lib: buffer.c
	gcc -Wall -fPIC -c buffer.c
	gcc -Wall -shared -fPIC buffer.c -o libbuffer.so

clean:
	rm *o *a main_static main_shared main_dynamic input results*
