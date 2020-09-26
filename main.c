// header files
#include "const.h"

// image files
#include "image.h"
#include "miku.h"
#include "title.h"
#include "leek.h"
#include "boof.h"
#include "meek.h"

// integers
int game = 0;
int pos = 0;
int frame = 0;
int timer = 0;
int moving = 1;
int moving2 = 1;
int gameInit = 0;
int score = 0;
int lives = 3;
int highScore = 69;
int menu = 0;
int o = 4; int l = 10;
int down = 0;
int reset = 0;

// position integers
int mikuX  = 150, mikuY  = 240;
int miku2X = 192, miku2Y = 240;
int leekDistanceX, leekDistanceY;
int boofDistanceX, boofDistanceY;

int main()
{
	// init functions
	PadInit(0); // initializes the gamepad
    ResetCallback(); // initialises all system callbacks
    initGraphics(); // this will initialise the graphics
    initFont(); // initialise the psy-q debugging font
	
	// init images
	initImage(miku,  150, 240, 0,  896, 0);
	initImage(miku,  212, 240, 1,  896, 0);
	initImage(title, 50,  30,  2,  576, 0);
	initImage(meek,  320, 80,  3,  576, 61);
	initImage(leek,  34,  134, 4,  832, 0);
	initImage(image, 0,   0,   16, 320, 0);
	initImage(image, 208, 0,   17, 320, 0);
	initImage(image, 0,   182, 18, 320, 0);
	initImage(image, 208, 182, 19, 320, 0);
	
	// resize miku
	image1[0].w = 21;
	image1[1].w = 21;

	// update loop
    while(1) update();

	// everything's fucked
    ResetGraph(3); // set the video mode back
    PadStopCom(); // stop controller communications    
    return 0;
}

// load tim image from ram into frame buffer slot
void initImage(img, x, y, i, tx, ty)
{
   RECT rect; // RECT structure
   GsIMAGE tim_data; // holds tim graphic info
   	
   // put data from tim file into rect         
   GsGetTimInfo ((u_long *)(img+4),&tim_data);

   // load the image into the frame buffer
   rect.x = tim_data.px; // tim start X coord
   rect.y = tim_data.py; // tim start Y coord
   rect.w = tim_data.pw; // data width
   rect.h = tim_data.ph; // data height
   
   // load the tim data into the frame buffer
   LoadImage(&rect, tim_data.pixel);       

   // load the CLUT into the frame buffer
   rect.x = tim_data.cx; // x pos in frame buffer
   rect.y = tim_data.cy; // y pos in frame buffer
   rect.w = tim_data.cw; // width of CLUT
   rect.h = tim_data.ch; // height of CLUT
   
   // load data into frame buffer (DMA from DRAM to VRAM)
   LoadImage(&rect, tim_data.clut);                

   // initialise sprite
   image1[i].attribute=0x2000000; // 16 bit CLUT, all options off (0x1 = 8-bit, 0x2 = 16-bit)
   image1[i].x = x; // draw at x coord
   image1[i].y = y; // draw at y coord
   image1[i].w = tim_data.pw; // width of sprite
   image1[i].h = tim_data.ph; // height of sprite
   image1[i].tpage=GetTPage(1, 2, tx, 0); // texture page | texture mode (0 4-bit, 1 8-bit, 2 16-bit), semi-transparency rate, texture x, texture y in the framebuffer
   image1[i].r = 128; // RGB Data
   image1[i].g = 128;
   image1[i].b = 128;
   image1[i].u=0; // position within timfile for sprite
   image1[i].v=ty;                                           
   image1[i].cx = tim_data.cx; // CLUT location x coord
   image1[i].cy = tim_data.cy; // CLUT location y coord
   image1[i].r=image1[i].g=image1[i].b=128; // normal luminosity
   image1[i].mx = 0; // rotation x coord
   image1[i].my = 0; // rotation y coord
   image1[i].scalex = ONE; // scale x coord (ONE = 100%)
   image1[i].scaley = ONE; // scale y coord (ONE = 100%)
   image1[i].rotate = 0; // degrees to rotate   

   // wait for all drawing to finish
   DrawSync(0);
}

