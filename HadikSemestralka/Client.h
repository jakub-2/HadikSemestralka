#pragma once

#include "Hadik.h"
#include "Menu.h"
#include "Server.h"

typedef struct server_data
{
	local_client_receive_buffer* receive_buffer;
	local_client_send_buffer* send_buffer;
	int mode, type, timer, width, height, playerCount;
}server_data;

typedef struct inter_thread_buffer
{
	local_client_receive_buffer* receive_buffer;
	local_client_send_buffer* send_buffer;
	pthread_mutex_t lock_inter_client;
	_Bool is_end;
	_Bool pause;
}inter_thread_buffer;

void* send_data(void* data);

void* receive_data(void* data);

void create_session();