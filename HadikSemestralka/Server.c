#include "Server.h"

#include <sys/shm.h>

pthread_t player1In_t;
pthread_t player2In_t;
// thread that runs game and send progress
pthread_t send_t; // Send to both at once

// threads for connection check
pthread_t sharedOut_t;
pthread_t sharedIn_t;

local_client_send_buffer* lBufferSend;
local_client_receive_buffer* lBufferReceive;

Snake** snakes;
Fruit** fruits;
char* stringBuffer;

//void* localPlayerInput(void* data)
//{
//	local_client_receive_buffer* buffer = (local_client_receive_buffer*)data;
//	_Bool is_end = 0;
//	while (!is_end)
//	{
//		//ziskam remote input
//		int remoteDirection = 0;
//
//		//zapisem do bufferu pre run hry
//		//locknem pre zapis directionu
//		pthread_mutex_lock(buffer->lock);
//
//		//pokial nebol precitany smer tak cakam
//		while (buffer->direction[0] == -1)
//		{
//			pthread_cond_wait(buffer->read_local, buffer->lock);
//		}
//
//		if (buffer->direction[0] == -2)
//		{
//			break;
//		}
//
//		//smer bol precitany tak zapisem ziskany input
//		buffer->direction[0] = remoteDirection;
//		
//		//triggernem cond ze som odovzdal input
//		pthread_mutex_unlock(buffer->lock);
//
//		//TODO overit ci treba
//		//asi bez signalizacie aby hra nestala
//		//preistotu signal pre dalsi thread ale asi netreba
//		pthread_cond_signal(buffer->read_remote);
//		//repeat?
//	}
//
//	return NULL;
//}

void* remotePlayerInput(void* data)
{
	local_client_receive_buffer* buffer = (local_client_receive_buffer*)data;

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

	_Bool is_end = 0;
	while (!is_end)
	{
		//TODO precitaj shared memory
		//ziskam remote input
		int remoteDirection = 0;

		if (strlen(data) > 0) {

			remoteDirection = atoi(data);

			// Respond to the client
			memset(data, 0, shm_size);
		}


		//zapisem do bufferu pre run hry
		//locknem pre zapis directionu
		pthread_mutex_lock(buffer->lock);

		//pokial nebol precitany smer tak cakam
		while (buffer->direction[1] == -1)
		{
			pthread_cond_wait(buffer->read_remote, buffer->lock);
		}

		if (buffer->direction[1] == -2)
		{
			break;
		}

		//smer bol precitany tak zapisem ziskany input
		buffer->direction[1] = remoteDirection;

		//triggernem cond ze som odovzdal input
		pthread_mutex_unlock(buffer->lock);
		//asi bez signalizacie aby hra nestala
		//preistotu signal pre dalsi thread ale asi netreba
		pthread_cond_signal(buffer->read_local);
		//repeat?
	}
	return NULL;
}

//run game and send data
void* runGame(void* data)
{
	send_buffer* buffer = (send_buffer*)data;

	// locate shared memory segment
	int shmid = shmget(420, buffer->buffer_size, 0666);
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

	while (1)
	{
		//locke buffer s direction
		pthread_mutex_lock(buffer->receive_buffer->lock);
		int direction[2];
		for (int i = 0; i < 2; ++i)
		{
			direction[i] = buffer->receive_buffer->direction[i];
			if (direction[i] == -1)
			{
				direction[i] = snakes[i]->direction;
			}
			direction[i] = -1;
		}
		pthread_mutex_unlock(buffer->receive_buffer->lock);
		pthread_cond_signal(buffer->receive_buffer->read_local);
		pthread_cond_signal(buffer->receive_buffer->read_remote);

		//if zahranie moveu ukonci hru
		if (play(direction, buffer->snakes, buffer->fruits, 0))
		{
			break;
		}

		//send local data
		pthread_mutex_lock(buffer->send_buffer->lock);

		buffer->send_buffer->snakes = snakes;
		buffer->send_buffer->fruits = fruits;

		pthread_mutex_unlock(buffer->send_buffer->lock);
		pthread_cond_signal(buffer->send_buffer->is_New);

		//send shared data
		serialize_game(snakes, fruits, buffer->buffer, buffer->buffer_size);

		strncpy(data, buffer->buffer, buffer->buffer_size - 1);
		//printf("Message sent: %s\n", data);

		usleep(200000);
	}

	//send local data
	pthread_mutex_lock(buffer->send_buffer->lock);

	buffer->send_buffer->snakes = NULL;
	buffer->send_buffer->fruits = NULL;

	pthread_mutex_unlock(buffer->send_buffer->lock);
	pthread_cond_signal(buffer->send_buffer->is_New);

	//send shared data and detach shared memory
	strncpy(data, "End\0", buffer->buffer_size - 1);
	shmdt(data);
}

void* check_connection(void* data)
{
	connected_client* buffer = (connected_client*)data;

	// locate shared memory segment
	int shmid = shmget(1000, 25, 0666);
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

	int counter = 0;
	while (1)
	{
		if (strlen(data) > 0) {
			if (strcmp(data, "End") == 0)
			{
				memset(data, 0, SHM_SIZE);
				break;
			}

			//printf("Read timestamp: %s\n", data);

			// Respond to the client
			memset(data, 0, SHM_SIZE);
			counter = 0;
		}
		else
		{
			if (counter > 10)
			{
				//TODO pausni hru
				break;
			}
			counter++;
		}
		usleep(10000);
	}

	shmdt(data);

	return NULL;
}

void createServer(local_client_send_buffer* client_buffer, local_client_receive_buffer* client_receive_buffer)
{
	//TODO spravit thready

}

void createGameS(int type, int gameMode, int width, int height)
{
	snakes = malloc(sizeof(Snake) * 2);
	fruits = malloc(sizeof(Fruit) * 2);

	createGame(snakes, fruits, 0);
	srand(time(0));

	int buff_size = calculate_buffer_size(WIDTH, HEIGHT, 2, 2);
	stringBuffer = malloc(buff_size);

}

void test()
{
	Snake** snakes = malloc(sizeof(Snake) * 2);
	Fruit** fruits = malloc(sizeof(Fruit) * 2);
	char* buffer;

	_Bool print = 1;
	createGame(snakes, fruits, print);

	int ch = KEY_RIGHT;
	srand(time(0));

	int buff_size = calculate_buffer_size(WIDTH, HEIGHT, 2, 2);
	buffer = malloc(buff_size);

	int temp1 = sizeof(buffer);

	int direction[] = { KEY_RIGHT , KEY_LEFT };
	while (1) {
		// Get user input
		ch = getch();
		if (ch != ERR)
		{
			switch (ch) {
				// Player 1 controls (WASD)
			case 'w': direction[0] = KEY_UP; break;
			case 's': direction[0] = KEY_DOWN; break;
			case 'a': direction[0] = KEY_LEFT; break;
			case 'd': direction[0] = KEY_RIGHT; break;

				// Player 2 controls (Arrow keys)
			case KEY_UP:    direction[1] = KEY_UP; break;
			case KEY_DOWN:  direction[1] = KEY_DOWN; break;
			case KEY_LEFT:  direction[1] = KEY_LEFT; break;
			case KEY_RIGHT: direction[1] = KEY_RIGHT; break;
			}
		}

		if (play(direction, snakes, fruits, print))
		{
			break;
		}

		if (print)
		{
			refresh();
		}


		serialize_game(snakes, fruits, buffer, buff_size);

		usleep(200000);
	}
	endGame(snakes, print);
	for (int i = 0; i < 2; ++i)
	{
		free(fruits[i]);
	}
	free(buffer);
}


