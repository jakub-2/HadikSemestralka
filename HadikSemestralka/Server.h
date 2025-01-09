#pragma once

#include <pthread.h>
#include "Hadik.h"

typedef struct local_client_send_buffer
{
	Snake** snakes;
	Fruit** fruits;
	pthread_mutex_t* lock;
	pthread_cond_t* is_New;
	pthread_cond_t* is_Read;
} local_client_send_buffer;

typedef struct local_client_receive_buffer
{
	int* direction;
	pthread_mutex_t* lock;
	pthread_cond_t* read_local;
	pthread_cond_t* read_remote;
	_Bool is_end;
}local_client_receive_buffer;

typedef struct connected_client
{
	pthread_mutex_t lock;
	_Bool is_connected;
}connected_client;

typedef struct send_buffer
{
	Snake** snakes;
	Fruit** fruits;
	char* buffer;
	int buffer_size;
	local_client_send_buffer* send_buffer;
	local_client_receive_buffer* receive_buffer;
}send_buffer;

void* remotePlayerInput(void* data);
//void* localPlayerInput(void* data);
void* runGame(void* data);

void* check_connection(void* data);

void createServer(local_client_send_buffer* client_buffer, local_client_receive_buffer* client_receive_buffer);

void createGameS(int type, int gameMode, int width, int height);

//void sendGameStatus();
//void makeMove();

void test();