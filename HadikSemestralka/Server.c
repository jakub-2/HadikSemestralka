#include "Server.h"


void* remotePlayerInput(void* datas)
{
	local_client_receive_buffer* buffer = (local_client_receive_buffer*)datas;

	int shm_size = 5;
	// locate shared memory segment
	int shmid = shmget(69, shm_size, IPC_CREAT | 0666);
	if (shmid == -1) {
		perror("shmget_remote");
		exit(1);
	}

	// attach shared memory segment to client's address space
	char* data = (char*)shmat(shmid, NULL, 0);
	if (data == (char*)(-1)) {
		perror("shmat_remote");
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
		//pthread_cond_signal(buffer->read_local);
		//repeat?
	}
	return NULL;
}

//run game and send data
void* runGame(void* datas)
{
	run_game_buffer* buffer = (run_game_buffer*)datas;

	// locate shared memory segment
	//int shmid = shmget(420, buffer->buffer_size, 0666);
	int shmid = shmget(420, 2048, IPC_CREAT | 0666);
	if (shmid == -1) {
		perror("shmget_server");
		exit(1);
	}

	// attach shared memory segment to client's address space
	char* data = (char*)shmat(shmid, NULL, 0);
	if (data == (char*)(-1)) {
		perror("shmat_server");
		exit(1);
	}


	//TODO posli info o hre (sirka, dlzka, typ, mapa, mod, timer) a cakaj na connection hraca (buffer->connection...)


	while (1)
	{
		//locke buffer s direction
		pthread_mutex_lock(buffer->receive_buffer->lock);
		int direction[2];
		for (int i = 0; i < 2; ++i)
		{
			direction[i] = buffer->receive_buffer->direction[i];
			//if (direction[i] == -1)
			//{
			//	direction[i] = buffer->game_data->snakes[i]->direction;
			//}
			buffer->receive_buffer->direction[i] = -1;
		}
		pthread_mutex_unlock(buffer->receive_buffer->lock);
		pthread_cond_signal(buffer->receive_buffer->read_remote);

		//if zahranie moveu ukonci hru
		if (play(direction, buffer->game_data, 0))
		{
			break;
		}

		//send local data
		pthread_mutex_lock(buffer->send_buffer->lock);

		buffer->send_buffer->game_data = buffer->game_data;

		pthread_mutex_unlock(buffer->send_buffer->lock);
		pthread_cond_signal(buffer->send_buffer->is_New);
		pthread_cond_signal(buffer->receive_buffer->read_local);

		//send shared data
		//serialize_game(snakes, fruits, buffer->buffer, buffer->buffer_size);
		serialize_game_data(buffer->game_data, buffer->buffer, buffer->buffer_size);

		strncpy(data, buffer->buffer, buffer->buffer_size - 1);
		//printf("Message sent: %s\n", data);

		usleep(200000);
	}

	//send local data
	pthread_mutex_lock(buffer->send_buffer->lock);

	buffer->send_buffer->game_data = NULL;

	pthread_mutex_unlock(buffer->send_buffer->lock);
	pthread_cond_signal(buffer->send_buffer->is_New);

	//send shared data and detach shared memory
	strncpy(data, "End\0", buffer->buffer_size - 1);
	shmdt(data);
}

void* check_connection(void* datas)
{
	connected_client* buffer = (connected_client*)datas;

	int SHM_SIZE = 25;
	// locate shared memory segment
	int shmid = shmget(1000, SHM_SIZE, IPC_CREAT | 0666);
	if (shmid == -1) {
		perror("shmget_connection");
		exit(1);
	}

	// attach shared memory segment to client's address space
	char* data = (char*)shmat(shmid, NULL, 0);
	if (data == (char*)(-1)) {
		perror("shmat_connection");
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

void createServer(run_game_buffer* buffer)
{
	pthread_t run_t;
	pthread_t connection_check_t;
	pthread_t remote_input_t;

	pthread_create(&run_t, NULL, runGame, buffer);
	pthread_create(&connection_check_t, NULL, check_connection, buffer->connection_buffer);
	pthread_create(&remote_input_t, NULL, remotePlayerInput, buffer->receive_buffer);

	pthread_join(run_t, NULL);
	pthread_join(connection_check_t, NULL);
	pthread_join(remote_input_t, NULL);

}


void createGameS(int type, int mode, int width, int height, int timer, local_client_send_buffer* client_buffer, local_client_receive_buffer* client_receive_buffer)
{
	GameData* game_data = malloc(sizeof(GameData));
	game_data->width = width;
	game_data->height = height;
	game_data->type = type;
	game_data->mode = mode;
	game_data->timer = timer;
	game_data->count_free_spaces = 20;

	createGame(game_data, 0);
	srand(time(0));

	int buff_size = calculate_buffer_size(game_data->width, game_data->height, 2, 2);
	char* stringBuffer = malloc(buff_size);

	run_game_buffer* buffer = malloc(sizeof(run_game_buffer));
	//malloc na buffer pre connection hraca
	buffer->connection_buffer = malloc(sizeof(connected_client));
	buffer->connection_buffer->lock = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(buffer->connection_buffer->lock, NULL);
	buffer->receive_buffer = client_receive_buffer;
	buffer->send_buffer = client_buffer;
	buffer->buffer = stringBuffer;
	buffer->buffer_size = buff_size;
	buffer->game_data = game_data;

	createServer(buffer);
}

void test()
{
	GameData* game_data = malloc(sizeof(GameData));
	game_data->snakes = malloc(sizeof(Snake) * 2);
	game_data->fruits = malloc(sizeof(Fruit) * 2);
	game_data->width = 10;
	game_data->height = 10;
	game_data->mode = 0;
	game_data->type = 1;
	game_data->timer = 0;
	game_data->count_free_spaces = game_data->width * game_data->height;

	char* buffer;

	_Bool print = 1;
	createGame(game_data, print);

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

		if (play(direction, game_data, print))
		{
			break;
		}

		if (print)
		{
			refresh();
		}


		serialize_game_data(game_data, buffer, buff_size);

		usleep(200000);
	}
	endGame(game_data->snakes, print);
	for (int i = 0; i < 2; ++i)
	{
		free(game_data->fruits[i]);
	}
	free(buffer);
}


