#pragma once

#include "Hadik.h"


typedef struct inter_buffer
{
	_Bool is_end;
	pthread_mutex_t lock;
}inter_buffer;

typedef struct temp_receive
{
	int width;
	int height;
	Snake** snakes;
	Fruit** fruits;
	inter_buffer* inter_buffer;
}temp_receive;

//TODO poslat input
void* send_data(void* data);

void* receive_data(void* data);

void* send_connection_up(void* data);

_Bool try_connect_server();
void start();