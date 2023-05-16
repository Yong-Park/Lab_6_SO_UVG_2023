#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

#define NUM_THREADS 10
#define NUM_ITERATIONS 5
#define RESOURCE_COUNT 10

pthread_mutex_t resource_mutex;
pthread_mutex_t log_mutex;
pthread_cond_t resource_cond;
int available_resources = RESOURCE_COUNT;

void print_log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    pthread_mutex_lock(&log_mutex);
    vprintf(format, args);
    pthread_mutex_unlock(&log_mutex);
    va_end(args);
}

int decrease_count(int count) {
    print_log("Iniciando decrease_count\n");
    pthread_mutex_lock(&resource_mutex);
    print_log("Mutex adquirido, entrando al monitor\n");
    while (available_resources < count) {
        pthread_cond_wait(&resource_cond, &resource_mutex);
    }
    print_log("Recursos suficientes disponibles, consumiendo...\n");
    available_resources -= count;
    pthread_mutex_unlock(&resource_mutex);
    print_log("Terminando decrease_count\n");
    return 0;
}

int increase_count(int count) {
    print_log("Iniciando increase_count\n");
    pthread_mutex_lock(&resource_mutex);
    print_log("Mutex adquirido, entrando al monitor\n");
    available_resources += count;
    pthread_cond_broadcast(&resource_cond);
    pthread_mutex_unlock(&resource_mutex);
    print_log("Mutex liberado\n");
    print_log("Terminando increase_count\n");
    return 0;
}

void *thread_function(void *arg) {
    long thread_id = (long)arg;
    print_log("Iniciando thread %ld\n", thread_id);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        print_log("Iniciando iteracion %d\n", i + 1);
        int resources_needed = rand() % RESOURCE_COUNT + 1;
        print_log("Se consumiran %d recursos\n", resources_needed);
        decrease_count(resources_needed);
        print_log("%ld - (!) Recurso tomado\n", thread_id);

        int sleep_time = rand() % 3;
        sleep(sleep_time); // Simular trabajo

        print_log("%ld - Buenos dias! Recurso usado\n", thread_id);
        increase_count(resources_needed);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t threads[NUM_THREADS];
    pthread_mutex_init(&resource_mutex, NULL);
    pthread_mutex_init(&log_mutex, NULL);
    pthread_cond_init(&resource_cond, NULL);

    print_log("Iniciando programa\n");
    print_log("Creando threads\n");

    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_function, (void *)i);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&resource_mutex);
    pthread_mutex_destroy(&log_mutex);
    pthread_cond_destroy(&resource_cond);
    return 0;
}
