#include "Client.h"

void* send_data(void* data)
{
	local_client_receive_buffer* buffer = (local_client_receive_buffer*)data;
    int ch;
    _Bool end;
    while (1) {
        pthread_mutex_lock(buffer->lock);
        if (buffer->is_end)
        {
            pthread_mutex_unlock(buffer->lock);
            break;
        }
        pthread_mutex_unlock(buffer->lock);

        // TODO add wait for received data
        //pthread_mutex_lock(buffer->lock);
        //while (buffer->direction[0] == -1)
        //{
            //pthread_cond_wait(buffer->read_local, buffer->lock);
        //}

        //sleep(10);
        // Get user input
        ch = getch();
        if (ch != ERR)
        {
            if (ch == KEY_UP || ch == KEY_DOWN || ch == KEY_LEFT || ch == KEY_RIGHT) {
                pthread_mutex_lock(buffer->lock);

                buffer->direction[0] = ch;

                pthread_mutex_unlock(buffer->lock);
            }
        }
        //pthread_mutex_unlock(buffer->lock);
    }
    return NULL;
}

void* receive_data(void* data)
{
    local_client_send_buffer* buff = (local_client_send_buffer*)data;

    // pomocne premenne
	Fruit** fruits = malloc(sizeof(Fruit) * 2);
    Snake** snakes = malloc(sizeof(Snake) * 2);
    for (int i = 0; i < 2; ++i)
    {
        snakes[i] = malloc(sizeof(Snake));
    }
    _Bool drawn_border = 0;
    while (1) {

        pthread_mutex_lock(buff->lock);
        while (buff->game_data == NULL)
        {
            // pocka kym sa nenaplni
            pthread_cond_wait(buff->is_New, buff->lock);
        }

        if (buff->game_data == NULL)
        {
            break;
        }
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

        copy_snakes(snakes, buff->game_data->snakes);
        copy_fruits(fruits, buff->game_data->fruits);
        //free_fruits(fruits);
        //free_snakes(snakes);
        //buff->game_data->snakes = NULL;
        //buff->game_data->fruits = NULL;

        buff->game_data = NULL;

        pthread_mutex_unlock(buff->lock);
    }
    free_fruits(fruits);
    free_snakes(snakes);
    free(fruits);
    free(snakes);
    return NULL;
}

void* run_server(void* data)
{
    server_data* server_buffer = (server_data*)data;

    createGameS(server_buffer->type, server_buffer->mode, server_buffer->width, server_buffer->height, server_buffer->timer, server_buffer->send_buffer, server_buffer->receive_buffer);

}

void create_session()
{
    char* options[] = { "Standardny", "Casovy" };

    int gameMode = menu(options, 2);
    int cas = 0;
    if (gameMode == 1)
    {
        _Bool valid = 1;
        initscr();
        keypad(stdscr, TRUE);
        noecho();
        timeout(0);

        clear();
        int x = 0;
        while (valid)
        {
            mvprintw(0, x, "Zadaj hraci cas: "); // Print prompt

            // Flush output to ensure the user sees it
            refresh();

            // Use scanw (ncurses version of scanf) instead of scanf
            echo(); // Temporarily enable echo to show user input
            timeout(-1); // Block input to allow proper user input
            if (scanw("%d", &cas) != 1) {
                // If input is invalid (e.g., not a number)
                noecho();
                mvprintw(1, 0, "Nespravny vstup. Skuste znova.");
                refresh();
                timeout(0);
                continue;
            }
            noecho(); // Disable echo again
            timeout(0); // Restore non-blocking input mode

            if (cas <= 0) {
                mvprintw(1, 0, "Nespravny cas, vyskusaj znova.");
                refresh();
                continue;
            }
            valid = 0; // Exit the loop when valid input is provided
        }
        endwin();
    }

    char* optionz[] = { "Svet bez prekazok", "Svet s prekazkamy" };
    int type = menu(optionz, 2);
    int vyska, sirka = 0;

    if (type == 0) {
        _Bool valid = 1;
        initscr();
        keypad(stdscr, TRUE);
        noecho();
        timeout(0);

        clear();
        int x = 0;
        while (valid) {
            mvprintw(x, 0, "Zadaj sirku a vysku plochy (Max - 40, 40; Min - 15, 15): ");
            refresh();

            echo();
            timeout(-1);
            if (scanw("%d %d", &vyska, &sirka) != 2) { // Expecting two integers
                noecho();
                clear();
            	mvprintw(0, 0, "Nespravny vstup. Skuste znova.");
                refresh();
                timeout(0);
                x = 1;
                continue;
            }
            noecho();
            timeout(0);

        	if (vyska < 15 || sirka < 15 || vyska > 40 || sirka > 40) {
                clear();
                mvprintw(0, 0, "Nespravne zadane rozmery, skuste znova.");
                x = 1;
                refresh();
                continue;
            }
            valid = 0; // Exit loop if input is valid
        }
        clear();
        endwin();
    }


    local_client_receive_buffer* receive_buffer = malloc(sizeof(local_client_receive_buffer));
    receive_buffer->is_end = 0;
    receive_buffer->direction = malloc(sizeof(int) * 2);
	receive_buffer->direction[0] = 261;
    receive_buffer->direction[0] = 260;
    receive_buffer->lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(receive_buffer->lock, NULL);
    receive_buffer->read_remote = malloc(sizeof(pthread_cond_t));
    pthread_cond_init(receive_buffer->read_remote, NULL);
    receive_buffer->read_local = malloc(sizeof(pthread_cond_t));
    pthread_cond_init(receive_buffer->read_local, NULL);

    local_client_send_buffer* send_buffer = malloc(sizeof(local_client_send_buffer));
    send_buffer->game_data = NULL;
    send_buffer->lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(send_buffer->lock, NULL);
    send_buffer->is_New = malloc(sizeof(pthread_cond_t));
    pthread_cond_init(send_buffer->is_New, NULL);

    server_data server_data;
    server_data.send_buffer = send_buffer;
    server_data.receive_buffer = receive_buffer;
    server_data.type = type;
    server_data.mode = gameMode;
    server_data.timer = cas;
    server_data.width = sirka;
    server_data.height = vyska;

    pthread_t client_send_t;
    pthread_t client_receive_t;
    pthread_t server_t;

    pthread_create(&server_t, NULL, run_server, &server_data);
    pthread_create(&client_send_t, NULL, send_data, receive_buffer);
    pthread_create(&client_receive_t, NULL, receive_data, send_buffer);
    //TODO spravit thready clienta

    pthread_join(server_t, NULL);
	pthread_join(client_send_t, NULL);
    pthread_join(client_receive_t, NULL);

}
