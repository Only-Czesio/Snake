#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <time.h>
#include <vector>

#define SCREEN_HEIGHT 400
#define SCREEN_WIDTH 400
#define frames 10

void must_init(bool test, const char *description)
{
    if (test)
    {
        return;
    }
    printf("Couldn't initialize %s\n", description);
    exit(1);
}

typedef struct Apple
{
    int size = 10;
    int posX, posY;
    bool eaten;
} Apple;

enum Direction
{
    UP,
    RIGHT,
    DOWN,
    LEFT
};

struct SnakeSegment
{
    int posX, posY;
};

struct Snake
{
    Direction dir;
    int body_count;
    std::vector<SnakeSegment> body;
};

void initApple(Apple *fruit, int posX, int posY, bool eaten)
{
    fruit->posX = posX;
    fruit->posY = posY;
    fruit->eaten = eaten;
}

void spawnApple(Apple *fruit, int posX, int posY)
{
    initApple(fruit, posX, posY, false);
    al_draw_filled_rectangle(posX, posY, posX + 10, posY + 10, al_map_rgb(255, 255, 0));
}

int pickAppleLocation()
{
    // Return a random value aligned to a 10-pixel grid within screen bounds
    return (rand() % (SCREEN_WIDTH / 10)) * 10;
}

void initSnake(Snake *snake, int posX, int posY, Direction dir, int count)
{
    snake->dir = dir;

    snake->body_count = count;

    snake->body.clear();

    // Initialize snake with 3 segments
    snake->body.push_back({posX, posY});      // Head
    snake->body.push_back({posX - 10, posY}); // Body
    snake->body.push_back({posX - 20, posY}); // Tail
}
void killSnake(Snake *snakePlayer)
{
    // Check if the snake's head collides with any other segment of its body
    for (size_t i = 1; i < snakePlayer->body.size(); ++i)
    {
        // Check if the head position matches any of the body segments
        if (snakePlayer->body[0].posX == snakePlayer->body[i].posX &&
            snakePlayer->body[0].posY == snakePlayer->body[i].posY)
        {
            // Snake has collided with itself
            Direction randomDirection = static_cast<Direction>(rand() % 4);
            initSnake(snakePlayer, 200, 200, randomDirection, 3); // Re-initialize the snake
        }
    }
}

void moveSnake(Snake *snake)
{
    // Move the snake based on its direction

    // Move each segment to the position of the previous one
    for (int i = snake->body.size() - 1; i > 0; --i)
    {
        snake->body[i].posX = snake->body[i - 1].posX;
        snake->body[i].posY = snake->body[i - 1].posY;
    }

    // Move the head based on the direction
    switch (snake->dir)
    {
    case UP:
        snake->body[0].posY -= 10;
        break;
    case DOWN:
        snake->body[0].posY += 10;
        break;
    case LEFT:
        snake->body[0].posX -= 10;
        break;
    case RIGHT:
        snake->body[0].posX += 10;
        break;
    }

    // Keep the snake within bounds
    if (snake->body[0].posX >= SCREEN_WIDTH)
        snake->body[0].posX = 0;
    if (snake->body[0].posX < 0)
        snake->body[0].posX = SCREEN_WIDTH - 10;
    if (snake->body[0].posY >= SCREEN_HEIGHT)
        snake->body[0].posY = 0;
    if (snake->body[0].posY < 0)
        snake->body[0].posY = SCREEN_HEIGHT - 10;
}

void growSnake(Snake *snake)
{
    // Get the last segment (tail)
    snake->body_count++;
    SnakeSegment tail = snake->body.back();

    // Add a new segment at the tail's position
    snake->body.push_back(tail);
}

void drawSnake(Snake *snake)
{
    for (const auto &segment : snake->body)
    {
        al_draw_filled_rectangle(segment.posX, segment.posY, segment.posX + 10, segment.posY + 10, al_map_rgb(0, 255, 0));
    }
}

// keyboard stuff
#define KEY_SEEN 1
#define KEY_RELEASED 2
unsigned char key[ALLEGRO_KEY_MAX];

void keyboard_init()
{
    memset(key, 0, sizeof(key));
}

void keyboard_update(ALLEGRO_EVENT *event)
{
    switch (event->type)
    {
    case ALLEGRO_EVENT_TIMER:
        for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
            key[i] &= KEY_SEEN;
        break;

    case ALLEGRO_EVENT_KEY_DOWN:
        key[event->keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
        break;
    case ALLEGRO_EVENT_KEY_UP:
        key[event->keyboard.keycode] &= KEY_RELEASED;
        break;
    }
}

int main()
{
    al_init();
    must_init(al_init(), "allegro");

    al_install_keyboard();
    must_init(al_install_keyboard(), "keyboard");

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / frames);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    must_init(queue, "queue");

    ALLEGRO_DISPLAY *disp = al_create_display(SCREEN_HEIGHT, SCREEN_WIDTH);
    must_init(disp, "display");

    ALLEGRO_FONT *font = al_create_builtin_font();
    must_init(font, "font");

    must_init(al_init_primitives_addon(), "primitives");

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT event;

    // Seed random number generator once at the beginning of the program
    srand(time(NULL));

    al_start_timer(timer);

    Apple fruit;
    initApple(&fruit, pickAppleLocation(), pickAppleLocation(), false);

    Direction randomDirection = static_cast<Direction>(rand() % 4);
    Snake snakePlayer;
    initSnake(&snakePlayer, 200, 200, randomDirection, 3);

    while (!done)
    {
        al_wait_for_event(queue, &event);

        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:

            killSnake(&snakePlayer);
            moveSnake(&snakePlayer);

            // Check if the snake eats the apple
            if (snakePlayer.body[0].posX == fruit.posX && snakePlayer.body[0].posY == fruit.posY)
            {
                // Snake eats the apple, spawn a new one
                fruit.posX = pickAppleLocation();
                fruit.posY = pickAppleLocation();

                // Grow the snake
                growSnake(&snakePlayer);
            }

            if (key[ALLEGRO_KEY_UP])
                snakePlayer.dir = UP;
            if (key[ALLEGRO_KEY_DOWN])
                snakePlayer.dir = DOWN;
            if (key[ALLEGRO_KEY_LEFT])
                snakePlayer.dir = LEFT;
            if (key[ALLEGRO_KEY_RIGHT])
                snakePlayer.dir = RIGHT;

            if (key[ALLEGRO_KEY_ESCAPE])
            {
                done = true;
            }
            redraw = true;

            for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
                key[i] &= KEY_SEEN;

            redraw = true;
            break;

        case ALLEGRO_EVENT_KEY_DOWN:
            key[event.keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
            break;

        case ALLEGRO_EVENT_KEY_UP:
            key[event.keyboard.keycode] &= KEY_RELEASED; // Clear key release state
            break;

        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            done = true;
            break;
        }
        if (done)
            break;

        if (redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));

            al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH / 2, 10, ALLEGRO_ALIGN_CENTER, "SnakePoints: %ld", snakePlayer.body.size());

            spawnApple(&fruit, fruit.posX, fruit.posY); // Draw the apple
            drawSnake(&snakePlayer);                    // Draw the snake

            al_flip_display();
            redraw = false;
        }
    }
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_font(font);
    al_destroy_display(disp);

    return 0;
}
