#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <signal.h>

FILE *input_file;
sem_t *write_semaphore, *read_semaphore;
int shm_fd;
double *shared_memory;

void catch_SIGNAL(){
    fclose(input_file);// Close input.bin file
    sem_close(write_semaphore); // close write_semaphore
    sem_close(read_semaphore); // close read_semaphore
    sem_unlink("my_write_semaphore"); // delete write_semaphore
    sem_unlink("my_read_semaphore"); // delete read_semaphore
    munmap(shared_memory, sizeof(double)); // unmap shared memory
    close(shm_fd); // close shared memory
    shm_unlink("my_shared_memory.bin"); // delete my_shared_memory.bin

    exit(1);
}

int main(int argc, char *argv[]) {
    double buff_input[1024];
    int i = 0; int n_numbers;

    // Signal Handlers
    signal(SIGHUP, catch_SIGNAL);
    signal(SIGTERM, catch_SIGNAL);
    signal(SIGINT, catch_SIGNAL);
    signal(SIGQUIT, catch_SIGNAL);
    signal(SIGKILL, catch_SIGNAL);

    // open input.asc for reading
    if((input_file = fopen("input.asc", "r")) == NULL){
        perror("input.asc did not opened");
        return 1;
    }
    // Read doubles from the file until end of file (EOF)
    while (fscanf(input_file, "%lf", &buff_input[i]) != EOF) {
    //    printf("%f,", buff_input[i]);
        i++;
    }
    n_numbers = i;
    //printf("\n");
    fclose(input_file);// Close input.bin file

    // Create a shared memory
    shm_fd = shm_open("my_shared_memory.bin", O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1){
        perror("my_shared_memory.bin did not create");
        return 1;
    }
    // Set the size as sizeof(double)
    if(ftruncate(shm_fd, sizeof(double)) == -1){
        perror("my_shared_memory.bin did not unlink");
        return 1;
    }
    // map shared memory
    shared_memory = (double *)mmap(NULL, sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shared_memory == (double *) -1){
        perror("shared_memory did not map");
        return 1;
    }

    //create a write_semaphore and a read_semaphore
    if((write_semaphore = sem_open("my_write_semaphore", O_CREAT, 0666, 0)) == SEM_FAILED){
        perror("my_write_semaphore did not create");
        return 1;
    }
    if((read_semaphore = sem_open("my_read_semaphore", O_CREAT, 0666, 0)) == SEM_FAILED){
        perror("my_read_semaphore did not create");
        return 1;
    }

    // write 1 double to shared memory 
    // and wait for next semaphore to write a new double
    for(i = 0; i < n_numbers; i++){
        sem_wait(write_semaphore); // wait for server to read the last value
        *shared_memory = buff_input[i]; // Write to shared memory
        //printf("%f\n", *shared_memory);
        sem_post(read_semaphore); //Signal server that it can read the next value
    }
    // after sending all doubles, write to shared memory DBL_MIN to finnish communication
    sem_wait(write_semaphore); // wait for server to read the last value
    *shared_memory = DBL_MIN; // Write to shared memory
    //printf("%f\n", *shared_memory);
    sem_post(read_semaphore); //Signal server that it can read the DBL_MIN value

    sem_wait(write_semaphore); // wait for server to read the DBL_MIN value
    // close and delete both semaphores
    sem_close(write_semaphore);
    sem_close(read_semaphore);
    sem_unlink("my_write_semaphore");    
    sem_unlink("my_read_semaphore");    

    // unmap shared memory
    if(munmap(shared_memory, sizeof(double)) == -1){
    perror("shared_memory did not unmap");
        return 1;
    }
    close(shm_fd); // close shared memory
    // delete shared memory
    if(shm_unlink("my_shared_memory.bin") == -1){
    perror("my_shared_memory.bin did not unlink");
        return 1;
    }

    return 0;
}
