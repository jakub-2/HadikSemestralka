#include "ClientBasic.h"


//#include <sys/ipc.h>
#include <pthread.h>
#include <sys/shm.h>

//TODO create struct for sending and receiving data

void* send_data(void* data)
{
    inter_buffer* buffer = (inter_buffer*)data;

    int shm_size = 5;
    // locate shared memory segment
    int shmid = shmget(69, shm_size, 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory segment to client's address space
    char* data = (char*)shmat(shmid, NULL, 0);
    if (data == (char*)(-1)) {
        perror("shmat");
        exit(1);
    }

    int ch;
    _Bool end;
    while (1) {
        pthread_mutex_lock(&buffer->lock);
        if (buffer->is_end)
        {
            pthread_mutex_unlock(&buffer->lock);
            break;
        }
        pthread_mutex_unlock(&buffer->lock);

        // Get user input
        ch = getch();
        if (ch != ERR)
        {
            if (ch == KEY_UP || ch == KEY_DOWN || ch == KEY_LEFT || ch == KEY_RIGHT) {
                strncpy(data, ch, shm_size);
            }
        }
    }
    shmdt(data);
    return NULL;
}

void* receive_data(void* data)
{
    temp_receive* buff = (temp_receive*)data;
    int shm_size = calculate_buffer_size(buff->width, buff->height, 2, 2);
    // locate shared memory segment
    int shmid = shmget(420, shm_size, 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory segment to client's address space
    char* data = (char*)shmat(shmid, NULL, 0);
    if (data == (char*)(-1)) {
        perror("shmat");
        exit(1);
    }
    Fruit** fruits = malloc(sizeof(Fruit) * 2);
    Snake** snakes = malloc(sizeof(Snake) * 2);

    for (int i = 0; i < 2; ++i) {
        //buff->fruits[i] = malloc(sizeof(Fruit));
        memcpy(buff->fruits[i], fruits[i], sizeof(Fruit));
        memcpy(buff->snakes[i], snakes[i], sizeof(Snake));
    }

    while (1) {
        if (strlen(data) > 0) {
            //printf("Client: %s\n", data);

            if (strcmp(data, "End"))
            {
                pthread_mutex_lock(&buff->inter_buffer->lock);
                buff->inter_buffer->is_end = 1;
                pthread_mutex_unlock(&buff->inter_buffer->lock);
                break;
            }

            deserialize_data(data, fruits, snakes);
            update(fruits, snakes, buff->fruits, buff->snakes);

            for (int i = 0; i < 2; ++i) {
                //buff->fruits[i] = malloc(sizeof(Fruit));
                memcpy(buff->fruits[i], fruits[i], sizeof(Fruit));
                memcpy(buff->snakes[i], snakes[i], sizeof(Snake));
            }

            free_fruits(fruits);
            free_snakes(snakes);
           

            // Respond to the client
            memset(data, 0, shm_size);
        }
    }
    free(fruits);
    free(snakes);
    shmdt(data);
    return NULL;
}

void* send_connection_up(void* data)
{
    inter_buffer* buffer = (inter_buffer*)data;
    int shm_size = 25;

	// locate shared memory segment
    int shmid = shmget(1000, shm_size, 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory segment to client's address space
    char* data = (char*)shmat(shmid, NULL, 0);
    if (data == (char*)(-1)) {
        perror("shmat");
        exit(1);
    }

    struct timespec ts;
    struct tm* timeinfo;


    char message[20] = "Som hore";
    while (1) {
        pthread_mutex_lock(&buffer->lock);
        if (buffer->is_end)
        {
            pthread_mutex_unlock(&buffer->lock);
            break;
        }
        pthread_mutex_unlock(&buffer->lock);

        // Write the timestamp into the shared memory
        snprintf(data, shm_size, "%s", message);

        usleep(2000);
    }
    shmdt(data);

    return NULL;
}

_Bool try_connect_server()
{

    return 1;
}

void start()
{
    //TODO nahadzat thready
}
