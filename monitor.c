#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

#define NUM_THREADS 10
#define NUM_ITERATIONS 5
#define RESOURCE_COUNT 5

sem_t log_sem;
pthread_mutex_t resource_mutex;
pthread_mutex_t barrier_mutex;
pthread_cond_t barrier_cond;
int available_resources = RESOURCE_COUNT;
int thread_count = 0;

void print_log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    sem_wait(&log_sem);
    vprintf(format, args);
    sem_post(&log_sem);
    va_end(args);
}

int decrease_count(int count) {
    pthread_mutex_lock(&resource_mutex);
    if (available_resources < count) {
        pthread_mutex_unlock(&resource_mutex);
        return -1;
    } else {
        available_resources -= count;
        pthread_mutex_unlock(&resource_mutex);
        return 0;
    }
}

int increase_count(int count) {
    pthread_mutex_lock(&resource_mutex);
    available_resources += count;
    pthread_mutex_unlock(&resource_mutex);
    return 0;
}

void *thread_function(void *arg) {
    long thread_id = (long)arg;
    print_log("Thread iniciado %ld\n", thread_id);

    // Agregar la sincronización de barrera alternativa
    pthread_mutex_lock(&barrier_mutex);
    thread_count++;
    if (thread_count == NUM_THREADS) {
        pthread_cond_broadcast(&barrier_cond);
    } else {
        pthread_cond_wait(&barrier_cond, &barrier_mutex);
    }
    pthread_mutex_unlock(&barrier_mutex);

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        while (decrease_count(1) == -1) {
            usleep(1000); // Esperar un tiempo antes de intentar nuevamente
        }
        print_log("Recurso %ld tomado\n", thread_id);

        print_log("Iteracion %d\n", i + 1);

        int sleep_time = rand() % 3;
        sleep(sleep_time); // Simular trabajo

        print_log("Recurso %ld usado\n", thread_id);

        increase_count(1);
        print_log("Recurso %ld devuelto\n", thread_id);
    }
    print_log("Terminando thread %ld\n", thread_id);
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t threads[NUM_THREADS];
    sem_init(&log_sem, 0, 1);
    pthread_mutex_init(&resource_mutex, NULL);
    pthread_mutex_init(&barrier_mutex, NULL);
    pthread_cond_init(&barrier_cond, NULL);

    printf("Iniciando\n");
    printf("Creando threads\n");

    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_function, (void *)i);
    }

    printf("Esperando threads\n");

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&log_sem);
    pthread_mutex_destroy(&resource_mutex);
    pthread_mutex_destroy(&barrier_mutex);
    pthread_cond_destroy(&barrier_cond);
    return 0;
}
