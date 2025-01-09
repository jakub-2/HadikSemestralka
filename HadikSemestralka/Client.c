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
    }
    return NULL;
}

void* receive_data(void* data)
{
    local_client_send_buffer* buff = (local_client_send_buffer*)data;

    // pomocne premenne
	Fruit** fruits = malloc(sizeof(Fruit) * 2);
    Snake** snakes = malloc(sizeof(Snake) * 2);
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

        update(fruits, snakes, buff->game_data->fruits, buff->game_data->snakes);

        for (int i = 0; i < 2; ++i) {
            //buff->fruits[i] = malloc(sizeof(Fruit));
            memcpy(buff->game_data->fruits[i], fruits[i], sizeof(Fruit));
            memcpy(buff->game_data->snakes[i], snakes[i], sizeof(Snake));
        }
        free_fruits(fruits);
        free_snakes(snakes);
        buff->game_data->snakes = NULL;
        buff->game_data->fruits = NULL;

        pthread_mutex_unlock(buff->lock);
    }

    free(fruits);
    free(snakes);
    return NULL;
}

void* run_server(void* data)
{
    server_data* server_buffer = (server_data*)data;

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
            mvprintw(0, x, "Zadaj hraci cas: ");
            scanf("%d", &cas);

            if (cas <= 0)
            {
                mvprintw(0, 0, "Nespravny cas vyskusaj znova");
                x = 1;
                continue;
            }
            valid = 0;
        }
        endwin();
    }

    char* optionz[] = {"Svet bez prekazok", "Svet s prekazkamy"};
    int type = menu(optionz, 2);
    int vyska, sirka = 0;

    if (server_buffer->send_buffer->game_data->type == 0)
    {
        _Bool valid = 1;
        initscr();
        keypad(stdscr, TRUE);
        noecho();
        timeout(0);

        clear();
        int xPrint = 0;
        while (valid)
        {
            mvprintw(0, xPrint, "Zadaj sirku a vysku plochy (Max - 40, 40; Min - 15, 15): ");
            scanf("%d, %d", &vyska, &sirka);

            if (vyska < 15 || sirka < 15 || vyska > 40 || sirka > 40)
            {
                mvprintw(0, 0, "Nespravne zadane rozmery");
                xPrint = 1;
                continue;
            }
            valid = 0;
        }
        endwin();
    }


    createGameS(type, gameMode, sirka, vyska, cas, server_buffer->send_buffer, server_buffer->receive_buffer);

}

void create_session()
{
    local_client_receive_buffer* receive_buffer = malloc(sizeof(local_client_receive_buffer));
    receive_buffer->is_end = 0;
    receive_buffer->direction = malloc(sizeof(int) * 2);
    receive_buffer->lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(receive_buffer->lock, NULL);

    local_client_send_buffer* send_buffer = malloc(sizeof(local_client_send_buffer));
    send_buffer->game_data = NULL;
    send_buffer->lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(send_buffer->lock, NULL);
    send_buffer->is_New = malloc(sizeof(pthread_cond_t));
    pthread_cond_init(send_buffer->is_New, NULL);

    server_data server_data;
    server_data.send_buffer = send_buffer;
    server_data.receive_buffer = receive_buffer;

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
