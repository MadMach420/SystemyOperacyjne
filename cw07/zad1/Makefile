all: main hairdresser

main: main.c
	gcc -Wall -pthread -o main main.c

hairdresser: hairdresser.c
	gcc -Wall -pthread -o hairdresser hairdresser.c

clean:
	rm main hairdresser