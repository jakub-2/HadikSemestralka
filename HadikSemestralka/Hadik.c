#include "Hadik.h"

Fruit* create_fruit(int x, int y)
{
    Fruit* fruit = malloc(sizeof(Fruit));
    fruit->x = x;
    fruit->y = y;
    return fruit;
}

void free_fruits(Fruit** fruits)
{
    for (int i = 0; i < 2; ++i)
    {
        free(fruits[i]);
    }
}

SnakeSegment* create_segment(int x, int y) {
    SnakeSegment* segment = (SnakeSegment*)malloc(sizeof(SnakeSegment));
    segment->x = x;
    segment->y = y;
    segment->next = NULL;
    return segment;
}

Snake* create_snake(char idChar, int x, int y, int direction) {
    Snake* snake = malloc(sizeof(Snake));
    snake->head = create_segment(x, y);
    snake->direction = direction;
    snake->score = 0;
    snake->isDead = 0;
    snake->idChar = idChar;
    return snake;
}

void free_snakes(Snake** snakes) {
    for (int i = 0; i < 2; ++i)
    {
        SnakeSegment* head = snakes[i]->head;
        while (head) {
            SnakeSegment* temp = head;
            head = head->next;
            free(temp);
        }
        free(snakes[i]);
    }
}

void add_segment(SnakeSegment* head) {
    SnakeSegment* current = head;
    while (current->next) {
        current = current->next;
    }
    current->next = create_segment(current->x, current->y);
}

void draw_borders() {
    for (int i = 0; i < WIDTH; ++i) {
        mvprintw(0, i, "#");
        mvprintw(HEIGHT - 1, i, "#");
        refresh();
    }
    for (int i = 0; i < HEIGHT; ++i) {
        mvprintw(i, 0, "#");
        mvprintw(i, WIDTH - 1, "#");
        refresh();
    }
}

void draw_obstacles(int* obstacleX, int* obstacleY, SnakeSegment* head, int sirka, int vyska, int numOfObstacles)
{
	for (int i = 0; i < numOfObstacles; ++i)
	{
        int isValid;
        do {
            isValid = 1;
            *obstacleX = rand() % (WIDTH - 2) + 1;
            *obstacleY = rand() % (HEIGHT - 2) + 1;
            SnakeSegment* current = head;
            while (current) {
                if (current->x == *obstacleX && current->y == *obstacleY) {
                    isValid = 0;
                    break;
                }
                current = current->next;
            }
        } while (!isValid);
	}
}

void draw_score(Snake** snakes)
{
	for (int i = 0; i < 2; ++i)
	{
        mvprintw(HEIGHT + 1 + i, 0, snakes[i]->score);
	}
}

void draw_snake(Snake* snake) {
    SnakeSegment* current = snake->head;
    while (current) {
        mvprintw(current->y, current->x, &(snake->idChar));
        current = current->next;
    }
}

void draw_snakes(Snake** snakes)
{
    for (int i = 0; i < 2; ++i)
    {
        char c;
        //sprintf(&c, "%d", i);
        draw_snake(snakes[i]);
    }
}

void draw_fruit(Fruit** fruits)
{
    for (int i = 0; i < 2; ++i)
    {
        mvprintw(fruits[i]->y, fruits[i]->x, "X");
    }
    refresh();
}

void erase_snake(SnakeSegment* snake) {
    SnakeSegment* current = snake;
    while (current) {
        mvprintw(current->y, current->x, " "); // Move to the position and replace with a space
        current = current->next;
    }
    refresh(); // Refresh the screen to apply changes
}

void erase_fruit(Fruit* fruit)
{
	mvprintw(fruit->y, fruit->x, " ");
    refresh(); // Refresh the screen to apply changes
}

//TODO check for number of free spaces
void generate_food(Fruit** fruits, int index, Snake** snakes) {
    int valid;
    do {
        valid = 1;
        fruits[index]->x = rand() % (WIDTH - 2) + 1;
        fruits[index]->y = rand() % (HEIGHT - 2) + 1;

        if (fruits[index]->x == fruits[(index + 1) % 2]->x && fruits[index]->y == fruits[(index + 1) % 2]->y)
        {
            valid = 0;
            continue;
        }
        for (int i = 0; i < 2; ++i)
        {
            SnakeSegment* current = snakes[i]->head;
            while (current) {
                if (current->x == fruits[index]->x && current->y == fruits[index]->y) {
                    valid = 0;
                    break;
                }
                current = current->next;
            }
        }
    } while (!valid);
}

_Bool collidesWithFruit(Fruit** fruits, int x, int y, int* fruitIndex)
{
    for (int i = 0; i < 2; ++i)
    {
        if (x == fruits[i]->x && y == fruits[i]->y)
        {
            *fruitIndex = i;
            return 1;
        }
    }
    return 0;
}

void serialize_fruits(Fruit** fruits, char* buffer, int buffer_size) {
    snprintf(buffer, buffer_size, "%d,%d;%d,%d;",
        fruits[0]->x, fruits[0]->y,
        fruits[1]->x, fruits[1]->y);
}

