#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "genetic_algorithm.h"
#include <pthread.h>
#include <unistd.h>

int min(int a, int b)
{
    if (a<b)
        return a;
    else
        return b;
}

int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int argc, char *argv[], int *P)
{
	FILE *fp;

	if (argc < 3) {
		fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
		fclose(fp);
		return 0;
	}

	if (*object_count % 10) {
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i) {
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int) strtol(argv[2], NULL, 10);
	*P = (int) strtol(argv[3], NULL, 10);
	
	if (*generations_count == 0) {
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i) {
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i) {
		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(const sack_object *objects, individual *generation, int object_count, int sack_capacity, int thread_id, int P)
{
	int weight;
	int profit;
	int start,end;
	start = thread_id * object_count / P;
	end = min((thread_id + 1) * object_count / P, object_count);
	
	for (int i = start; i < end; ++i) {
		weight = 0;
		profit = 0;

		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			if (generation[i].chromosomes[j]) {
				weight += objects[j].weight;
				profit += objects[j].profit;
			}
		}

		generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
	}
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0) {
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	} else {
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step) {
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
	}
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));

}

void free_generation(individual *generation)
{
	int i;

	for (i = 0; i < generation->chromosome_length; ++i) {
		free(generation[i].chromosomes);
		generation[i].chromosomes = NULL;
		generation[i].fitness = 0;
	}
}

