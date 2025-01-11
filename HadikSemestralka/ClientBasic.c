#include "ClientBasic.h"

//#include <sys/ipc.h>
#include <pthread.h>
#include <sys/shm.h>

//TODO create struct for sending and receiving data


void* send_data_basic(void* datas)
{
    inter_buffer* buffer = (inter_buffer*)datas;

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
    initscr();
    keypad(stdscr, TRUE); // Enable special keys
    noecho();             // Disable character echo
    timeout(0);
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
                //strncpy(data, ch, shm_size);
                char ch_str[5];
                sprintf(ch_str, "%d\0", ch);
            	strncpy(data, ch_str, shm_size);
            }
        }
        usleep(20000);
    }
    shmdt(data);
    return NULL;
}

void* receive_data_basic(void* datas)
{
    temp_receive* buff = (temp_receive*)datas;
    //int shm_size = calculate_buffer_size(buff->width, buff->height, 2, 2);
    int shm_size = 2048;
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
    // reset data just to be sure
    memset(data, 0, shm_size);
    //Fruit** fruits = malloc(sizeof(Fruit) * 2);
    //Snake** snakes = malloc(sizeof(Snake) * 2);

    //for (int i = 0; i < 2; ++i) {
    //    //buff->fruits[i] = malloc(sizeof(Fruit));
    //    memcpy(buff->fruits[i], fruits[i], sizeof(Fruit));
    //    memcpy(buff->snakes[i], snakes[i], sizeof(Snake));
    //}

    // pomocne premenne
    Fruit** fruits = malloc(sizeof(Fruit) * 2);
    Snake** snakes = malloc(sizeof(Snake) * 2);
    _Bool drawn_border = 0;

    initscr();
    keypad(stdscr, TRUE);
    noecho();
    timeout(0);
    curs_set(0);

    clear();
    while (1) {
        if (strlen(data) > 0) {
            //printf("Client: %s\n", data);
            int temp = strcmp(data, "End");
            if (strcmp(data, "End") == 0)
            {
                pthread_mutex_lock(&buff->inter_buffer->lock);
                buff->inter_buffer->is_end = 1;
                pthread_mutex_unlock(&buff->inter_buffer->lock);
                break;
            }

            //deserialize_data(data, fruits, snakes);
            deserialize_game_data(data, buff->game_data);
            //update(fruits, snakes, buff->fruits, buff->snakes);

            _Bool tests = buff->game_data->snakes[1]->isDead;
            if (snakes[0] == NULL)
            {
                update(buff->game_data->fruits, buff->game_data->snakes, buff->game_data);
            }
            else
            {
                update(fruits, snakes, buff->game_data);
            }
            if (!drawn_border)
            {
                draw_map(buff->game_data);
            }

            free_snakes(snakes);
            for (int i = 0; i < 2; ++i)
            {
                snakes[i] = copySnake(buff->game_data->snakes[i]);
            }
            for (int i = 0; i < 2; ++i)
            {
                free(fruits[i]);
                fruits[i] = copy_fruits(buff->game_data->fruits[i]);
            }

            //for (int i = 0; i < 2; ++i) {
            //    //buff->fruits[i] = malloc(sizeof(Fruit));
            //    memcpy(buff->fruits[i], fruits[i], sizeof(Fruit));
            //    memcpy(buff->snakes[i], snakes[i], sizeof(Snake));
            //}

            //free_fruits(fruits);
            //free_snakes(snakes);
           

            // erase memory
            memset(data, 0, shm_size);
        }
        usleep(20000);
    }
    free_fruits(fruits);
    free_snakes(snakes);
    free(fruits);
    free(snakes);
    shmdt(data);

    clear();
    endwin();
    return NULL;
}

void* send_connection_up(void* datas)
{
    inter_buffer* buffer = (inter_buffer*)datas;
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
    pthread_t test_t, send_t;
    temp_receive* test = malloc(sizeof(temp_receive));
    test->game_data = malloc(sizeof(GameData));
    test->game_data->snakes = malloc(sizeof(Snake) * 2);
    test->game_data->fruits = malloc(sizeof(Fruit) * 2);
    test->inter_buffer = malloc(sizeof(inter_buffer));
    //test->inter_buffer->lock

    pthread_create(&test_t, NULL, receive_data_basic, test);
    pthread_create(&send_t, NULL, send_data_basic, test->inter_buffer);
    //TODO nahadzat thready
    pthread_join(test_t, NULL);
    pthread_join(send_t, NULL);

    printf("Game ended with results:\n");
    for (int i = 0; i < 2; ++i)
    {
        printf("Score Snake %c: %d\n", test->game_data->snakes[i]->idChar, test->game_data->snakes[i]->score);
    }
}
