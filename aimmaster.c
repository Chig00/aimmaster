#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <SDL.h>

#define TITLE "Aim Master by Chigozie Agomo"
#define WINDOW_W 800
#define WINDOW_H 800
#define QUIT SDL_SCANCODE_ESCAPE
#define RESET SDL_SCANCODE_R
#define TARGET_BMP "target.bmp"
#define TARGET_W 50
#define TARGET_H 50
#define INIT_MAX_AGE 10
#define HALF_AGE 2
#define BASE_SPEED 0.5
#define SPEED_GRAD 0.05

const double PI = acos(-1);

typedef struct {
	SDL_Surface* surface;
	SDL_Rect rect;
	double pos[2];
	double vel[2];
	double birth;
	double max_age;
	bool click;
} Target;

int rand_int(int low, int high) {
	return low + (high - low + 1) * rand() / (RAND_MAX + 1);
}

double calc_speed(int score) {
	return BASE_SPEED * (1 + score * SPEED_GRAD);
}

double calc_max_age(int score) {
	return INIT_MAX_AGE * HALF_AGE / (double) (score + HALF_AGE);
}

double get_time() {
	return (double) clock() / CLOCKS_PER_SEC;
}

void create_target(Target* target, int score) {
	target->pos[0] = rand_int(0, WINDOW_W - TARGET_W);
	target->pos[1] = rand_int(0, WINDOW_H - TARGET_H);
	target->rect.x = target->pos[0];
	target->rect.y = target->pos[1];
	double angle = rand_int(0, 359) * PI / 180;
	target->vel[0] = calc_speed(score) * cos(angle);
	target->vel[1] = calc_speed(score) * sin(angle);
	target->max_age = calc_max_age(score);
	target->birth = get_time();
}

bool hit_check(SDL_Point point, SDL_Rect rect) {
	int cx = rect.x + rect.w / 2;
	int cy = rect.y + rect.h / 2;
	int x = point.x - cx;
	int y = point.y - cy;
	
	return pow((pow(x, 2) + pow(y, 2)), 0.5) <= rect.w / 2;
}

bool update_target(Target* target, int* score, SDL_Surface* display) {
	if (get_time() >= target->max_age + target->birth) {
		return true;
	}
	
	target->pos[0] += target->vel[0];
	target->pos[1] += target->vel[1];
	
	if (target->pos[0] < 0) {
		target->pos[0] = 0;
	}
	
	else if (target->pos[0] > WINDOW_W - TARGET_W) {
		target->pos[0] = WINDOW_W - TARGET_W;
	}
	
	if (target->pos[1] < 0) {
		target->pos[1] = 0;
	}
	
	else if (target->pos[1] > WINDOW_H - TARGET_H) {
		target->pos[1] = WINDOW_H - TARGET_H;
	}
	
	target->rect.x = target->pos[0];
	target->rect.y = target->pos[1];
	
	SDL_BlitSurface(target->surface, NULL, display, &target->rect);
	
	SDL_Point mouse;
	
	if (SDL_GetMouseState(&mouse.x, &mouse.y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		if (!target->click) {
			if (hit_check(mouse, target->rect)) {
				++*score;
				create_target(target, *score);
			}
			
			else {
				return true;
			}
			
			target->click = true;
		}
	}
	
	else {
		target->click = false;
	}
	
	return false;
}

int main(int argc, char** argv) {
	srand(time(NULL));
	
	SDL_Window* window = SDL_CreateWindow(
		TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN
	);
	SDL_Surface* display = SDL_GetWindowSurface(window);
	
	Target target = {
		.surface = SDL_LoadBMP(TARGET_BMP),
		.rect = {
			.w = TARGET_W,
			.h = TARGET_H
		}
	};
	
	bool quit = false;
	
	while (!quit) {
		bool end = false;
		int score = 0;
		
		create_target(&target, score);
		target.click = false;
		
		while (!quit && !end) {
			quit = SDL_GetKeyboardState(NULL)[QUIT];
			end = SDL_GetKeyboardState(NULL)[RESET];
			
			int bg = 255 * (1 - (get_time() - target.birth) / target.max_age);
			SDL_FillRect(display, NULL, SDL_MapRGB(display->format, bg ,bg ,bg));
			
			if (update_target(&target, &score, display)) {
				printf("Score: %d\n", score);
				
				while (!quit && !end) {
					quit = SDL_GetKeyboardState(NULL)[QUIT];
					end = SDL_GetKeyboardState(NULL)[RESET];
					SDL_PumpEvents();
				}
			}
			
			SDL_UpdateWindowSurface(window);
			SDL_PumpEvents();
		}
		
		while (SDL_GetKeyboardState(NULL)[RESET]) {
			SDL_PumpEvents();
		}
	}
}