/*
Copyright (C) 2012-2013 Andreas Schiffler

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "common.h"

#include "SDL2_gfxPrimitives.h"
#include "SDL2_rotozoom.h"

static CommonState *state;

/* Screen size */
#define WIDTH	DEFAULT_WINDOW_WIDTH
#define HEIGHT	DEFAULT_WINDOW_HEIGHT

/* Pause flag */
int pause = 0;

/* Done flag */
int done = 0;

/* Custom rotation setup */
double custom_angle=0.0;
double custom_fx=1.0;
double custom_fy=1.0;
int custom_smooth=0;

/* Delay between frames */
int delay;

/* Current message */
char *messageText;

#define POSITION_CENTER		1
#define POSITION_BOTTOMRIGHT	2

void RotatePicture (SDL_Surface *picture, int rotate, int flip, int smooth, int position) 
{
	SDL_Surface *rotozoom_picture;
	SDL_Texture *rotozoom_texture;
	SDL_Rect dest;
	int framecount, framemax, frameinc;
	double angle, zoomf, zoomfx, zoomfy;
	SDL_Renderer *renderer = state->renderers[0];
	SDL_Event event;

	printf("%s\n", messageText);

	/* Rotate and display the picture */
	framemax=4*360; 
	frameinc=1;
	for (framecount=-360; framecount<framemax && !done; framecount += frameinc) {
		while (SDL_PollEvent(&event)) CommonEvent(state, &event, &done);
		if ((framecount % 360)==0) frameinc++;
		SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
		SDL_RenderClear(renderer);
		zoomf=(float)(framecount+2*360)/(float)framemax;
		zoomf=1.5*zoomf*zoomf;
		/* Are we in flipping mode? */
		if (flip) {
			/* Flip X factor */
			if (flip & 1) {
				zoomfx=-zoomf;
			} else {
				zoomfx=zoomf;
			}
			/* Flip Y factor */
			if (flip & 2) {
				zoomfy=-zoomf;
			} else {
				zoomfy=zoomf;
			}
			angle=framecount*rotate;
			if (((framecount % 120)==0) || (delay>0)) {
				printf ("  Frame: %i   Rotate: angle=%.2f  Zoom: x=%.2f y=%.2f\n",framecount,angle,zoomfx,zoomfy);
			}
			if ((rotozoom_picture=rotozoomSurfaceXY (picture, angle, zoomfx, zoomfy, smooth))!=NULL) {
				switch (position) {
				case POSITION_CENTER:
					dest.x = (DEFAULT_WINDOW_WIDTH - rotozoom_picture->w)/2;
					dest.y = (DEFAULT_WINDOW_HEIGHT - rotozoom_picture->h)/2;
					break;
				case POSITION_BOTTOMRIGHT:
					dest.x = (DEFAULT_WINDOW_WIDTH/2) - rotozoom_picture->w;
					dest.y = (DEFAULT_WINDOW_HEIGHT/2) - rotozoom_picture->h;
					break;
				}
				dest.w = rotozoom_picture->w;
				dest.h = rotozoom_picture->h;

				/* Convert to texture and draw */
				rotozoom_texture = SDL_CreateTextureFromSurface(renderer, rotozoom_picture);
				SDL_FreeSurface(rotozoom_picture);
				if (!rotozoom_texture) {
					SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s\n", SDL_GetError());
					break;
				}				
				SDL_RenderCopy(renderer, rotozoom_texture, NULL, &dest);				
				SDL_DestroyTexture(rotozoom_texture);
			}
		} else {
			angle=framecount*rotate;
			if ((framecount % 120)==0) {
				printf ("  Frame: %i   Rotate: angle=%.2f  Zoom: f=%.2f \n",framecount,angle,zoomf);
			}
			if ((rotozoom_picture=rotozoomSurface (picture, angle, zoomf, smooth))!=NULL) {
				switch (position) {
				case POSITION_CENTER:
					dest.x = (DEFAULT_WINDOW_WIDTH - rotozoom_picture->w)/2;
					dest.y = (DEFAULT_WINDOW_HEIGHT - rotozoom_picture->h)/2;
					break;
				case POSITION_BOTTOMRIGHT:
					dest.x = (DEFAULT_WINDOW_WIDTH/2) - rotozoom_picture->w;
					dest.y = (DEFAULT_WINDOW_HEIGHT/2) - rotozoom_picture->h;
					break;
				}
				dest.w = rotozoom_picture->w;
				dest.h = rotozoom_picture->h;

				/* Convert to texture and draw */
				rotozoom_texture = SDL_CreateTextureFromSurface(renderer, rotozoom_picture);
				SDL_FreeSurface(rotozoom_picture);
				if (!rotozoom_texture) {
					SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s\n", SDL_GetError());
					break;
				}				
				SDL_RenderCopy(renderer, rotozoom_texture, NULL, &dest);				
				SDL_DestroyTexture(rotozoom_texture);
			}
		}


		stringRGBA(renderer, 8, 8, messageText, 255, 255, 255, 255);

		/* Display */
		SDL_RenderPresent(renderer);

		/* Maybe delay */
		if (delay>0) {
			SDL_Delay(delay);
		}
	}

	if (rotate) {
		/* Final display with angle=0 */
		SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
		SDL_RenderClear(renderer);
		if (flip) {
			if ((rotozoom_picture=rotozoomSurfaceXY (picture, 0.01, zoomfx, zoomfy, smooth))!=NULL) {
				dest.x = (DEFAULT_WINDOW_WIDTH - rotozoom_picture->w)/2;;
				dest.y = (DEFAULT_WINDOW_HEIGHT - rotozoom_picture->h)/2;
				dest.w = rotozoom_picture->w;
				dest.h = rotozoom_picture->h;

				/* Convert to texture and draw */
				rotozoom_texture = SDL_CreateTextureFromSurface(renderer, rotozoom_picture);
				SDL_FreeSurface(rotozoom_picture);
				if (!rotozoom_texture) {
					SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s\n", SDL_GetError());
					return;
				}				
				SDL_RenderCopy(renderer, rotozoom_texture, NULL, &dest);				
				SDL_DestroyTexture(rotozoom_texture);
			}		
		} else {
			if ((rotozoom_picture=rotozoomSurface (picture, 0.01, zoomf, smooth))!=NULL) {
				dest.x = (DEFAULT_WINDOW_WIDTH - rotozoom_picture->w)/2;;
				dest.y = (DEFAULT_WINDOW_HEIGHT - rotozoom_picture->h)/2;
				dest.w = rotozoom_picture->w;
				dest.h = rotozoom_picture->h;

				/* Convert to texture and draw */
				rotozoom_texture = SDL_CreateTextureFromSurface(renderer, rotozoom_picture);
				SDL_FreeSurface(rotozoom_picture);
				if (!rotozoom_texture) {
					SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s\n", SDL_GetError());
					return;
				}				
				SDL_RenderCopy(renderer, rotozoom_texture, NULL, &dest);				
				SDL_DestroyTexture(rotozoom_texture);
			}		
		}

		stringRGBA(renderer, 8, 8, messageText, 255, 255, 255, 255);

		/* Display */
		SDL_RenderPresent(renderer);

		/* Maybe delay */
		if (delay>0) {
			SDL_Delay(delay);
		}
	}

	/* Pause for a sec */
	SDL_Delay(1000);
}

