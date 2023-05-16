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

sem_t resource_sem;
pthread_mutex_t log_mutex;
pthread_mutex_t barrier_mutex;
pthread_cond_t barrier_cond;
int available_resources = RESOURCE_COUNT;
int thread_count = 0;

void print_log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    pthread_mutex_lock(&log_mutex);
    vprintf(format, args);
    pthread_mutex_unlock(&log_mutex);
    va_end(args);
}

void *thread_function(void *arg) {
    long thread_id = (long)arg;
    print_log("Iniciando thread %ld\n", thread_id);

    // Agregar la sincronizaciÃ³n de barrera alternativa
    pthread_mutex_lock(&barrier_mutex);
    thread_count++;
    if (thread_count == NUM_THREADS) {
        pthread_cond_broadcast(&barrier_cond);
    } else {
        pthread_cond_wait(&barrier_cond, &barrier_mutex);
    }
    pthread_mutex_unlock(&barrier_mutex);

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        sem_wait(&resource_sem);
        print_log("%ld - Semaforo abierto con exito.\n", thread_id);

        pthread_mutex_lock(&log_mutex);
        available_resources--;
        pthread_mutex_unlock(&log_mutex);

        print_log("Iniciando iteracion %d\n", i + 1);
        print_log("%ld - (!) Recurso tomado\n", thread_id);

        int sleep_time = rand() % 3;
        sleep(sleep_time); // Simular trabajo

        print_log("%ld - Buenos dias! Recurso usado\n", thread_id);

        pthread_mutex_lock(&log_mutex);
        available_resources++;
        pthread_mutex_unlock(&log_mutex);

        print_log("%ld - Recurso devuelto :)\n", thread_id);

        sem_post(&resource_sem);
    }
    print_log("%ld - Semaforo muerto\n", thread_id);
    print_log("Terminando thread %ld\n", thread_id);
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t threads[NUM_THREADS];
    sem_init(&resource_sem, 0, RESOURCE_COUNT);
    pthread_mutex_init(&log_mutex, NULL);
    pthread_mutex_init(&barrier_mutex, NULL);
    pthread_cond_init(&barrier_cond, NULL);

    printf("Iniciando programa\n");
    printf("Creando threads\n");

    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_function, (void *)i);
    }

    printf("Esperando threads\n");

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&resource_sem);
    pthread_mutex_destroy(&log_mutex);
    pthread_mutex_destroy(&barrier_mutex);
    pthread_cond_destroy(&barrier_cond);
    return 0;
}
