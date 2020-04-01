#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

char *input_buffer;
char *output_buffer;

int buffer_size;

sem_t rw_mutex;
int producer_position = 0;
int consumer_position = 0;

void *reader_thread(void *arg)
{
	FILE *f = fopen(arg, "r");	

	char c = getc(f);
	while(c != EOF){
		sem_wait(&rw_mutex);

		input_buffer[producer_position] = c;
		producer_position = (producer_position + 1) % buffer_size;

		sem_post(&rw_mutex);
	}
	pthread_exit(NULL);
}

void *writer_thread(void *arg)
{

}

void *encryption_thread(void *arg)
{

}

void *input_thread(void *arg)
{

}

void *output_thread(void *arg)
{

}

int main(int argc, char* argv[])
{ 
	
	if(argc != 3)
	{
		printf("Usage: ./main [infile] [outfile]\n");	
		return 0;
	}

	printf("Enter a buffer size: ");
	scanf("%d", &buffer_size);

	input_buffer = malloc(buffer_size);
	output_buffer = malloc(buffer_size);
	
	sem_init(&rw_mutex, 0, 1);

	pthread_t p_reader, p_writer, p_encryption, p_input, p_output;

	pthread_create(&p_reader, NULL, reader_thread, argv[1]); 
	pthread_create(&p_writer, NULL, writer_thread, argv[2]);
	pthread_create(&p_encryption, NULL, encryption_thread, NULL);
	pthread_create(&p_input, NULL, input_thread, NULL);
	pthread_create(&p_output, NULL, output_thread, NULL);
	
  	return 0;
}