void update()
{
	// display shit
	display();
	
		//if(PadRead(0) & _PAD(0, PADLdown)) FntPrint("pad 0");
		//if(PadRead(0) & _PAD(1, PADLdown)) FntPrint("pad 1");
	
	// frame timer
	timer++;
	if (timer == 15)
	{
		frameUpdate();
		timer = 0;
	}
	
	// background scroll position
	pos++;
	if (pos > 26) pos = 0;
	
	// i'm bad at coding
	image1[16].x  = 0   + pos - 28; image1[16].y  = 0  + pos - 28;
	image1[17].x  = 208 + pos - 28; image1[17].y  = 0  + pos - 28;
	image1[18].x = 0    + pos - 28; image1[18].y = 182 + pos - 28;
	image1[19].x = 208  + pos - 28; image1[19].y = 182 + pos - 28;
	
	// game states
	if(game == 0)
	{
		FntPrint("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n      1 PLAYER\n\n      2 PLAYER");
		
		// menu position
		if (menu == 0) image1[4].y = 134;
		else image1[4].y = 150;

		// menu miku
		if (image1[3].x >= 140) image1[3].x -= 3;
		
		// navigate menu
		if(PadRead(0) & PADLdown || PadRead(0) & PADLup)
		{
			if (down == 0)
			{
				if (menu == 0) menu = 1;
				else menu = 0;
				down = 1;
			}
		}
		else down = 0;
		
		if(PadRead(0) & PADRdown) printf("idk");
		else reset = 0;
		
		// begin game
		if(PadRead(0) & PADRdown && reset == 0)
		{
			image1[2].w = 0; // hide title
			image1[4].x = rng(300); image1[4].y = -19; // repurpouse menu leek
			image1[3].w = 0; // hide menu miku
			if (menu == 1) mikuX = 106;
			
			// restore objects
			image1[5].w  = 20;
			image1[6].w  = 20;
			image1[7].w  = 20;
			image1[10].w = 13;
			image1[11].w = 13;
			
			game++;
		}
	}
	
	else if(game == 1)
	{	
		// enterance cutscene
		if (menu == 0)
		{
			if(mikuY >= 150) mikuY--;
			else game++;
		}
		else
		{
			if(mikuY >= 150)
			{
				mikuY--;
				miku2Y--;
			}
			else game++;
		}
	}
	
	else if(game == 2)
	{
		
		if(gameInit == 0)
		{
			initImage(leek, rng(300), -rng(300) - 19, 5,   832, 0);
			initImage(leek, rng(300), -rng(300) - 19, 6,   832, 0);
			initImage(leek, rng(300), -rng(300) - 19, 7,   832, 0);
			//initImage(leek, rng(300), -rng(300) - 19, 8,   832, 0);
			//initImage(leek, rng(300), -rng(300) - 19, 9,   832, 0);
			initImage(boof, rng(307), -rng(300) - 13, 10, 832, 19);
			initImage(boof, rng(307), -rng(300) - 13, 11, 832, 19);
			//initImage(boof, rng(307), -rng(300) - 13, 12, 832, 19);
			//initImage(boof, rng(307), -rng(300) - 13, 13, 832, 19);
			//initImage(boof, rng(307), -rng(300) - 13, 14, 832, 19);
			//initImage(boof, rng(307), -rng(300) - 13, 15, 832, 19);
			image1[10].w = 13;
			image1[11].w = 13;
			//image1[12].w = 13;
			gameInit++;
		}
		
		// control miku pos
		if(PadRead(0) & _PAD(0, PADLdown))  mikuY++;
		if(PadRead(0) & _PAD(0, PADLup))    mikuY--;
		if(PadRead(0) & _PAD(0, PADLleft))  mikuX--;
		if(PadRead(0) & _PAD(0, PADLright)) mikuX++;
		
		// p2
		if (menu == 1)
		{
			if(PadRead(0) & _PAD(1, PADLdown))  miku2Y++;
			if(PadRead(0) & _PAD(1, PADLup))    miku2Y--;
			if(PadRead(0) & _PAD(1, PADLleft))  miku2X--;
			if(PadRead(0) & _PAD(1, PADLright)) miku2X++;
		}
		
		// check if miku is moving
		if(PadRead(0) & PADLdown || PadRead(0) & PADLup
		|| PadRead(0) & PADLleft || PadRead(0) & PADLright)
		{
			moving = 1;
		}
		else moving = 0;
		
		// check if miku2 is moving
		if(PadRead(1) & PADLdown || PadRead(1) & PADLup
		|| PadRead(1) & PADLleft || PadRead(1) & PADLright)
		{
			moving2 = 1;
		}
		else moving2 = 0;
		
		// miku loop screen
		if(mikuX == -21) mikuX = 319;
		if(mikuX == 320) mikuX = -20;
		if(mikuY == -20) mikuY = 239;
		if(mikuY == 240) mikuY = -19;

		if (menu == 0)
		{
			// display score
			FntPrint("\nSCORE: %d", score);
			FntPrint("                     LIVES: %d", lives);
			FntPrint("\n\nHI-SCORE: %d", highScore);
		}
		else
		{
			FntPrint("\nP1 SCORE: %d", score);
			FntPrint("               P2 SCORE: %d", score);
			FntPrint("\n\nP1 LIVES: %d", lives);
			FntPrint("               P2 LIVES: %d", lives);
		}
		
		if (lives == 0) game++;
	}
	else if (game == 3)
	{
		// miku fucking died
		FntPrint("\nMIKU IS DEAD.");
		image1[0].u=84;
		
		// reset game
		if(PadRead(0) & PADRdown)
		{
			// reset variables
			mikuX = 150; miku2X = 192;
			mikuY = 240; miku2Y = 240;
			lives = 3; score = 0;
			image1[4].x = 34;
			down = 1;
			reset = 1;
			
			// reset menu hud
			image1[2].w = 221;
			image1[3].w = 226;
			image1[3].x = 320;
			
			// hide objects
			image1[5].w  = 0; resetLeek(5);
			image1[6].w  = 0; resetLeek(6);
			image1[7].w  = 0; resetLeek(7);
			image1[10].w = 0; resetBoof(10);
			image1[11].w = 0; resetBoof(11);
			
			game = 0;
		}
	}
	
	if (game >= 2)
	{
		// loop objects
		for (o; o <= 7; o++)
		{
			// leek movement
			image1[o].y++;
			if (image1[o].y >= 240) resetLeek(o);
				
			// leek collision
			leekDistanceX = mikuX - image1[o].x;
			leekDistanceY = mikuY - image1[o].y;
				
			if (leekDistanceX > -20 && leekDistanceX < 20
			&&  leekDistanceY > -19 && leekDistanceY < 19
			&&  game == 2)
			{
				// reset leek
				resetLeek(o);
				score++;
				if (score >= highScore) highScore = score;
			}
		}
		o = 4;
		for (l; l <= 11; l++)
		{	
			// boof movement
			image1[l].y++;
			if (image1[l].y == 240) resetBoof(l);
				
			// boof collision
			boofDistanceX = mikuX - image1[l].x;
			boofDistanceY = mikuY - image1[l].y;
				
			if (boofDistanceX > -13 && boofDistanceX < 13
			&&  boofDistanceY > -13 && boofDistanceY < 13
			&&  game == 2)
			{
				// reset boof
				resetBoof(l);
				lives--;
			}
			
			// animate boof
			if (frame == 0) image1[l].u = 0;
			else image1[l].u=13;
		}
		l = 10;
	}
	
	// animate miku
	if (game < 3)
	{
		if (moving == 1)
		{
			if (frame == 0) image1[0].u = 42;
			else image1[0].u=63;
		}
		else if (moving == 0)
		{
			if (frame == 0) image1[0].u = 0;
			else image1[0].u=21;
		}
	}
	
	// animate miku 2
	if (moving2 == 1)
	{
		if (frame == 0) image1[1].u = 42;
		else image1[1].u=63;
	}
	else
	{
		if (frame == 0) image1[1].u = 0;
		else image1[1].u=21;
	}
	
	// move miku
	image1[0].x = mikuX;  image1[0].y = mikuY;
	image1[1].x = miku2X; image1[1].y = miku2Y;
}

// check current frame
void frameUpdate()
{	
	if (frame == 0) frame = 1;
	else frame = 0;
}

// generate random number
int rng(max)
{
	int num = 0;
	num = (rand()%max);
	return num;
}

// reset objects
void resetLeek(leek)
{
	image1[leek].y = -rng(300) - 19;
	image1[leek].x = rng(300);
}

void resetBoof(boof)
{
	image1[boof].y = -rng(50) - 13;
	image1[boof].x = rng(307);
}