void ZoomPicture (SDL_Surface *picture, int smooth) 
{
	SDL_Surface *rotozoom_picture;
	SDL_Texture *rotozoom_texture;
	SDL_Rect dest;
	int framecount, framemax, frameinc;
	double zoomxf,zoomyf;
	SDL_Renderer *renderer = state->renderers[0];
	SDL_Event event;

	printf("%s\n", messageText);

	/* Zoom and display the picture */
	framemax=4*360; frameinc=1;
	for (framecount=360; framecount<framemax && !done; framecount += frameinc) {
		while (SDL_PollEvent(&event)) CommonEvent(state, &event, &done);
		if ((framecount % 360)==0) frameinc++;
		SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
		SDL_RenderClear(renderer);
		zoomxf=(float)framecount/(float)framemax;
		zoomxf=1.5*zoomxf*zoomxf;
		zoomyf=0.5+fabs(1.0*sin((double)framecount/80.0));
		if ((framecount % 120)==0) {
			printf ("  Frame: %i   Zoom: x=%.2f y=%.2f\n",framecount,zoomxf,zoomyf);
		}
		if ((rotozoom_picture=zoomSurface (picture, zoomxf, zoomyf, smooth))!=NULL) {
			dest.x = (DEFAULT_WINDOW_WIDTH - rotozoom_picture->w)/2;;
			dest.y = (DEFAULT_WINDOW_HEIGHT - rotozoom_picture->h)/2;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;

			/* Convert to texture and draw */
			rotozoom_texture = SDL_CreateTextureFromSurface(renderer, rotozoom_picture);
			SDL_FreeSurface(rotozoom_picture);
			if (!rotozoom_texture) {
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s\n", SDL_GetError());
				return;
			}				
			SDL_RenderCopy(renderer, rotozoom_texture, NULL, &dest);				
			SDL_DestroyTexture(rotozoom_texture);
		}

		stringRGBA(renderer, 8, 8, messageText, 255, 255, 255, 255);

		/* Display */
		SDL_RenderPresent(renderer);

		/* Maybe delay */
		if (delay>0) {
			SDL_Delay(delay);
		}
	}

	/* Pause for a sec */
	SDL_Delay(1000);
}