void serialize_snake(Snake* snake, char* buffer, int buffer_size) {
    SnakeSegment* current = snake->head;
    int offset = 0;

    while (current != NULL && offset < buffer_size) {
        int written = snprintf(buffer + offset, buffer_size - offset, "%d,%d->", current->x, current->y);
        offset += written;
        current = current->next;
    }

    if (offset > 0) {
        buffer[offset - 2] = ';'; // Replace the last "->" with ";"
        buffer[offset - 1] = '\0';
    }
}

void serialize_game(Snake** snakes, Fruit** fruits, char* buffer, int buffer_size) {
    char fruit_buffer[50], snake1_buffer[500], snake2_buffer[500];

    // Serialize fruits
    serialize_fruits(fruits, fruit_buffer, sizeof(fruit_buffer));

    // Serialize both snakes
    serialize_snake(snakes[0], snake1_buffer, sizeof(snake1_buffer));
    serialize_snake(snakes[1], snake2_buffer, sizeof(snake2_buffer));

    // Combine into the main buffer
    snprintf(buffer, buffer_size, "Fruits:%sSnake1:%sSnake2:%s\0", fruit_buffer, snake1_buffer, snake2_buffer);
}

void deserialize_data(const char* data, Fruit** fruits, Snake** snakes) {
    // Deserialize Fruits
    const char* fruits_start = strstr(data, "Fruits:") + strlen("Fruits:");
    const char* fruits_end = strstr(data, "Snake1:");
    char fruits_data[256];
    strncpy(fruits_data, fruits_start, fruits_end - fruits_start);
    fruits_data[fruits_end - fruits_start] = '\0';

    char* fruit_token = strtok(fruits_data, ";");
    int fruit_index = 0;
    while (fruit_token) {
        int x, y;
        sscanf(fruit_token, "%d,%d", &x, &y);
        fruits[fruit_index++] = create_fruit(x, y);
        fruit_token = strtok(NULL, ";");
    }

    // Deserialize Snakes
    for (int snake_index = 0; snake_index < 2; ++snake_index) {
        char snake_label[16];
        sprintf(snake_label, "Snake%d:", snake_index + 1);
        const char* snake_start = strstr(data, snake_label) + strlen(snake_label);
        const char* snake_end = strstr(data + (snake_start - data) + 1, "Snake");
        if (!snake_end) {
            snake_end = data + strlen(data);
        }

        char snake_data[1024];
        strncpy(snake_data, snake_start, snake_end - snake_start);
        snake_data[snake_end - snake_start] = '\0';

        char* segment_token = strtok(snake_data, "->");
        Snake* snake = create_snake('1' + snake_index, 0, 0, 0); // Temporary placeholder
        SnakeSegment* head = NULL;
        SnakeSegment* tail = NULL;

        while (segment_token) {
            int x, y;
            sscanf(segment_token, "%d,%d", &x, &y);
            SnakeSegment* segment = create_segment(x, y);

            if (!head) {
                head = segment;
                tail = segment;
            }
            else {
                tail->next = segment;
                tail = segment;
            }

            segment_token = strtok(NULL, "->");
        }
        snake->head = head;
        snakes[snake_index] = snake;
    }
}

int calculate_buffer_size(int width, int height, int num_fruits, int num_snakes) {
    int max_segments_per_snake = (width * height) / num_snakes; // Max segments per snake
    int fruit_size = num_fruits * 6; // Each fruit: "x,y;" -> 6 chars
    int snake_size = num_snakes * (max_segments_per_snake * 6); // Each segment: "x,y->" or ";"
    int metadata_size = 30; // For labels and newlines

    return fruit_size + snake_size + metadata_size;
}

void update(Fruit** fruits, Snake** snakes, Fruit** old_fruits, Snake** old_snakes)
{
	for (int i = 0; i < 2; ++i)
	{
        erase_snake(old_snakes[i]->head);
        erase_fruit(old_fruits[i]);
	}

    draw_snakes(snakes);
    draw_fruit(fruits);
    draw_score(snakes);
}

