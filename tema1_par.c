#include <stdlib.h>
#include <pthread.h>
#include "genetic_algorithm.h"
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	// array with all the objects that can be placed in the sack
	sack_object *objects = NULL;

	// number of objects
	int object_count = 0;

	// maximum weight that can be carried in the sack
	int sack_capacity = 0;

	// number of generations
	int generations_count = 0;

    int P;
    pthread_barrier_t Bar;
  	int r;
  	int i;
  	void *status;

    if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, argc, argv, &P)) {
		return 0;
	}

    individual *current_generation = (individual*) calloc(object_count, sizeof(individual));
	individual *next_generation = (individual*) calloc(object_count, sizeof(individual));
    pthread_t threads[P];
    arguments argum[P];
    pthread_barrier_init(&Bar,NULL,P);

  	for (i = 0; i < P; i++) {
        argum[i].objects = objects;
        argum[i].object_count = object_count;
        argum[i].sack_capacity = sack_capacity;
        argum[i].generations_count = generations_count;
        argum[i].thread_id = i;
        argum[i].threads = P;
        argum[i].Bar = &Bar;
        argum[i].current_generation = current_generation;
        argum[i].next_generation = next_generation;
		r = pthread_create(&threads[i], NULL, run_genetic_algorithm, &argum[i]);

		if (r) {
	  		printf("Eroare la crearea thread-ului %d\n", i);
	  		exit(-1);
		}
  	}

  	for (i = 0; i < P; i++) {
		r = pthread_join(threads[i], &status);

		if (r) {
	  		printf("Eroare la asteptarea thread-ului %d\n", i);
	  		exit(-1);
		}
  	}

	free(objects);
// free resources for old generation
	free_generation(current_generation);
	free_generation(next_generation);

	// free resources
	free(current_generation);
	free(next_generation);
    pthread_barrier_destroy(&Bar);

	return 0;
}