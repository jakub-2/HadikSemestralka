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

Obstacle* create_obstacle(int x, int y)
{
    Obstacle* obstacle = malloc(sizeof(Obstacle));
    obstacle->x = x;
    obstacle->y = y;
    return obstacle;
}

void load_map(const char* filename, GameData* gameData) {
    FILE* mapFile = fopen(filename, "r");
    if (!mapFile) {
        perror("Error opening file.");
        exit(102);
    }

    fscanf(mapFile, "%d;%d", &(gameData->width), &(gameData->height));
    fscanf(mapFile, "%d", &(gameData->count_obstacles));

    gameData->obstacles = malloc(sizeof(Obstacle) * gameData->count_obstacles);

    int x, y;
    for (int i = 0; i < gameData->count_obstacles; ++i)
    {
        fscanf(mapFile, "%d;%d", &x, &y);
        gameData->obstacles[i] = create_obstacle(x, y);
    }

    fclose(mapFile);
}

void draw_map(GameData* gameData)
{
    // Draw borders
    for (int i = 0; i < gameData->width; ++i) {
        mvprintw(0, i, "#");
        mvprintw(gameData->height - 1, i, "#");
        //refresh();
    }
    for (int i = 0; i < gameData->height; ++i) {
        mvprintw(i, 0, "#");
        mvprintw(i, gameData->width - 1, "#");
        //refresh();
    }

    // Draw obstacles
    for (int i = 0; i < gameData->count_obstacles; i++) {
        mvprintw(gameData->obstacles[i]->y, gameData->obstacles[i]->x, "#");
    }
    //refresh();
}

void free_obstacles(GameData* gameData)
{
    for (int i = 0; i < gameData->count_obstacles; ++i)
    {
        free(gameData->obstacles[i]);
    }

    free(gameData->obstacles);
    gameData->obstacles = NULL;
}

// TODO vymazat netreba
void serialize_obstacles(Obstacle** obstacles, char* buffer, int buffer_size, int obstaclesCount) {
    //snprintf(buffer, buffer_size, "%d,%d;%d,%d;", obstacles[0]->x, obstacles[0]->y, obstacles[1]->x, obstacles[1]->y);

    for (int i = 0; i < obstaclesCount; ++i)
    {
        snprintf(buffer, buffer_size, "%d,%d;", obstacles[i]->x, obstacles[i]->y);
    }
}

