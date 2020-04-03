#include <stdio.h> 
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

int inbuffer_producer = 0;
int inbuffer_consumer = 0;

int outbuffer_producer = 0;
int outbuffer_consumer = 0;

int encrypt_position = 0;


int buffer_size = 0;

char *input_buffer;
char *output_buffer;

//Semaphore descriptions in README
sem_t inbuffer_used_slots, inbuffer_open_slots, inbuffer_mutex, encrypted, unencrypted, outbuffer_mutex, outbuffer_used_slots, outbuffer_open_slots, outbuffer_count, outbuffer_uncounted;

//This function reads from the input file one character at a time, and uses that character to produce a character that fills a slot in the input_buffer
void *reader_thread(void *arg)
{
  	FILE *f = fopen(arg, "r");

  	char c;
 
  	do{
    		c = fgetc(f);
    
    		sem_wait(&encrypted);
    		sem_wait(&inbuffer_open_slots);
    		sem_wait(&inbuffer_mutex);

		//Critical section to produce a character for the input buffer
    		input_buffer[inbuffer_producer] = c;
    		inbuffer_producer = (inbuffer_producer + 1) % buffer_size;
    
    		sem_post(&inbuffer_mutex);
    		sem_post(&inbuffer_used_slots);
    		sem_post(&unencrypted);
  	}while(c != EOF);

  	pthread_exit(NULL);
}

//This function writes the encrypted characters from the output_buffer to the output file
void *writer_thread(void *arg)
{
  	FILE *f = fopen(arg, "w");
  	char c;
  	while(c != EOF){
    		sem_wait(&outbuffer_used_slots);
    		sem_wait(&outbuffer_count);
    		sem_wait(&outbuffer_mutex);

		//Critical section to take a character from the output_buffer
    		c = output_buffer[outbuffer_consumer];
    		outbuffer_consumer = (outbuffer_consumer + 1) % buffer_size;
    
    		sem_post(&outbuffer_mutex);
    		sem_post(&outbuffer_uncounted);
    		sem_post(&outbuffer_open_slots);
    
		//if the character is not EOF, write it to the output file
    		if(c != EOF) fputc(c, f);
  	}
  	pthread_exit(NULL);
}

//This function consumes a character from the input_buffer, encrypts it using the algorithm provided, and produces it for the output_buffer to use.
void *encryption_thread(void *arg)
{
  	char c;
  	int s = 1;
  	do{
    		sem_wait(&unencrypted);
    		sem_wait(&inbuffer_mutex);

		//Critical section to get a character out of the input buffer so it can later be encrypted
    		c = input_buffer[encrypt_position];
    		encrypt_position = (encrypt_position + 1) % buffer_size;

    		sem_post(&inbuffer_mutex);
    		sem_post(&encrypted);

		//Encryption algorithm
   		if((c > 64 && c < 91) || (c > 96 && c < 123))
		{
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
    
    		sem_wait(&outbuffer_open_slots);
    		sem_wait(&outbuffer_mutex);

		//Critical section to place the produced encrypted characrter into the output buffer
    		output_buffer[outbuffer_producer] = c;
    		outbuffer_producer = (outbuffer_producer + 1) % buffer_size;

    		sem_post(&outbuffer_mutex);
    		sem_post(&outbuffer_used_slots);
  	}while(c != EOF);

  	pthread_exit(NULL);
}

//This function is responsible for counting each occurance of each character in the input file
void *input_thread(void *arg)
{

  	char c;
  	char alphabet[26][1];

  	int i;
  	for(i = 0; i < 26; i++){
    		alphabet[i][0] = 0;
  	}
  
  	do{
    		sem_wait(&inbuffer_used_slots);
    		sem_wait(&inbuffer_mutex);
		
		//critical section to grab character that is going to be counted
    		c = input_buffer[inbuffer_consumer];
    		inbuffer_consumer = (inbuffer_consumer + 1) % buffer_size;

    		sem_post(&inbuffer_mutex);
    		sem_post(&inbuffer_open_slots);
    
		//If the character is a lowercase character, convert it to uppercase	

    		if((c > 64 && c < 91) || (c > 96 && c < 123)){
			if(c > 96 && c < 123) c = c - 32;

      			alphabet[c - 65][0]++;
    		}
  	}while(c != EOF);

  	printf("Input file contains:\n");
  	for(i = 0; i < 26; i++){
    		if(alphabet[i][0] != 0){
      			printf("%c: %d\n", i + 65, alphabet[i][0]);
    		}
  	}
  
  	pthread_exit(NULL);

}

//This function is responsible for counting each occurance of each character in the input file
void *output_thread(void *arg)
{

  	char c;
  	char alphabet[26][1];
	int count_position = 0;

  	int i;
  	for(i = 0; i < 26; i++){
    		alphabet[i][0] = 0;
  	}
  
  	do{
    		sem_wait(&outbuffer_uncounted);
    		sem_wait(&outbuffer_mutex);

		//critical section to grab a character out of the output_buffer to count
    		c = output_buffer[count_position];
    		count_position = (count_position + 1) % buffer_size;

    		sem_post(&outbuffer_mutex);
    		sem_post(&outbuffer_count);

    		if((c > 64 && c < 91) || (c > 96 && c < 123)){
			if(c > 96 && c < 123) c = c - 32;	
      			alphabet[c - 65][0]++;
    		}
  	}while(c != EOF);

  	printf("Output file contains:\n");

  	for(i = 0; i < 26; i++){
    		if(alphabet[i][0] != 0){
      			printf("%c: %d\n", i + 65, alphabet[i][0]);
    		}
  	}
  	pthread_exit(NULL);
} 





int main(int argc, char* argv[])
{

  	if(argc != 3){
    		printf("Usage ./main [inputfile] [outputfile]\n");
      		return -1;
  	}

  	printf("Enter the buffer size: ");
  	scanf("%d", &buffer_size);
  
  	input_buffer = malloc(buffer_size);
  	output_buffer = malloc(buffer_size);

  	sem_init(&inbuffer_mutex, 0, 1);
  	sem_init(&inbuffer_used_slots, 0, 0);
  	sem_init(&inbuffer_open_slots, 0, buffer_size);

  	sem_init(&unencrypted, 0, 0);
  	sem_init(&encrypted, 0, buffer_size);

  	sem_init(&outbuffer_mutex, 0, 1);
  	sem_init(&outbuffer_used_slots, 0, 0);
  	sem_init(&outbuffer_open_slots, 0, buffer_size);

  	sem_init(&outbuffer_count, 0, 0);
  	sem_init(&outbuffer_uncounted, 0, buffer_size);
  
  	pthread_t p_reader, p_writer, p_encryption, p_input, p_output;

  	pthread_create(&p_reader, NULL, reader_thread, argv[1]);
  	pthread_create(&p_writer, NULL, writer_thread, argv[2]);
  	pthread_create(&p_encryption, NULL, encryption_thread, NULL);
  	pthread_create(&p_input, NULL, input_thread, NULL);
  	pthread_create(&p_output, NULL, output_thread, NULL);

  	pthread_join(p_reader, NULL);
  	pthread_join(p_writer, NULL);
  	pthread_join(p_encryption, NULL);
  	pthread_join(p_input, NULL);
  	pthread_join(p_output, NULL);

	return 0;
}