void RotatePicture90Degrees (SDL_Surface *picture) 
{
	SDL_Surface *rotozoom_picture;
	SDL_Texture *rotozoom_texture;
	SDL_Rect dest;
	int framecount, framemax, frameinc;
	int numClockwiseTurns;
	SDL_Renderer *renderer = state->renderers[0];
	SDL_Event event;

	printf("%s\n", messageText);

	/* Rotate and display the picture */
	framemax = 21;
	frameinc = 1;
	numClockwiseTurns = -4;
	for (framecount=0; framecount<framemax && !done; framecount += frameinc) {
		while (SDL_PollEvent(&event)) CommonEvent(state, &event, &done);
		SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
		SDL_RenderClear(renderer);
		printf ("  Frame: %i   Rotate90: %i clockwise turns\n",framecount,numClockwiseTurns+4);
		if ((rotozoom_picture=rotateSurface90Degrees(picture, numClockwiseTurns))!=NULL) {
			dest.x = (DEFAULT_WINDOW_WIDTH - rotozoom_picture->w)/2;;
			dest.y = (DEFAULT_WINDOW_HEIGHT - rotozoom_picture->h)/2;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;

			/* Convert to texture and draw */
			rotozoom_texture = SDL_CreateTextureFromSurface(renderer, rotozoom_picture);
			SDL_FreeSurface(rotozoom_picture);
			if (!rotozoom_texture) {
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s\n", SDL_GetError());
				return;
			}				
			SDL_RenderCopy(renderer, rotozoom_texture, NULL, &dest);				
			SDL_DestroyTexture(rotozoom_texture);
		}

		stringRGBA(renderer, 8, 8, messageText, 255, 255, 255, 255);

		/* Display */
		SDL_RenderPresent(renderer);

		/* Always delay */
		SDL_Delay(333);
		if (delay>0) {
			SDL_Delay(delay);
		}

		numClockwiseTurns++;
	}

	/* Pause for a sec */
	SDL_Delay(1000);
}

#define ROTATE_OFF	0
#define ROTATE_ON	1

#define FLIP_OFF	0
#define FLIP_X		1
#define FLIP_Y		2
#define FLIP_XY		3

