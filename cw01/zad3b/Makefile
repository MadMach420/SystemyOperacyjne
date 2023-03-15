all: test

test: mains_o create_tests
	./main_static_O < input > results_static_O.txt
	./main_shared_O < input > results_shared_O.txt
	./main_dynamic_O < input > results_dynamic_O.txt
	./main_static_O0 < input > results_static_O0.txt
	./main_shared_O0 < input > results_shared_O0.txt
	./main_dynamic_O0 < input > results_dynamic_O0.txt
	./main_static_Os < input > results_static_Os.txt
	./main_shared_Os < input > results_shared_Os.txt
	./main_dynamic_Os < input > results_dynamic_Os.txt

test_3a: mains create_tests
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

mains_o: main_static_o main_shared_o main_dynamic_o

main_static_o: main.c static_buffer_lib
	gcc -O -Wall -o main_static_O main.c -L. -lbuffer
	gcc -O0 -Wall -o main_static_O0 main.c -L. -lbuffer
	gcc -Os -Wall -o main_static_Os main.c -L. -lbuffer

main_shared_o: main.c shared_buffer_lib
	gcc -O -Wall -o main_shared_O main.c -L. -lbuffer
	gcc -O0 -Wall -o main_shared_O0 main.c -L. -lbuffer
	gcc -Os -Wall -o main_shared_Os main.c -L. -lbuffer
	sudo cp libbuffer.so /usr/lib

main_dynamic_o: main_dynamic.c shared_buffer_lib
	gcc -O -Wall -o main_dynamic_O main_dynamic.c
	gcc -O0 -Wall -o main_dynamic_O0 main_dynamic.c
	gcc -Os -Wall -o main_dynamic_Os main_dynamic.c

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
	rm *o *a main_static_* main_shared_* main_dynamic_* input results*