void moveSnake(Snake** snakes, Fruit** fruits, _Bool _print)
{
    // Move snakes
    int new_x[] = { snakes[0]->head->x, snakes[1]->head->x };
    int new_y[] = { snakes[0]->head->y, snakes[1]->head->y };

    for (int i = 0; i < 2; ++i)
    {
        if (snakes[i]->direction == KEY_UP) {
            new_y[i]--;
        }
        else if (snakes[i]->direction == KEY_DOWN) {
            new_y[i]++;
        }
        else if (snakes[i]->direction == KEY_LEFT) {
            new_x[i]--;
        }
        else if (snakes[i]->direction == KEY_RIGHT) {
            new_x[i]++;
        }
    }

    // Check collisions
    // Check if colliding with border
    for (int i = 0; i < 2; ++i)
    {
        if (snakes[i]->isDead == 1)
        {
            continue;
        }
        if (new_x[i] <= 0 || new_x[i] >= WIDTH - 1 || new_y[i] <= 0 || new_y[i] >= HEIGHT - 1) {
            if (_print)
            {
                erase_snake(snakes[i]->head);
            }
            snakes[i]->isDead = 1;
        }
    }

    // check if snakes move to same space
    if (snakes[0]->isDead == snakes[1]->isDead)
    {
        if (new_x[0] == new_x[1] && new_y[0] == new_y[1])
        {
            snakes[0]->isDead = 1;
            snakes[1]->isDead = 1;
        }
    }

    _Bool tempDeath[] = { 0, 0 };
    for (int i = 0; i < 2; ++i)
    {
        if (snakes[i]->isDead == 1)
        {
            continue;
        }
        // check if snake collides with itself
        SnakeSegment* current = snakes[i]->head;
        while (current->next) {
            if (current->x == new_x[i] && current->y == new_y[i]) {
                if (_print)
                {
                    erase_snake(snakes[i]->head);
                }
                snakes[i]->isDead = 1;
            }
            current = current->next;
        }

        // check if any part of snake2 collides with new cords
        int index2 = (i + 1) % 2;
        if (snakes[index2]->isDead == 1)
        {
            continue;
        }
        current = snakes[index2]->head;
        while (current->next) {
            if (current->x == new_x[i] && current->y == new_y[i]) {
                if (_print)
                {
                    erase_snake(snakes[i]->head);
                }
                tempDeath[i] = 1;
                //snakes[i]->isDead = 1;
            }
            current = current->next;
        }
    }

    for (int i = 0; i < 2; ++i)
    {
	    if (tempDeath[i])
	    {
            snakes[i]->isDead = tempDeath[i];
	    }
    }

    // Add new head
    for (int i = 0; i < 2; ++i)
    {
        if (snakes[i]->isDead == 1)
        {
            continue;
        }
        SnakeSegment* new_head = create_segment(new_x[i], new_y[i]);
        new_head->next = snakes[i]->head;
        snakes[i]->head = new_head;
        if (_print)
        {
            mvprintw(new_y[i], new_x[i], &(snakes[i]->idChar));
        }
    }

    // Check food collision
    for (int i = 0; i < 2; ++i)
    {
        //check if snake is dead
        if (snakes[i]->isDead == 1)
        {
            continue;
        }
        int fruitIndex = -1;
        if (collidesWithFruit(fruits, new_x[i], new_y[i], &fruitIndex)) {
            snakes[i]->score++;
            generate_food(fruits, fruitIndex, snakes);
        }
        else {
            // Remove tail
            SnakeSegment* temp = snakes[i]->head;
            while (temp->next->next) {
                temp = temp->next;
            }
            //mvaddch(temp->next->y, temp->next->x, ' '); // Move to the position and replace with a space
            if (_print)
            {
                // TODO overit solve na miznutie znuku pri tesnom prejdeni pri chvoste
	            if (!(new_x[i] == temp->next->x && new_y[i] == temp->next->y))
	            {
                    mvprintw(temp->next->y, temp->next->x, " ");
	            }
            }
            free(temp->next);
            temp->next = NULL;
        }
    }
}

void createGame(Snake** snakes, Fruit** fruits, _Bool _print)
{
    //print = _print;
    snakes[0] = create_snake('1', WIDTH / 2, HEIGHT / 2, KEY_RIGHT);
    snakes[1] = create_snake('2', WIDTH / 2, HEIGHT / 2 + 1, KEY_LEFT);

    fruits[0] = create_fruit(0, 0);
    fruits[1] = create_fruit(0, 1);

    for (int i = 0; i < 2; ++i)
    {
        add_segment(snakes[i]->head);
        add_segment(snakes[i]->head);
    }

    for (int i = 0; i < 2; ++i)
    {
        generate_food(fruits, i, snakes);
    }

    if (_print)
    {
        initscr();
        keypad(stdscr, TRUE);
        noecho();
        curs_set(FALSE);
        timeout(0);

        int delay = DELAY_HORIZONTAL;

        clear();
        // Draw borders, food, and snake
        draw_borders();
        draw_snakes(snakes);

    }
}

void endGame(Snake** snakes, _Bool _print)
{
    if (_print)
    {
        endwin();
        for (int i = 0; i < 2; ++i)
        {
            printf("Game Over! Final Score Snake %d: %d\n", i + 1, snakes[i]->score);
        }
    }
    free_snakes(snakes);
}

_Bool play(int* moves, Snake** snakes, Fruit** fruits, _Bool _print)
{
    if (_print)
    {
        draw_fruit(fruits);
    }

    for (int i = 0; i < 2; ++i)
    {
        if ((moves[i] == KEY_UP && snakes[i]->direction != KEY_DOWN) ||
            (moves[i] == KEY_DOWN && snakes[i]->direction != KEY_UP) ||
            (moves[i] == KEY_LEFT && snakes[i]->direction != KEY_RIGHT) ||
            (moves[i] == KEY_RIGHT && snakes[i]->direction != KEY_LEFT)) {
            snakes[i]->direction = moves[i];
        }
    }

    // moveSnake returns if snake is dead after move or not
    moveSnake(snakes, fruits, _print);

    if (snakes[0]->isDead == 1 && snakes[1]->isDead == 1)
    {
        return 1;
    }
    return 0;
}