void CustomTest(SDL_Surface *picture, double a, double x, double y, int smooth){
	SDL_Surface *rotozoom_picture;
	SDL_Texture *rotozoom_texture;
	SDL_Rect dest;
	SDL_Renderer *renderer = state->renderers[0];

	printf("%s\n", messageText);
	printf("  Frame: C   Rotate: angle=%.2f  Zoom: fx=%.2f fy=%.2f \n",a,x,y);

	SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
	SDL_RenderClear(renderer);
	if ((rotozoom_picture=rotozoomSurfaceXY (picture, a, x, y, smooth))!=NULL) {
		dest.x = (DEFAULT_WINDOW_WIDTH - rotozoom_picture->w)/2;;
		dest.y = (DEFAULT_WINDOW_HEIGHT - rotozoom_picture->h)/2;
		dest.w = rotozoom_picture->w;
		dest.h = rotozoom_picture->h;

		/* Convert to texture and draw */
		rotozoom_texture = SDL_CreateTextureFromSurface(renderer, rotozoom_picture);
		SDL_FreeSurface(rotozoom_picture);
		if (!rotozoom_texture) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s\n", SDL_GetError());
			return;
		}				
		SDL_RenderCopy(renderer, rotozoom_texture, NULL, &dest);				
		SDL_DestroyTexture(rotozoom_texture);
	}

	/* Display */
	SDL_RenderPresent(renderer);

	/* Maybe delay */
	if (delay>0) {
		SDL_Delay(delay);
	}

	SDL_Delay(1000);		
}


