#pragma once

#include <pthread.h>
#include "Hadik.h"
#include <sys/shm.h>

// buffer for sending gameData to local client
typedef struct local_client_send_buffer
{
	GameData* game_data;
	pthread_mutex_t* lock;
	pthread_cond_t* is_New;
	pthread_cond_t* is_Read;
} local_client_send_buffer;

// buffer for receiving local client input
typedef struct local_client_receive_buffer
{
	int* direction;
	pthread_mutex_t* lock;
	pthread_cond_t* read_local;
	pthread_cond_t* read_remote;
	_Bool is_end;
}local_client_receive_buffer;

// check if client is still connected to server buffer
typedef struct connected_client
{
	pthread_mutex_t* lock;
	pthread_cond_t* conn_wait;
	_Bool is_connected;
	_Bool reconnected;
	_Bool is_end;
}connected_client;

// buffer for runGame thread
typedef struct run_game_buffer
{
	GameData* game_data;
	char* buffer;
	int buffer_size;
	local_client_send_buffer* send_buffer;
	local_client_receive_buffer* receive_buffer;
	connected_client* connection_buffer;
}run_game_buffer;

void* remotePlayerInput(void* data);
//void* localPlayerInput(void* data);
void* runGame(void* data);

void* check_connection(void* data);

void createServer(run_game_buffer* buffer);
//void createServer(local_client_send_buffer* send_buffer, local_client_receive_buffer* receive_buffer);

void createGameS(int type, int mode, int width, int height, int timer, int playerCount, local_client_send_buffer* client_buffer, local_client_receive_buffer* client_receive_buffer);

//void sendGameStatus();
//void makeMove();

void test();