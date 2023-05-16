#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

// Variables globales
int recursos;
int iteraciones;
sem_t semaforo_recursos;

// Método que ejecutarán los threads
void *consumir_recurso(void *arg) {
    int cantidad_recursos = *((int *) arg);
    for (int i = 0; i < iteraciones; i++) {
        for (int j = 0; j < cantidad_recursos; j++) {
            sem_wait(&semaforo_recursos); // Decrementa la cantidad de recursos disponibles
        }
        printf("Thread %lu consume %d recursos. Recursos restantes: %d\n", pthread_self(), cantidad_recursos, recursos - cantidad_recursos);
        int tiempo_espera = rand() % 3 + 1;
        sleep(tiempo_espera);
        printf("Thread %lu devuelve %d recursos tras %d segundos\n", pthread_self(), cantidad_recursos, tiempo_espera);
        for (int j = 0; j < cantidad_recursos; j++) {
            sem_post(&semaforo_recursos); // Incrementa la cantidad de recursos disponibles
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s [numero_de_threads] [iteraciones_por_thread] [recursos_por_thread]\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    iteraciones = atoi(argv[2]);
    int recursos_por_thread = atoi(argv[3]);
    recursos = num_threads * recursos_por_thread;
    pthread_t threads[num_threads];

    srand(time(NULL));
    sem_init(&semaforo_recursos, 0, recursos);

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, consumir_recurso, (void *)&recursos_por_thread);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&semaforo_recursos);

    return 0;
}