void Draw (int start, int end)
{
	SDL_Surface *picture, *picture_again;
	char *bmpfile;
	SDL_Renderer *renderer = state->renderers[0];

	/* Define masking bytes */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	Uint32 rmask = 0xff000000; 
	Uint32 gmask = 0x00ff0000;
	Uint32 bmask = 0x0000ff00; 
	Uint32 amask = 0x000000ff;
#else
	Uint32 amask = 0xff000000; 
	Uint32 bmask = 0x00ff0000;
	Uint32 gmask = 0x0000ff00; 
	Uint32 rmask = 0x000000ff;
#endif

	/* --------- 8 bit tests -------- */

	if (start<=6) {

		/* Message */
		printf("8 bit tests ...\n");

		/* Load the image into a surface */
		bmpfile = "sample8.bmp";
		printf("Loading picture: %s\n", bmpfile);
		picture = SDL_LoadBMP(bmpfile);
		if ( picture == NULL ) {
			fprintf(stderr, "Couldn't load %s: %s\n", bmpfile, SDL_GetError());
			return;
		}

		if (start <= 1) {
			sprintf(messageText, "1.  rotozoom: Rotating and zooming");
			RotatePicture(picture,ROTATE_ON,FLIP_OFF,SMOOTHING_OFF,POSITION_CENTER);
		}
		if (end == 1) goto done8bit;

		if (start <= 2) {
			sprintf(messageText, "2.  rotozoom: Just zooming (angle=0)");
			RotatePicture(picture,ROTATE_OFF,FLIP_OFF,SMOOTHING_OFF,POSITION_CENTER);
			RotatePicture(picture,ROTATE_OFF,FLIP_OFF,SMOOTHING_OFF,POSITION_BOTTOMRIGHT);
		}
		if (end == 2) goto done8bit;

		if (start <= 3) {
			sprintf(messageText, "3.  zoom: Just zooming");
			ZoomPicture(picture,SMOOTHING_OFF);
		}
		if (end == 3) goto done8bit;

		if (start <= 4) {
			sprintf(messageText, "4.  rotozoom: Rotating and zooming, interpolation on but unused");
			RotatePicture(picture,ROTATE_ON,FLIP_OFF,SMOOTHING_ON,POSITION_CENTER);
		}
		if (end == 4) goto done8bit;

		if (start <= 5) {
			sprintf(messageText, "5.  rotozoom: Just zooming (angle=0), interpolation on but unused");
			RotatePicture(picture,ROTATE_OFF,FLIP_OFF,SMOOTHING_ON,POSITION_CENTER);
			RotatePicture(picture,ROTATE_OFF,FLIP_OFF,SMOOTHING_ON,POSITION_BOTTOMRIGHT);
		}
		if (end == 5) goto done8bit;

		if (start <= 6) {
			sprintf(messageText, "6.  zoom: Just zooming, interpolation on but unused");
			ZoomPicture(picture,SMOOTHING_ON);
		}
		if (end == 6) goto done8bit;

done8bit:

		/* Free the picture */
		SDL_FreeSurface(picture);

		if (end <= 6) return;
	}

	/* -------- 24 bit test --------- */

	if (start<=12) {

		/* Message */
		printf("24 bit tests ...\n");

		/* Load the image into a surface */
		bmpfile = "sample24.bmp";
		printf("Loading picture: %s\n", bmpfile);
		picture = SDL_LoadBMP(bmpfile);
		if ( picture == NULL ) {
			fprintf(stderr, "Couldn't load %s: %s\n", bmpfile, SDL_GetError());
			return;
		}

		/* Add white frame */
		rectangleColor(renderer, 0, 0, picture->w-1, picture->h-1, 0xffffffff);

		if (start <= 7) {
			sprintf(messageText, "7.  rotozoom: Rotating and zooming, no interpolation");
			RotatePicture(picture,ROTATE_ON,FLIP_OFF,SMOOTHING_OFF,POSITION_CENTER);
		}
		if (end == 7) goto done24bit;

		if (start <= 8) {
			sprintf(messageText, "8a.  rotozoom: Just zooming (angle=0), no interpolation, centered");
			RotatePicture(picture,ROTATE_OFF,FLIP_OFF,SMOOTHING_OFF,POSITION_CENTER);
			sprintf(messageText, "8b.  rotozoom: Just zooming (angle=0), no interpolation, corner");
			RotatePicture(picture,ROTATE_OFF,FLIP_OFF,SMOOTHING_OFF,POSITION_BOTTOMRIGHT);
			sprintf(messageText, "8c.  rotozoom: Just zooming (angle=0), X flip, no interpolation, centered");
			RotatePicture(picture,ROTATE_OFF,FLIP_X,SMOOTHING_OFF,POSITION_CENTER);
			sprintf(messageText, "8d.  rotozoom: Just zooming (angle=0), Y flip, no interpolation, centered");
			RotatePicture(picture,ROTATE_OFF,FLIP_Y,SMOOTHING_OFF,POSITION_CENTER);
			sprintf(messageText, "8e.  rotozoom: Just zooming (angle=0), XY flip, no interpolation, centered");
			RotatePicture(picture,ROTATE_OFF,FLIP_XY,SMOOTHING_OFF,POSITION_CENTER);
		}
		if (end == 8) goto done24bit;

		if (start <= 9) {
			sprintf(messageText, "9.  zoom: Just zooming, no interpolation");
			ZoomPicture(picture,SMOOTHING_OFF);
		}
		if (end == 9) goto done24bit;

		if (start <= 10) {
			sprintf(messageText, "10. rotozoom: Rotating and zooming, with interpolation");
			RotatePicture(picture,ROTATE_ON,FLIP_OFF,SMOOTHING_ON,POSITION_CENTER);
		}
		if (end == 10) goto done24bit;

		if (start <= 11) {
			sprintf(messageText, "11a. rotozoom: Just zooming (angle=0), with interpolation, centered");
			RotatePicture(picture,ROTATE_OFF,FLIP_OFF,SMOOTHING_ON,POSITION_CENTER);
			sprintf(messageText, "11b. rotozoom: Just zooming (angle=0), with interpolation, corner");
			RotatePicture(picture,ROTATE_OFF,FLIP_OFF,SMOOTHING_ON,POSITION_BOTTOMRIGHT);
			sprintf(messageText, "11c. rotozoom: Just zooming (angle=0), X flip, with interpolation, corner");
			RotatePicture(picture,ROTATE_OFF,FLIP_X,SMOOTHING_ON,POSITION_CENTER);
			sprintf(messageText, "11d. rotozoom: Just zooming (angle=0), Y flip, with interpolation, corner");
			RotatePicture(picture,ROTATE_OFF,FLIP_Y,SMOOTHING_ON,POSITION_CENTER);
			sprintf(messageText, "11e. rotozoom: Just zooming (angle=0), XY flip, with interpolation, corner");
			RotatePicture(picture,ROTATE_OFF,FLIP_XY,SMOOTHING_ON,POSITION_CENTER);
		}
		if (end == 11) goto done24bit;

		if (start <= 12) {
			sprintf(messageText, "12. zoom: Just zooming, with interpolation");
			ZoomPicture(picture,SMOOTHING_ON);
		}
		if (end == 12) goto done24bit;

done24bit:

		/* Free the picture */
		SDL_FreeSurface(picture);

		if (end <= 12) return;
	}

	/* -------- 32 bit test --------- */

	if (start<=16) {

		/* Message */
		printf("32 bit tests ...\n");

		/* Load the image into a surface */
		bmpfile = "sample24.bmp";
		printf("Loading picture: %s\n", bmpfile);
		picture = SDL_LoadBMP(bmpfile);
		if ( picture == NULL ) {
			fprintf(stderr, "Couldn't load %s: %s\n", bmpfile, SDL_GetError());
			return;
		}

		/* New source surface is 32bit with defined RGBA ordering */
		/* Much faster to do this once rather than the routine on the fly */
		fprintf(stderr,"Converting 24bit image into 32bit RGBA surface ...\n");
		picture_again = SDL_CreateRGBSurface(SDL_SWSURFACE, picture->w, picture->h, 32, rmask, gmask, bmask, amask);
		if (picture_again == NULL) goto done32bit;		
		SDL_BlitSurface(picture,NULL,picture_again,NULL);

		if (start <= 13) {
			sprintf(messageText, "13. Rotating and zooming, with interpolation (RGBA source)");
			RotatePicture(picture_again,ROTATE_ON,FLIP_OFF,SMOOTHING_ON,POSITION_CENTER);
		}
		if (end == 13) goto done32bit;

		if (start <= 14) {
			sprintf(messageText, "14. Just zooming (angle=0), with interpolation (RGBA source)");
			RotatePicture(picture_again,ROTATE_OFF,FLIP_OFF,SMOOTHING_ON,POSITION_CENTER);
			RotatePicture(picture_again,ROTATE_OFF,FLIP_OFF,SMOOTHING_ON,POSITION_BOTTOMRIGHT);
		}
		if (end == 14) goto done32bit;

		SDL_FreeSurface(picture_again);
		picture_again=NULL;

		/* New source surface is 32bit with defined ABGR ordering */
		/* Much faster to do this once rather than the routine on the fly */
		fprintf(stderr,"Converting 24bit image into 32bit ABGR surface ...\n");
		picture_again = SDL_CreateRGBSurface(SDL_SWSURFACE, picture->w, picture->h, 32, amask, bmask, gmask, rmask);
		if (picture_again == NULL) goto done32bit;		
		SDL_BlitSurface(picture,NULL,picture_again,NULL);

		if (start <= 14) {
			sprintf(messageText, "15. Rotating and zooming, with interpolation (ABGR source)");
			RotatePicture(picture_again,ROTATE_ON,FLIP_OFF,SMOOTHING_ON,POSITION_CENTER);
		}
		if (end == 14) goto done32bit;

		if (start <= 14) {
			sprintf(messageText, "16. Just zooming (angle=0), with interpolation (ABGR source)");
			RotatePicture(picture_again,ROTATE_OFF,FLIP_OFF,SMOOTHING_ON,POSITION_CENTER);
			RotatePicture(picture_again,ROTATE_OFF,FLIP_OFF,SMOOTHING_ON,POSITION_BOTTOMRIGHT);
		}
		if (end == 14) goto done32bit;


done32bit:

		/* Free the picture */
		SDL_FreeSurface(picture);
		if (picture_again) SDL_FreeSurface(picture_again);

		if (end <= 16) return;
	}

	/* -------- 32 bit flip test --------- */

	if (start<=22) {

		/* Message */
		printf("32 bit flip tests ...\n");

		/* Load the image into a surface */
		bmpfile = "sample24.bmp";
		printf("Loading picture: %s\n", bmpfile);
		picture = SDL_LoadBMP(bmpfile);
		if ( picture == NULL ) {
			fprintf(stderr, "Couldn't load %s: %s\n", bmpfile, SDL_GetError());
			return;
		}

		/* Excercise flipping functions on 32bit RGBA */
		printf("Converting 24bit image into 32bit RGBA surface ...\n");
		picture_again = SDL_CreateRGBSurface(SDL_SWSURFACE, picture->w, picture->h, 32, rmask, gmask, bmask, amask);
		if (picture_again == NULL) goto doneflip;
		SDL_BlitSurface(picture,NULL,picture_again,NULL);

		if (start <= 17) {
			sprintf(messageText, "17. Rotating with x-flip, no interpolation (RGBA source)");
			RotatePicture(picture_again,ROTATE_ON,FLIP_X,SMOOTHING_OFF,POSITION_CENTER);
		}
		if (end == 17) goto doneflip;

		if (start <= 18) {
			sprintf(messageText, "18. Rotating with y-flip, no interpolation");
			RotatePicture(picture_again,ROTATE_ON,FLIP_Y,SMOOTHING_OFF,POSITION_CENTER);
		}
		if (end == 18) goto doneflip;

		if (start <= 19) {
			sprintf(messageText, "19. Rotating with xy-flip, no interpolation");
			RotatePicture(picture_again,ROTATE_ON,FLIP_XY,SMOOTHING_OFF,POSITION_CENTER);
		}
		if (end == 19) goto doneflip;

		if (start <= 20) {
			sprintf(messageText, "20. Rotating with x-flip, with interpolation");
			RotatePicture(picture_again,ROTATE_ON,FLIP_X,SMOOTHING_ON,POSITION_CENTER);
		}
		if (end == 20) goto doneflip;

		if (start <= 21) {
			sprintf(messageText, "21. Rotating with y-flip, with interpolation");
			RotatePicture(picture_again,ROTATE_ON,FLIP_Y,SMOOTHING_ON,POSITION_CENTER);
		}
		if (end == 21) goto doneflip;

		if (start <= 22) {
			sprintf(messageText, "22. Rotating with xy-flip, with interpolation");
			RotatePicture(picture_again,ROTATE_ON,FLIP_XY,SMOOTHING_ON,POSITION_CENTER);
		}
		if (end == 22) goto doneflip;

doneflip:

		/* Free the pictures */
		SDL_FreeSurface(picture);
		if (picture_again) SDL_FreeSurface(picture_again);

		if (end <= 22) return;
	}

	if (start<=24) {

		/* Message */
		printf("Loading 24bit image\n");

		/* Load the image into a surface */
		bmpfile = "sample24.bmp";
		printf("Loading picture: %s\n", bmpfile);
		picture = SDL_LoadBMP(bmpfile);
		if ( picture == NULL ) {
			fprintf(stderr, "Couldn't load %s: %s\n", bmpfile, SDL_GetError());
			return;
		}

		/* Excercise flipping functions on 32bit RGBA */
		fprintf(stderr,"Converting 24bit image into 32bit RGBA surface ...\n");
		picture_again = SDL_CreateRGBSurface(SDL_SWSURFACE, picture->w, picture->h, 32, rmask, gmask, bmask, amask);
		SDL_BlitSurface(picture,NULL,picture_again,NULL);

		sprintf(messageText, "23. CustomTest, values from commandline (32bit)");
		CustomTest(picture_again, custom_angle, custom_fx, custom_fy, custom_smooth);

		SDL_FreeSurface(picture_again);

		/* Free the picture */
		SDL_FreeSurface(picture);

		/* Message */
		printf("Loading 8bit image\n");

		/* Load the image into a surface */
		bmpfile = "sample8.bmp";
		printf("Loading picture: %s\n", bmpfile);
		picture = SDL_LoadBMP(bmpfile);
		if ( picture == NULL ) {
			fprintf(stderr, "Couldn't load %s: %s\n", bmpfile, SDL_GetError());
			return;
		}

		sprintf(messageText, "24. CustomTest, values from commandline (8bit)");
		CustomTest(picture, custom_angle, custom_fx, custom_fy, custom_smooth);

		/* Free the picture */
		SDL_FreeSurface(picture);

		if (end <= 24) return;
	}

	if (start<=25) {

		/* Message */
		printf("Loading 24bit image\n");

		/* Load the image into a surface */
		bmpfile = "sample24.bmp";
		printf("Loading picture: %s\n", bmpfile);
		picture = SDL_LoadBMP(bmpfile);
		if ( picture == NULL ) {
			fprintf(stderr, "Couldn't load %s: %s\n", bmpfile, SDL_GetError());
			return;
		}

		/* New source surface is 32bit with defined RGBA ordering */
		printf("Converting 24bit image into 32bit RGBA surface ...\n");
		picture_again = SDL_CreateRGBSurface(SDL_SWSURFACE, picture->w, picture->h, 32, rmask, gmask, bmask, amask);
		if (picture_again == NULL) goto donerotate90;
		SDL_BlitSurface(picture,NULL,picture_again,NULL);

		/* Excercise rotate90 function on 32bit RGBA */
		sprintf(messageText, "25.  rotate90: Rotate 90 degrees clockwise (32bit)");
		RotatePicture90Degrees(picture_again);

donerotate90:

		/* Free the pictures */
		SDL_FreeSurface(picture);
		if (picture_again) SDL_FreeSurface(picture_again);

		if (end <= 25) return;
	}

	return;
}


