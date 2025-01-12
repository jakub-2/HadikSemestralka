#pragma once

#include "Hadik.h"


typedef struct inter_buffer
{
	_Bool is_end;
	_Bool pause;
	//_B
	pthread_mutex_t lock;
}inter_buffer;

typedef struct temp_receive
{
	GameData* game_data;
	inter_buffer* inter_buffer;
}temp_receive;

//TODO poslat input
void* send_data_basic(void* datas);

void* receive_data_basic(void* datas);

void* send_connection_up(void* datas);

_Bool try_connect_server();
void start();