#pragma once
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h> 

#define WIDTH 20
#define HEIGHT 15
#define DELAY_HORIZONTAL 150000 // microseconds between horizontal updates
#define DELAY_VERTICAL 200000   // microseconds between vertical updates

//_Bool print;

typedef struct Fruit
{
    int x, y;
} Fruit;

typedef struct Obstacle
{
    int x, y;
} Obstacle;

typedef struct SnakeSegment {
    int x, y;
    struct SnakeSegment* next;
} SnakeSegment;

typedef struct Snake
{
    SnakeSegment* head;
    int score;
    int direction;
    _Bool isDead;
    char idChar;
} Snake;

typedef struct GameData
{
    Snake** snakes;
    Fruit** fruits;
    Obstacle** obstacles;
    int width, height;
    int timer;
    int count_obstacles;
    int count_used_spaces;
} GameData;

Fruit* create_fruit(int x, int y);

void free_fruits(Fruit** fruits);

Obstacle* create_obstacle(int x, int y);

void load_map(const char* filename);

void draw_map(Obstacle** obstacles, int obstaclesCount);

SnakeSegment* create_segment(int x, int y);

Snake* create_snake(char idChar, int x, int y, int direction);

void free_snakes(Snake** snakes);

void add_segment(SnakeSegment* head);

void draw_borders();

void draw_score(Snake** snakes);

void draw_snake(Snake* snake);

void draw_snakes(Snake** snakes);

void draw_fruit(Fruit** fruits);

void erase_snake(SnakeSegment* snake);

void erase_fruit(Fruit* fruit);

void generate_food(Fruit** fruits, int index, Snake** snakes);

_Bool collidesWithFruit(Fruit** fruits, int x, int y, int* fruitIndex);

// TODO vymaz ak je to chujovina
void serialize_fruits(Fruit** fruits, char* buffer, int buffer_size);
void serialize_snake(Snake* snake, char* buffer, int buffer_size);
void serialize_game(Snake** snakes, Fruit** fruits, char* buffer, int buffer_size);
void deserialize_data(const char* data, Fruit** fruits, Snake** snakes);
int calculate_buffer_size(int width, int height, int num_fruits, int num_snakes);
void update(Fruit** fruits, Snake** snakes, Fruit** old_fruits, Snake** old_snakes);

void moveSnake(Snake** snakes, Fruit** fruits, _Bool _print);

void createGame(Snake** snakes, Fruit** fruits, _Bool _print);

void endGame(Snake** snakes, _Bool _print);

_Bool play(int* moves, Snake** snakes, Fruit** fruits, _Bool _print);