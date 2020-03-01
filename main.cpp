#include <SDL2/SDL.h>
#include "chip8.h"
#include <stdio.h>

SDL_Surface* gWindowSurface = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Window* gWindow = NULL;

int chip8Keys[16]={SDLK_x, 
					SDLK_1,
					SDLK_2,
					SDLK_3,

					SDLK_q, 
					SDLK_w, 
					SDLK_e,
					SDLK_a, 

					SDLK_s, 
					SDLK_d, 
					SDLK_z,
					SDLK_c, 
					
					SDLK_4, 
					SDLK_r, 
					SDLK_f,
					SDLK_v 
};

Uint32 nextEmuTick = 0, currentTime = 0;
bool autoStep = true;

bool setupGraphics();
void drawGraphics();
void close();

int main(int argc, char **argv){

	chip8 MyChip8;
	MyChip8.initialise();
	MyChip8.loadGame(argv[1]);
	setupGraphics();
	printf("loading done!\n");

	bool quit = false;
	SDL_Event e;
	while(!quit){
		currentTime = SDL_GetTicks();
		while(SDL_PollEvent(&e) != 0){
			if(e.type == SDL_QUIT){
				quit = true;
				break;
			} else if (e.type == SDL_KEYDOWN){
				if(e.key.keysym.sym == SDLK_p){
					MyChip8.printState();
				} else if (e.key.keysym.sym == SDLK_SPACE) {
					MyChip8.emulateCycle();
					printf("Step!\n");
				} else if (e.key.keysym.sym == SDLK_o){
					autoStep = !autoStep;
					printf("Toggling autostep!\n");
				} else {
					//printf("checking keys %d \n", e.key.keysym.sym);
					for(int k = 0; k < 16; ++k){
						//printf("key %d check!\n", chip8Keys[k]);
						if(e.key.keysym.sym == chip8Keys[k]){ 
							MyChip8.key[k]=true; 
							//printf("key %d down!\n", chip8Keys[k]);
						}
					}
				}
			} else if(e.type == SDL_KEYUP){
				for(int k = 0; k < 16; ++k){
					if(e.key.keysym.sym == chip8Keys[k]){ MyChip8.key[k]=false; }
				}
			}

		}
		// check  then emu cycle
		if(SDL_TICKS_PASSED(currentTime, nextEmuTick)){
			nextEmuTick = currentTime + 17;
			if(autoStep){MyChip8.emulateCycle();}
			// draw graphics
			if(MyChip8.getDrawFlag()){

				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0xAA, 0xFF);
				SDL_RenderClear(gRenderer);
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);			
				SDL_Rect pixel = {0, 0, 10, 10};
				//for(int i=0;i<64;++i){printf("%d",i);}
				for(int y = 0; y < 32; ++y){
					for(int x = 0; x < 64; ++x){
						if (MyChip8.gfx[y*64+x]){
							pixel.x = x * 10;
							pixel.y = y * 10;
							SDL_RenderFillRect( gRenderer, &pixel);
							//printf("X");
						} 
					}
				}

			}
			SDL_RenderPresent(gRenderer);
			MyChip8.resetDrawFlag();
		}	
	}
	close();
	return 0;
}

bool setupGraphics(){
	bool success = true;

	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("Could not initialise SDL: %s\n", SDL_GetError());
		success = false;
	} else {
		gWindow = SDL_CreateWindow("Chip8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640,320, SDL_WINDOW_SHOWN); 	
		if(gWindow == NULL){
			printf("Cant create window: %s\n", SDL_GetError());
			success = false;
		} else {
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if(gRenderer == NULL){
				printf("Cant create renderer: %s\n", SDL_GetError());
				success = false;
			}
		}
	}

	return success;
}

void drawGraphics(){


}

void close(){
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	SDL_DestroyRenderer(gRenderer);
	gRenderer = NULL;
	SDL_Quit;
}
