// PROSZĘ NIE USUWAĆ Z TEGO PLIKU ŻADNYCH KOMENTARZY
// Programy z usuniętymi komentarzami otrzymają złą ocenę

/**
 * W tym programie mamy dwa wątki: producenta i konsumenta. Wątek
 * producenta produkuje wartości i przechowuje je w współdzielonym
 * buforze, podczas gdy wątek konsumenta pobiera wartości z bufora.
 *
 * Używamy muteksu i dwóch zmiennych warunkowych do synchronizacji
 * dostępu do bufora. Muteks zapewnia, że tylko jeden wątek może
 * jednocześnie uzyskać dostęp do bufora, podczas gdy zmienne
 * warunkowe pozwalają wątkom sygnalizować sobie nawzajem, gdy bufor
 * jest pełny lub pusty.
 *
 * Wątek producenta zajmuje muteks, sprawdza za pomocą zmiennej count,
 * czy bufor jest pełny, i jeśli tak, oczekuje na zmiennej warunkowej
 * producer_cond. Gdy w buforze jest miejsce, wątek producenta
 * produkuje wartość, przechowuje ją w buforze i sygnalizuje zmiennej
 * warunkowej, aby powiadomić wątek konsumenta. Na koniec wątek
 * producenta zwalnia muteks.
 *
 * Wątek konsumenta zajmuje muteks, sprawdza za pomocą zmiennej count,
 * czy bufor jest pusty, i jeśli tak, oczekuje na zmiennej warunkowej
 * consumer_cond. Gdy wartość jest dostępna w buforze, wątek
 * konsumenta pobiera ją, przetwarza wartość i sygnalizuje zmiennej
 * warunkowej producer_cond, aby powiadomić wątek producenta. Na
 * koniec wątek konsumenta zwalnia muteks.
 *
 * Funkcja main tworzy wątki producenta i konsumenta, a następnie
 * oczekuje na ich zakończenie.
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "checks.h"

#define BUFFER_SIZE 10
#define VALUE_COUNT 20

int buffer[BUFFER_SIZE];
int count = 0;

/*
 * Używamy muteksu i dwóch zmiennych warunkowych do synchronizacji
 * dostępu do bufora. Muteks zapewnia, że tylko jeden wątek może
 * jednocześnie uzyskać dostęp do bufora, podczas gdy zmienne
 * warunkowe pozwalają wątkom sygnalizować sobie nawzajem, gdy bufor
 * jest pełny lub pusty.
 *
 * Zdefiniuj mutex i zmienne warunkowe: producer_cond i consumer_cond. */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;


void *producer(void *arg) {
/*
 * Wątek producenta zajmuje muteks, sprawdza za pomocą zmiennej count,
 * czy bufor jest pełny, i jeśli tak, oczekuje na zmiennej warunkowej
 * producer_cond. Gdy w buforze jest miejsce, wątek producenta
 * produkuje wartość, przechowuje ją w buforze i sygnalizuje zmiennej
 * warunkowej, aby powiadomić wątek konsumenta. Na koniec wątek
 * producenta zwalnia muteks.
 */

    /* Przekaż mutex do funkcji check_mutex_producer */
    check_mutex_producer(&mutex);
    /* Przekaż odpowiednią zmienną warunkową do funkcji check_cond_producer */
    check_cond_producer(&producer_cond);

    for (int i = 0; i < VALUE_COUNT; ++i) {
        /* Zajmij mutex */
        pthread_mutex_lock(&mutex);
        /* Czekaj na zmiennej warunkowej tak długo jak: count == BUFFER_SIZE */
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&producer_cond, &mutex);
        }

        buffer[count++] = i;
        /* Zasygnalizuj, że bufor jest pełny */
        pthread_cond_broadcast(&consumer_cond);
        /* Zwolnij mutex */
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}


void *consumer(void *arg) {
/* Wątek konsumenta zajmuje muteks, sprawdza za pomocą zmiennej count,
 * czy bufor jest pusty, i jeśli tak, oczekuje na zmiennej warunkowej
 * consumer_cond. Gdy wartość jest dostępna w buforze, wątek
 * konsumenta pobiera ją, przetwarza wartość i sygnalizuje zmiennej
 * warunkowej producer_cond, aby powiadomić wątek producenta. Na
 * koniec wątek konsumenta zwalnia muteks.
 */

    /* Przekaż mutex do funkcji check_mutex_consumer */
    check_mutex_consumer(&mutex);
    /* Przekaż odpowiednią zmienną warunkową do funkcji
       check_cond_consumer */
    check_cond_consumer(&consumer_cond);

    for (int i = 0; i < VALUE_COUNT; ++i) {
        /* Zajmij mutex */
        pthread_mutex_lock(&mutex);
        /* Czekaj na zmiennej warunkowej tak długo jak: count == 0 */
        while (count == 0) {
            pthread_cond_wait(&consumer_cond, &mutex);
        }

        int value = buffer[--count];
        process_value(value);

        /* Zasygnalizuj, że bufor jest pusty */
        pthread_cond_broadcast(&producer_cond);
        /* Zwolnij mutex */
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}


int main() {
/* Funkcja main tworzy wątki producenta i konsumenta, a następnie
 * oczekuje na ich zakończenie.
 */

    /* Zadeklaruj dwie zmienne typu pthread_t */
    pthread_t producer_thread, consumer_thread;
    /* Stwórz wątek producenta, przekaż mu funkcję producer do wykonywania */
    pthread_create(&producer_thread, NULL, producer, NULL);
    /* Stwórz wątek konsumenta, przekaż mu funkcję consumer do wykonywania */
    pthread_create(&consumer_thread, NULL, consumer, NULL);
    /* Poczekaj na zakończenie działania obu wątków */
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    check_wait();
    check_results();
    return 0;
}