// TODO vymazat netreba
void deserialize_obstacles(const char* data, Obstacle** obstacles) {
    const char* obstacles_start = strstr(data, "Obstacles:") + strlen("Obstacles:");
    const char* obstacles_end = strstr(data, "EndOfObstacles");

    char obstacles_data[256];
    strncpy(obstacles_data, obstacles_start, obstacles_end - obstacles_start);
    obstacles_data[obstacles_end - obstacles_start] = '\0';

    char* obstacle_token = strtok(obstacles_data, ";");
    int obstacle_index = 0;
    while (obstacle_token) {
        int x, y;
        sscanf(obstacle_token, "%d,%d", &x, &y);
        obstacles[obstacle_index++] = create_obstacle(x, y);
        obstacle_token = strtok(NULL, ";");
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
	    if (snakes[i] == NULL)
	    {
            continue;
	    }
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

// TODO delete -> moved to draw_map
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

void draw_score(GameData* game_data)
{
    for (int i = 0; i < 2; ++i)
    {
        mvprintw(game_data->height + 1 + i, 0, "Score Snake %c: %d", game_data->snakes[i]->idChar, game_data->snakes[i]->score);
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
	    if (snakes[i]->isDead == 0)
	    {
            draw_snake(snakes[i]);
	    }
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

void draw_timer(GameData* game_data)
{
    mvprintw(game_data->height, 0, "Remaining time: %.0fs", game_data->timer);
}

void erase_snake(SnakeSegment* snake) {
    SnakeSegment* current = snake;
    while (current) {
        mvprintw(current->y, current->x, " "); // Move to the position and replace with a space
        current = current->next;
        refresh();
    }
    refresh(); // Refresh the screen to apply changes
}

void erase_fruit(Fruit* fruit)
{
    mvprintw(fruit->y, fruit->x, " ");
    refresh(); // Refresh the screen to apply changes
}

//TODO check for number of free spaces
void generate_food(GameData* game_data, int index) {
	if (game_data->count_free_spaces >= 2)
	{
        int valid;
        do {
            valid = 1;
            game_data->fruits[index]->x = rand() % (game_data->width - 2) + 1;
            game_data->fruits[index]->y = rand() % (game_data->height - 2) + 1;

            if (game_data->fruits[index]->x == game_data->fruits[(index + 1) % 2]->x && game_data->fruits[index]->y == game_data->fruits[(index + 1) % 2]->y)
            {
                valid = 0;
                continue;
            }
            for (int i = 0; i < 2; ++i)
            {
                SnakeSegment* current = game_data->snakes[i]->head;
                while (current) {
                    if (current->x == game_data->fruits[index]->x && current->y == game_data->fruits[index]->y) {
                        valid = 0;
                        break;
                    }
                    current = current->next;
                }
            }
            for (int i = 0; i < game_data->count_obstacles; ++i)
            {
	            if (game_data->fruits[index]->x == game_data->obstacles[i]->x && game_data->fruits[index]->y == game_data->obstacles[i]->y)
	            {
                    valid = 0;
                    break;
	            }
            }
        } while (!valid);
	}
	else
	{
        game_data->fruits[index]->x = -1;
        game_data->fruits[index]->y = -1;
	}
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

// new game_data serialization
void serialize_game_data(GameData* game_data, char* buffer, int buffer_size) {
    char temp[256];

    // Serialize GameData properties
    snprintf(buffer, buffer_size, "GameData:%d,%d,%d,%d,%d,%d,%d;\n",
        game_data->width, game_data->height, game_data->type, game_data->mode,
        (int)game_data->timer, game_data->count_obstacles, game_data->count_free_spaces);

    // Serialize Fruits
    strcat(buffer, "Fruits:");
    for (int i = 0; i < 2; ++i) {
        snprintf(temp, 256, "%d,%d;", game_data->fruits[i]->x, game_data->fruits[i]->y);
        strcat(buffer, temp);
    }
    strcat(buffer, "\n");

    // Serialize Snakes
    strcat(buffer, "Snakes:");
    for (int i = 0; i < 2; ++i) {
        snprintf(temp, 256, "%c,%d,%d,%d:", game_data->snakes[i]->idChar,
            game_data->snakes[i]->score, game_data->snakes[i]->direction, game_data->snakes[i]->isDead);
        strcat(buffer, temp);

        SnakeSegment* segment = game_data->snakes[i]->head;
        while (segment) {
            snprintf(temp, 256, "%d,%d->", segment->x, segment->y);
            strcat(buffer, temp);
            segment = segment->next;
        }
        strcat(buffer, ";");
    }
    strcat(buffer, "\n");

    // Serialize Obstacles
    strcat(buffer, "Obstacles:");
    for (int i = 0; i < game_data->count_obstacles; ++i) {
        snprintf(temp, 256, "%d,%d;", game_data->obstacles[i]->x, game_data->obstacles[i]->y);
        strcat(buffer, temp);
    }
    strcat(buffer, "\n");
}

//old game serialization
void serialize_game(Snake** snakes, Fruit** fruits, Obstacle** obstacles, char* buffer, int buffer_size) {
    char fruit_buffer[50], snake1_buffer[500], snake2_buffer[500];

    // Serialize fruits
    serialize_fruits(fruits, fruit_buffer, sizeof(fruit_buffer));

    // Serialize obstacless
    //serialize_obstacles(obstacles, buffer, buffer_size, ?);

    // Serialize both snakes
    serialize_snake(snakes[0], snake1_buffer, sizeof(snake1_buffer));
    serialize_snake(snakes[1], snake2_buffer, sizeof(snake2_buffer));

    // Combine into the main buffer
    snprintf(buffer, buffer_size, "Fruits:%sSnake1:%sSnake2:%s\0", fruit_buffer, snake1_buffer, snake2_buffer);
}

//new deserialization
void deserialize_game_data(const char* data, GameData* game) {
    // Parse GameData properties
    const char* game_data_start = strstr(data, "GameData:") + strlen("GameData:");
    const char* game_data_end = strstr(game_data_start, ";");
    char game_data[256];
    strncpy(game_data, game_data_start, game_data_end - game_data_start);
    game_data[game_data_end - game_data_start] = '\0';

    int vymaz_timer;
    sscanf(game_data, "%d,%d,%d,%d,%d,%d,%d",
        &game->width, &game->height, &game->type, &game->mode,
        &vymaz_timer, &game->count_obstacles, &game->count_free_spaces);

    game->timer = vymaz_timer;
    // Parse Fruits
    const char* fruits_start = strstr(data, "Fruits:") + strlen("Fruits:");
    const char* fruits_end = strstr(fruits_start, "\n");
    char fruits_data[256];
    strncpy(fruits_data, fruits_start, fruits_end - fruits_start);
    fruits_data[fruits_end - fruits_start] = '\0';

    char* fruit_token = strtok(fruits_data, ";");
    int fruit_index = 0;
    while (fruit_token) {
        int x, y;
        sscanf(fruit_token, "%d,%d", &x, &y);
        game->fruits[fruit_index++] = create_fruit(x, y);
        fruit_token = strtok(NULL, ";");
    }

    // Parse Snakes
    const char* snakes_start = strstr(data, "Snakes:") + strlen("Snakes:");
    const char* snakes_end = strstr(snakes_start, "\n");
    char snakes_data[1024];
    strncpy(snakes_data, snakes_start, snakes_end - snakes_start);
    snakes_data[snakes_end - snakes_start] = '\0';

    char* snake_token = strtok(snakes_data, ";");
    int snake_index = 0;
    while (snake_token) {
        char idChar;
        int score, direction, isDead;

        sscanf(snake_token, "%c,%d,%d,%d:", &idChar, &score, &direction, &isDead);

        Snake* snake = create_snake(idChar, 0, 0, direction);
        SnakeSegment* head = snake->head;
        SnakeSegment* tail = snake->head;
        int x, y;
        char* segment_token = strstr(snake_token, ":") + 1;
        if (segment_token && sscanf(segment_token, "%d,%d", &x, &y) == 2)
        {
            head->x = x;
            head->y = y;
        }
        while (segment_token && sscanf(segment_token, "%d,%d", &x, &y) == 2) {
            SnakeSegment* segment = create_segment(x, y);

            if (!head) {
                head = segment;
                tail = segment;
            }
            else {
                tail->next = segment;
                tail = segment;
            }

            segment_token = strstr(segment_token, "->");
            if (segment_token) segment_token += 2;
        }

        snake->score = score;
        snake->isDead = isDead;
        game->snakes[snake_index++] = snake;

        snake_token = strtok(NULL, ";");
    }

    // Parse Obstacles
    const char* obstacles_start = strstr(data, "Obstacles:") + strlen("Obstacles:");
    const char* obstacles_end = strstr(obstacles_start, "\n");
    char obstacles_data[256];
    strncpy(obstacles_data, obstacles_start, obstacles_end - obstacles_start);
    obstacles_data[obstacles_end - obstacles_start] = '\0';

    char* obstacle_token = strtok(obstacles_data, ";");
    int obstacle_index = 0;
    while (obstacle_token) {
        int x, y;
        sscanf(obstacle_token, "%d,%d", &x, &y);
        game->obstacles[obstacle_index++] = create_obstacle(x, y);
        obstacle_token = strtok(NULL, ";");
    }
}

//old
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

//new buffer_size
int calculate_max_buffer_size(GameData* game) {
    // Maximum possible segments for all snakes
    int max_segments_per_snake = (game->width * game->height) / 2;
    int max_snake_segments = 2 * max_segments_per_snake; // Assuming 2 snakes

    // Maximum size for snakes
    int snake_size = max_snake_segments * 6; // Each segment: "x,y->"
    snake_size += 2 * 3; // Snake ID for each snake: "1:" or "2:"

    // Maximum size for fruits
    int max_fruits = game->width * game->height; // Assuming fruits could fill the entire grid
    int fruit_size = max_fruits * 6; // Each fruit: "x,y;"

    // Maximum size for obstacles
    int max_obstacles = game->width * game->height; // Obstacles could theoretically fill the grid
    int obstacle_size = max_obstacles * 6; // Each obstacle: "x,y;"

    // Metadata
    int metadata_size = 200; // For labels, numbers, separators, and additional metadata

    // Total size
    return fruit_size + snake_size + obstacle_size + metadata_size;
}

//old
int calculate_buffer_size(int width, int height, int num_fruits, int num_snakes) {
    int max_segments_per_snake = (width * height) / num_snakes; // Max segments per snake
    int fruit_size = num_fruits * 6; // Each fruit: "x,y;" -> 6 chars
    int snake_size = num_snakes * (max_segments_per_snake * 6); // Each segment: "x,y->" or ";"
    int metadata_size = 30; // For labels and newlines

    return fruit_size + snake_size + metadata_size;
}

// Function to copy a single SnakeSegment node
SnakeSegment* copySnakeSegment(const SnakeSegment* original) {
    if (original == NULL) {
        return NULL;
    }

    // Allocate memory for the new segment
    SnakeSegment* copy = (SnakeSegment*)malloc(sizeof(SnakeSegment));
    if (!copy) {
        return NULL; // Handle allocation failure
    }

    // Copy the values
    copy->x = original->x;
    copy->y = original->y;
    copy->next = copySnakeSegment(original->next); // Recursively copy the next segment

    return copy;
}

// Function to copy the entire Snake structure
Snake* copySnake(const Snake* original) {
    if (original == NULL) {
        return NULL;
    }

    // Allocate memory for the new snake
    Snake* snake = (Snake*)malloc(sizeof(Snake));
    if (!snake)
    {
        return NULL; // Handle allocation failure
    }

    // Copy the simple fields
    snake->score = original->score;
    snake->direction = original->direction;
    snake->isDead = original->isDead;
    snake->idChar = original->idChar;

    // Deep copy the linked list of SnakeSegment
    snake->head = copySnakeSegment(original->head);
    return snake;
}


Fruit* copy_fruits(Fruit* fruits_old)
{
    Fruit* fruit = create_fruit(fruits_old->x, fruits_old->y);
}

void update(Fruit** fruits, Snake** snakes, GameData* game_data)
{
    for (int i = 0; i < 2; ++i)
    {
        erase_snake(snakes[i]->head);
        erase_fruit(fruits[i]);
    }

    draw_snakes(game_data->snakes);
    draw_fruit(game_data->fruits);
    draw_score(game_data);
    if (game_data->mode == 1)
    {
        draw_timer(game_data);
    }
}

void moveSnake(GameData* game_data, _Bool _print)
{
    // Move snakes
    int new_x[] = { game_data->snakes[0]->head->x, game_data->snakes[1]->head->x };
    int new_y[] = { game_data->snakes[0]->head->y, game_data->snakes[1]->head->y };

    for (int i = 0; i < 2; ++i)
    {
        if (game_data->snakes[i]->direction == KEY_UP) {
            new_y[i]--;
        }
        else if (game_data->snakes[i]->direction == KEY_DOWN) {
            new_y[i]++;
        }
        else if (game_data->snakes[i]->direction == KEY_LEFT) {
            new_x[i]--;
        }
        else if (game_data->snakes[i]->direction == KEY_RIGHT) {
            new_x[i]++;
        }
    }

    // Check collisions
    // Check if colliding with border
    for (int i = 0; i < 2; ++i)
    {
        if (game_data->snakes[i]->isDead == 1)
        {
            continue;
        }
        //if (new_x[i] <= 0 || new_x[i] >= WIDTH - 1 || new_y[i] <= 0 || new_y[i] >= HEIGHT - 1) {
        //    if (_print)
        //    {
        //        erase_snake(game_data->snakes[i]->head);
        //    }
        //    game_data->snakes[i]->isDead = 1;
        //}
        if (new_x[i] <= 0)
        {
            new_x[i] = game_data->width - 2;
        }
        else if (new_x[i] >= game_data->width - 1)
        {
            new_x[i] = 1;
        }
        else if (new_y[i] <= 0)
        {
            new_y[i] = game_data->height - 2;
        }
        else if (new_y[i] >= game_data->height - 1)
        {
            new_y[i] = 1;
        }
    }

    // check if snakes move to same space
    if (game_data->snakes[0]->isDead == game_data->snakes[1]->isDead)
    {
        if (new_x[0] == new_x[1] && new_y[0] == new_y[1])
        {
            game_data->snakes[0]->isDead = 1;
            game_data->snakes[1]->isDead = 1;
        }
    }

    _Bool tempDeath[] = { 0, 0 };
    for (int i = 0; i < 2; ++i)
    {
        if (game_data->snakes[i]->isDead == 1)
        {
            continue;
        }
        // check if snake collides with itself
        SnakeSegment* current = game_data->snakes[i]->head;
        while (current->next) {
            if (current->x == new_x[i] && current->y == new_y[i]) {
                if (_print)
                {
                    erase_snake(game_data->snakes[i]->head);
                }
                game_data->snakes[i]->isDead = 1;
                game_data->count_free_spaces += game_data->snakes[i]->score + 3;
            }
            current = current->next;
        }

        // check if any part of snake2 collides with new cords
        int index2 = (i + 1) % 2;
        if (game_data->snakes[index2]->isDead == 1)
        {
            continue;
        }
        current = game_data->snakes[index2]->head;
        while (current->next) {
            if (current->x == new_x[i] && current->y == new_y[i]) {
                if (_print)
                {
                    erase_snake(game_data->snakes[i]->head);
                }
                tempDeath[i] = 1;
                //snakes[i]->isDead = 1;
            }
            current = current->next;
        }
    }

    //check if snake collides with obstacles
    for (int i = 0; i < 2; ++i)
    {
	    if (game_data->snakes[i]->isDead)
	    {
            continue;
	    }
	    for (int j = 0; j < game_data->count_obstacles; ++j)
	    {
		    if (new_x[i] == game_data->obstacles[j]->x && new_y[i] == game_data->obstacles[j]->y)
		    {
                if (_print)
                {
                    erase_snake(game_data->snakes[i]->head);
                }
                game_data->snakes[i]->isDead = 1;
                game_data->count_free_spaces += game_data->snakes[i]->score + 3;
		    }
	    }
    }

    for (int i = 0; i < 2; ++i)
    {
        if (tempDeath[i])
        {
            game_data->snakes[i]->isDead = tempDeath[i];
            game_data->count_free_spaces += game_data->snakes[i]->score + 3;
        }
    }

    // Add new head
    for (int i = 0; i < 2; ++i)
    {
        if (game_data->snakes[i]->isDead == 1)
        {
            continue;
        }
        SnakeSegment* new_head = create_segment(new_x[i], new_y[i]);
        new_head->next = game_data->snakes[i]->head;
        game_data->snakes[i]->head = new_head;
        if (_print)
        {
            mvprintw(new_y[i], new_x[i], &(game_data->snakes[i]->idChar));
        }
    }

    // Check food collision
    for (int i = 0; i < 2; ++i)
    {
        //check if snake is dead
        if (game_data->snakes[i]->isDead == 1)
        {
            continue;
        }
        int fruitIndex = -1;
        if (collidesWithFruit(game_data->fruits, new_x[i], new_y[i], &fruitIndex)) {
            game_data->snakes[i]->score++;
            game_data->count_free_spaces--;
            generate_food(game_data, fruitIndex);
            draw_fruit(game_data->fruits);
        }
        else {
            // Remove tail
            SnakeSegment* temp = game_data->snakes[i]->head;
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

void createGame(GameData* game_data, _Bool _print)
{
    game_data->snakes = malloc(sizeof(Snake) * 2);
    game_data->fruits = malloc(sizeof(Fruit) * 2);

    game_data->count_free_spaces = (game_data->width - 2) * (game_data->height - 2);

    if (game_data->type == 1)
    {
        //load_map("/home/jakub/.vs/HadikSemestralka/HadikSemestralka/map.txt", game_data);
        //load_map("/home/jakub/.vs/HadikSemestralka/HadikSemestralka/map1.txt", game_data);
        load_map("/home/jakub/.vs/HadikSemestralka/HadikSemestralka/map2.txt", game_data);
    }
    else
    {
        game_data->obstacles = NULL;
        game_data->count_obstacles = 0;
    }
    game_data->count_free_spaces -= game_data->count_obstacles;

    //print = _print;
    game_data->snakes[0] = create_snake('1', game_data->width / 2, game_data->height / 2, KEY_RIGHT);
    game_data->snakes[1] = create_snake('2', game_data->width / 2, game_data->height / 2 + 1, KEY_LEFT);

    game_data->fruits[0] = create_fruit(0, 0);
    game_data->fruits[1] = create_fruit(0, 1);

    for (int i = 0; i < 2; ++i)
    {
        add_segment(game_data->snakes[i]->head);
        add_segment(game_data->snakes[i]->head);
    }
    game_data->count_free_spaces -= 2 * 3;

    for (int i = 0; i < 2; ++i)
    {
        generate_food(game_data, i);
    }
    game_data->count_free_spaces -= 2;

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
        draw_map(game_data);
        //draw_obstacles();
        draw_snakes(game_data->snakes);

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
    //TODO pridat free na vsetko
    free_snakes(snakes);
}

_Bool play(int* moves, GameData* game_data, _Bool _print)
{
    if (_print)
    {
        draw_fruit(game_data->fruits);
    }

    for (int i = 0; i < 2; ++i)
    {
        if ((moves[i] == KEY_UP && game_data->snakes[i]->direction != KEY_DOWN) ||
            (moves[i] == KEY_DOWN && game_data->snakes[i]->direction != KEY_UP) ||
            (moves[i] == KEY_LEFT && game_data->snakes[i]->direction != KEY_RIGHT) ||
            (moves[i] == KEY_RIGHT && game_data->snakes[i]->direction != KEY_LEFT)) {
            game_data->snakes[i]->direction = moves[i];
        }
    }

    // moveSnake returns if snake is dead after move or not
    moveSnake(game_data, _print);

    if ((game_data->snakes[0]->isDead == 1 && game_data->snakes[1]->isDead == 1) || (game_data->mode == 1 && game_data->timer <= 0))
    {
        return 1;
    }
    if (_print)
    {
        draw_score(game_data);
        if (game_data->mode == 1)
        {
            draw_timer(game_data);
        }
    }
    return 0;
}