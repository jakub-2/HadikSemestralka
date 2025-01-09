#pragma once

#include "Hadik.h"
#include "Server.h"

typedef struct server_data
{
	local_client_receive_buffer* receive_buffer;
	local_client_send_buffer* send_buffer;
}server_data;


void* send_data(void* data);

void* receive_data(void* data);

void create_session();