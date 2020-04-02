#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

char *input_buffer;
char *output_buffer;

int buffer_size;

sem_t inbuffer_mutex, inbuffer_slots;

int producer_position = 0;
int consumer_position = 0;

void *reader_thread(void *arg)
{
	FILE *f = fopen(arg, "r");	

	char c; 

	do{
		c = fgetc(f);
		sem_wait(&inbuffer_slots);
		sem_wait(&inbuffer_unencrypted);
		sem_wait(&inbuffer_mutex);

		input_buffer[producer_position] = c;
		producer_position = (producer_position + 1) % buffer_size;

		sem_post(&inbuffer_mutex);
		sem_post(&inbuffer_slots);
	}while(c != EOF);

	pthread_exit(NULL);
}

void *writer_thread(void *arg)
{

}

void *encryption_thread(void *arg)
{
	char c;
	int s = 1;

	do{

		sem_wait(&inbuffer_mutex);

		c = input_buffer[producer_position];
		producer_position = (producer_position + 1) % buffer_size;

		sem_post(&inbuffer_mutex);

		if((c > 64 && c < 91) || (c > 96 && c < 122)){
			if(s == 1){
				c++;
				if(c == 91) c = 65;
				else if(c == 123) c = 97;
				s = -1;
			} else if(s == -1){
				c--;
				if(c == 64) c = 90;
				else if(c == 96) c = 122; 
				s = 0;
			} else if(s == 0){
				s = 1;
			}
		}

		
		output_buffer[consumer_position] = c;
		consumer_position = (consumer_position + 1) % buffer_size;

	}while(c != EOF);
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
	
	sem_init(&inbuffer_mutex, 0, 1);
	sem_init(&inbuffer_slots, buffer_size);
	sem_init(&inbuffer_unencrypted, buffer_size);

	pthread_t p_reader, p_writer, p_encryption, p_input, p_output;

	pthread_create(&p_reader, NULL, reader_thread, argv[1]); 
	int i;
//	pthread_create(&p_writer, NULL, writer_thread, argv[2]);
//	pthread_create(&p_encryption, NULL, encryption_thread, NULL);
//	pthread_create(&p_input, NULL, input_thread, NULL);
//	pthread_create(&p_output, NULL, output_thread, NULL);
	
	pthread_join(p_reader, NULL);
	for(i=0; i<buffer_size; i++)
	{
		printf("%c", input_buffer[i]);
	}
  	return 0;
}
