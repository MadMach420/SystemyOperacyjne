#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <locale.h>
#include <signal.h>
#include <pthread.h>

#define WIDTH 30
#define HEIGHT 30

char* current_grid;
char* next_grid;

// ----- GRID UTILS -----
char *create_grid() {
    return malloc(sizeof(char) * WIDTH * HEIGHT);
}


void destroy_grid(char *grid) {
    free(grid);
}


void draw_grid(char *grid) {
    for (int i = 0; i < HEIGHT; ++i) {
        // Two characters for more uniform spaces (vertical vs horizontal)
        for (int j = 0; j < WIDTH; ++j) {
            if (grid[i * WIDTH + j]) {
                mvprintw(i, j * 2, "■");
                mvprintw(i, j * 2 + 1, " ");
            } else {
                mvprintw(i, j * 2, " ");
                mvprintw(i, j * 2 + 1, " ");
            }
        }
    }

    refresh();
}


void init_grid(char *grid) {
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
        grid[i] = rand() % 2 == 0;
}


bool is_alive(int row, int col, char *grid) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) {
                continue;
            }
            int r = row + i;
            int c = col + j;
            if (r < 0 || r >= HEIGHT || c < 0 || c >= WIDTH) {
                continue;
            }
            if (grid[WIDTH * r + c]) {
                count++;
            }
        }
    }

    if (grid[row * WIDTH + col]) {
        if (count == 2 || count == 3) return true;
        else return false;
    }
    else {
        if (count == 3) return true;
        else return false;
    }
}


// ----- HANDLE SIGNALS -----
void handler(int signo) {
    endwin(); // End curses mode
    destroy_grid(current_grid);
    destroy_grid(next_grid);

    exit(0);
}


void handle_alarm(int signo) {}


// ----- GAME -----
void* calculate_in_thread(void* data) {
    signal(SIGALRM, handle_alarm);

    int* int_data = data;
    int start = int_data[0], end = int_data[1];

    while (true) {
        pause();
        for (int i = start; i < end; ++i) {
            int row = i / WIDTH, col = i % WIDTH;
            next_grid[row * WIDTH + col] = is_alive(row, col, current_grid);
        }
    }
}


void init_threads(int no_of_threads, pthread_t threads[]) {
    int args[no_of_threads][2];

    if (no_of_threads == 1) {
        args[0][0] = 0; args[0][1] = WIDTH * HEIGHT;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, calculate_in_thread, args[0]);
        threads[0] = thread_id;
    } else {
        int step = WIDTH * HEIGHT / no_of_threads;

        for (int i = 0; i < no_of_threads - 1; ++i) {
            args[i][0] = i * step;
            args[i][1] = (i + 1) * step;

            pthread_t thread_id;
            pthread_create(&thread_id, NULL, calculate_in_thread, args[i]);
            threads[i] = thread_id;
        }

        args[no_of_threads - 1][0] = (no_of_threads - 1) * step;
        args[no_of_threads - 1][1] = WIDTH * HEIGHT;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, calculate_in_thread, args[no_of_threads - 1]);
        threads[no_of_threads - 1] = thread_id;
    }
}


void start_game(int no_of_threads) {
    pthread_t threads[no_of_threads];

    init_threads(no_of_threads, threads);

    char *tmp;
    init_grid(current_grid);

    while (true)
    {
        draw_grid(current_grid);

        for (int i = 0; i < no_of_threads; ++i) pthread_kill(threads[i], SIGALRM);
        usleep(500 * 1000);

        // Step simulation
        tmp = current_grid;
        current_grid = next_grid;
        next_grid = tmp;
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Invalid number of arguments!\n");
        return 1;
    }
    int no_of_threads = atoi(argv[1]);

    signal(SIGINT, handler);

    srand(time(NULL));
    setlocale(LC_CTYPE, "");
    initscr(); // Start curses mode

    current_grid = create_grid();
    next_grid = create_grid();

    start_game(no_of_threads);

    endwin(); // End curses mode
    destroy_grid(current_grid);
    destroy_grid(next_grid);
    return 0;
}