/* ====== Main */

int main(int argc, char *argv[])
{
	int i, drawn, test;
	SDL_Event event;
	Uint32 then, now, frames;

	/* Initialize test framework */
	state = CommonCreateState(argv, SDL_INIT_VIDEO);
	if (!state) {
		return 1;
	}

	for (i = 1; i < argc;) {
		int consumed;

		consumed = CommonArg(state, i);
		if (consumed < 0) {
			fprintf(stderr,
				"Usage: %s %s\n",
				argv[0], CommonUsage(state));
			return 1;
		}
		i += consumed;
	}
	if (!CommonInit(state)) {
		return 2;
	}

	/* Create the windows and initialize the renderers */
	for (i = 0; i < state->num_windows; ++i) {
		SDL_Renderer *renderer = state->renderers[i];
		SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
		SDL_RenderClear(renderer);
	}

	/* Initialize random number generator */
	srand((unsigned int)time(NULL));

	/* Create common message buffer */
	messageText = (char *)SDL_malloc(1024);

	/* Main render loop */
	frames = 0;
	then = SDL_GetTicks();
	done = 0;
	drawn = 0;
	test = 0;
	while (!done) {
		/* Check for events */
		++frames;
		while (SDL_PollEvent(&event)) {
			CommonEvent(state, &event, &done);
			switch (event.type) {
			case SDL_KEYDOWN: {
				switch (event.key.keysym.sym) {
				case SDLK_SPACE: {
					/* Switch to next test */
					test++;
					drawn = 0;
					break;
								 }
				}
				break;
							  }
			case SDL_MOUSEBUTTONDOWN: {
				switch (event.button.button) {
				case SDL_BUTTON_LEFT: {
					/* Switch to next test */
					test++;
					drawn = 0;
					break;
									  }
				case SDL_BUTTON_RIGHT: {
					/* Switch to prev test */
					test--;
					drawn = 0;
					break;
									   }
				}
				break;
									  }
			}
		}

		if (!drawn) {
			/* Do all the drawing work */ 
			Draw(0, 25);
			drawn = 1;
		}

	}

	CommonQuit(state);

	/* Print out some timing information */
	now = SDL_GetTicks();
	if (now > then) {
		double fps = ((double) frames * 1000) / (now - then);
		printf("%2.2f frames per second\n", fps);
	}

	if (messageText) {
		free(messageText);
	}

	return 0;
}
