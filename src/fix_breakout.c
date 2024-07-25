#include "raylib.h"
//#include "zozlib.js"
void raylib_js_set_entry(void (*entry)(void));

#ifndef PLATFORM_WEB
#include <time.h>
#include <stdio.h>
#endif //PLATFORM_WEB

#define ARR_SIZE(arr) sizeof(arr)/sizeof(arr[0])

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 800
#define CRT_BLACK (Color){18, 18, 18, 255}

typedef struct 
{
	Rectangle rect;
	Color colour;
	_Bool active;
} Entity;

#define PLAYER_WIDTH 100
#define PLAYER_HEIGHT 20
#define PLAYER_SPEED 500.0f
Entity player;

#define BALL_SIZE 20
#define BALL_SPEED 300.0f
Entity ball;
Vector2 ball_velocity = (Vector2){0, 0};
_Bool ball_launched = false;

#define BRICK_COLUMN_COUNT 8
#define BRICK_ROW_COUNT 5
#define BRICK_MARGIN 2
#define BRICK_WIDTH (SCREEN_WIDTH - BRICK_MARGIN) / BRICK_COLUMN_COUNT - BRICK_MARGIN
#define BRICK_HEIGHT 20
#define BRICK_COUNT BRICK_COLUMN_COUNT * BRICK_ROW_COUNT
Entity bricks[BRICK_COUNT];

Color brick_color_lut[] = {RED, ORANGE, YELLOW, GREEN, BLUE};

_Bool aabb_check(Rectangle first, Rectangle second)
{
	return  first.x < second.x     +  second.width  &&
			first.x + first.width  >  second.x      &&
			first.y < second.y     +  second.height &&
			first.y + first.height >  second.y;
}

void draw_entity(Entity *e)
{
	DrawRectangleRec(e->rect, e->colour);
}

void game_frame(void)
{
	float delta_time = GetFrameTime();
	// Update
	do
	{
		Vector2 desired_movement = (Vector2){0.0f, 0.0f};
		float movement_input = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
		desired_movement.x = movement_input * PLAYER_SPEED * delta_time;

		if (!ball_launched && IsKeyPressed(KEY_SPACE)) 
		{
			float rand_ofs = (float)GetRandomValue(-50, 50) / 100.0f;

			ball_velocity.x = movement_input + rand_ofs * BALL_SPEED ;
			ball_velocity.y = -BALL_SPEED;
			ball_launched = true;
		}

		player.rect.x += desired_movement.x;
		if(player.rect.x < 0) player.rect.x = 0;
		if(player.rect.x + player.rect.width > SCREEN_WIDTH) player.rect.x = SCREEN_WIDTH - player.rect.width;

		if(!ball_launched)
		{
			ball.rect.x = (player.rect.x + player.rect.x + player.rect.width) / 2.0f - (ball.rect.width / 2.0f);
			ball.rect.y = SCREEN_HEIGHT - (3.5f * PLAYER_HEIGHT);
			break;
		}

		if(ball.rect.x < 0) ball_velocity.x = -ball_velocity.x;
		if(ball.rect.x + ball.rect.width > SCREEN_WIDTH) ball_velocity.x = -ball_velocity.x;
		if(ball.rect.y < 0) ball_velocity.y = -ball_velocity.y;
		if(ball.rect.y > SCREEN_HEIGHT)
		{
			ball_launched = false;
			break;
		}

		if(aabb_check(ball.rect, player.rect)) ball_velocity.y = -ball_velocity.y;

		for(int i = 0; i < BRICK_COUNT; i += 1)
		{
			if(!bricks[i].active) continue; //Already inactive
			if(!aabb_check(ball.rect, bricks[i].rect)) continue; //Did not hit the brick

			bricks[i].active = false;
			//This is just a dumb aabb check again, but this time with side info
			if(ball.rect.y < bricks[i].rect.y + bricks[i].rect.height) 
			{
				ball_velocity.y = -ball_velocity.y; 
				continue;
			}
			if(ball.rect.y + ball.rect.height > bricks[i].rect.y) 
			{
				ball_velocity.y = -ball_velocity.y; 
				continue;
			}
			if(ball.rect.x < bricks[i].rect.x + bricks[i].rect.width)
			{
				ball_velocity.x = -ball_velocity.x;
				continue;
			}
			if(ball.rect.x + ball.rect.width > bricks[i].rect.x)
			{
				ball_velocity.x = -ball_velocity.x; 
				continue;
			}
		}

		ball.rect.x += ball_velocity.x * delta_time;
		ball.rect.y += ball_velocity.y * delta_time;
		
	}while(0);

	// Draw
	{
		BeginDrawing();
		{
			ClearBackground(CRT_BLACK);
			#ifndef PLATFORM_WEB
			DrawFPS(0, 0);
			#endif //PLATFORM_WEB
			draw_entity(&ball);
			draw_entity(&player);

			for(int i = 0; i < BRICK_COUNT; i += 1)
			{
				if(bricks[i].active)
				draw_entity(&bricks[i]);
			}
		}
		EndDrawing();
	}
	
}

int main(void)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "WASMout");

	SetTargetFPS(60);

	player = (Entity)
	{
		(Rectangle)
		{
			(SCREEN_WIDTH / 2.0f) - (PLAYER_WIDTH / 2.0f), 
			SCREEN_HEIGHT - (2.0f * PLAYER_HEIGHT), 
			PLAYER_WIDTH, PLAYER_HEIGHT
		}, 
		RED,
		true,
	};
	ball = (Entity)
	{
		(Rectangle)
		{
			SCREEN_WIDTH / 2.0f - BALL_SIZE / 2.0f, 
			SCREEN_HEIGHT - (3.5f * PLAYER_HEIGHT), 
			BALL_SIZE, BALL_SIZE
		}, 
		RED,
		true
	};

	Vector2 offset = (Vector2){BRICK_MARGIN, BRICK_MARGIN};
	for(int y = 0; y < BRICK_ROW_COUNT; y += 1)
	{
		for(int x = 0; x < BRICK_COLUMN_COUNT; x += 1)
		{
			Entity *brick = &bricks[x + BRICK_COLUMN_COUNT * y];
			brick->rect.x = offset.x;
			brick->rect.y = offset.y;
			brick->rect.width = (float)BRICK_WIDTH;
			brick->rect.height = BRICK_HEIGHT;
			brick->colour = brick_color_lut[y];
			brick->active = true;

			offset.x += (float)BRICK_WIDTH + BRICK_MARGIN;
		}

		offset.x = BRICK_MARGIN;
		offset.y += BRICK_HEIGHT + BRICK_MARGIN;
	}

#ifdef PLATFORM_WEB
	raylib_js_set_entry(game_frame);
#else
	SetRandomSeed(time(NULL));
	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{

		game_frame();
	}

	CloseWindow();        // Close window and OpenGL context
#endif

	return 0;
}