void* run_genetic_algorithm(void *arg)
{
    arguments argum = *(arguments *)arg;
	int start, end, start_even, start_odd;
	int count, cursor;
	individual *tmp = NULL;
	individual aux;

	// set initial generation (composed of object_count individuals with a single item in the sack)
	for (int i = 0; i < argum.object_count; ++i) {
		argum.current_generation[i].fitness = 0;
		argum.current_generation[i].chromosomes = (int*) calloc(argum.object_count, sizeof(int));
		argum.current_generation[i].chromosomes[i] = 1;
		argum.current_generation[i].index = i;
		argum.current_generation[i].chromosome_length = argum.object_count;

		argum.next_generation[i].fitness = 0;
		argum.next_generation[i].chromosomes = (int*) calloc(argum.object_count, sizeof(int));
		argum.next_generation[i].index = i;
		argum.next_generation[i].chromosome_length = argum.object_count;
	}

	// iterate for each generation
	for (int k = 0; k < argum.generations_count; ++k) {
		cursor = 0;

	// compute fitness and sort by it
	compute_fitness_function(argum.objects, argum.current_generation, argum.object_count, argum.sack_capacity, argum.thread_id, argum.threads);
    pthread_barrier_wait(argum.Bar);
        
	//oets

	start = argum.thread_id * argum.object_count / argum.threads;

    if(start%2==0)
      {
          start_even = start;
          start_odd = start +1;
    } else
    {
        start_odd = start;
        start_even = start +1;
    }

    end = min((argum.thread_id + 1) * (double)argum.object_count / argum.threads, argum.object_count-1);

    for(int i=0; i<argum.object_count; i++)
        {
            for(int k=start_even; k<end; k+=2)
            {
             if( argum.current_generation[k].fitness<argum.current_generation[k+1].fitness)
                {
                    aux = argum.current_generation[k];
                    argum.current_generation[k]=argum.current_generation[k+1];
                    argum.current_generation[k+1]=aux;
                }   
            }
            pthread_barrier_wait(argum.Bar);
            for(int k=start_odd; k<end; k+=2)
            {
             if( argum.current_generation[k].fitness<argum.current_generation[k+1].fitness)
                {
                    aux = argum.current_generation[k];
                    argum.current_generation[k]=argum.current_generation[k+1];
                    argum.current_generation[k+1]=aux;
                }   
            }
            pthread_barrier_wait(argum.Bar);
        }

	// keep first 30% children (elite children selection)
	count = argum.object_count * 3 / 10;
	start = argum.thread_id * count / argum.threads;
	end = min((argum.thread_id + 1) * count / argum.threads, count);
	for (int i = start; i < end; ++i) {
		copy_individual(argum.current_generation + i, argum.next_generation + i);
	}
	cursor = count;
	pthread_barrier_wait(argum.Bar);

	// mutate first 20% children with the first version of bit string mutation
	count = argum.object_count * 2 / 10;
	start = argum.thread_id * count / argum.threads;
	end = min((argum.thread_id + 1) * count / argum.threads, count);
	for (int i = start; i < end; ++i) {
		copy_individual(argum.current_generation + i, argum.next_generation + cursor + i);
		mutate_bit_string_1(argum.next_generation + cursor + i, k);
	}
	cursor += count;
	pthread_barrier_wait(argum.Bar);

	// mutate next 20% children with the second version of bit string mutation
	count = argum.object_count * 2 / 10;
	start = argum.thread_id * count / argum.threads;
	end = min((argum.thread_id + 1) * count / argum.threads, count);
	for (int i = start; i < end; ++i) {
		copy_individual(argum.current_generation + i + count, argum.next_generation + cursor + i);
		mutate_bit_string_2(argum.next_generation + cursor + i, k);
	}
	cursor += count;
	pthread_barrier_wait(argum.Bar);

	// crossover first 30% parents with one-point crossover
	// (if there is an odd number of parents, the last one is kept as such)
	count = argum.object_count * 3 / 10;

	if (count % 2 == 1) {
		copy_individual(argum.current_generation + argum.object_count - 1, argum.next_generation + cursor + count - 1);
		count--;
	}

	start = argum.thread_id * count / argum.threads;
	end = min((argum.thread_id + 1) * count / argum.threads, count);

	for (int i = start; i < end; i += 2) {
		crossover(argum.current_generation + i, argum.next_generation + cursor + i, k);
	}
	pthread_barrier_wait(argum.Bar);

	// switch to new generation
	tmp = argum.current_generation;
	argum.current_generation = argum.next_generation;
	argum.next_generation = tmp;

	start = argum.thread_id * argum.object_count / argum.threads;
	end = min((argum.thread_id + 1) * argum.object_count / argum.threads, argum.object_count);
	for (int i = start; i < end; ++i) {
		argum.current_generation[i].index = i;
	}

	if (k % 5 == 0) {
		if(argum.thread_id == 0){
			print_best_fitness(argum.current_generation);
		}
	}
	}

	compute_fitness_function(argum.objects, argum.current_generation, argum.object_count, argum.sack_capacity, argum.thread_id, argum.threads);
    pthread_barrier_wait(argum.Bar);

	//oets
    
	start = argum.thread_id * argum.object_count / argum.threads;

    if(start%2==0)
      {
          start_even = start;
          start_odd = start +1;
    } else
    {
        start_odd = start;
        start_even = start +1;
    }

    end = min((argum.thread_id + 1) * (double)argum.object_count / argum.threads, argum.object_count-1);

    for(int i=0; i<argum.object_count; i++)
        {
            for(int k=start_even; k<end; k+=2)
            {
             if( argum.current_generation[k].fitness<argum.current_generation[k+1].fitness)
                {
                    aux = argum.current_generation[k];
                    argum.current_generation[k]=argum.current_generation[k+1];
                    argum.current_generation[k+1]=aux;
                }   
            }
            pthread_barrier_wait(argum.Bar);
            for(int k=start_odd; k<end; k+=2)
            {
             if( argum.current_generation[k].fitness<argum.current_generation[k+1].fitness)
                {
                    aux = argum.current_generation[k];
                    argum.current_generation[k]=argum.current_generation[k+1];
                    argum.current_generation[k+1]=aux;
                }   
            }
            pthread_barrier_wait(argum.Bar);
        }

	if(argum.thread_id == 0){
		print_best_fitness(argum.current_generation);
	}

return NULL;
}