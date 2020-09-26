#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <libgs.h>
#include <stdio.h>
#include <rand.h>

#define OT_LENGTH 1 // ordering table length
#define PACKETMAX (300) // the max num of objects

GsOT      WorldOrderingTable[2]; // ordering table handlers
GsOT_TAG  OrderingTable[2][1<<OT_LENGTH]; // ordering tables
PACKET    GPUOutputPacket[2][PACKETMAX]; // GPU Packet work area

GsSPRITE image1[50]; // sprite handler

unsigned long __ramsize =   0x00200000; // force 2 megabytes of RAM
unsigned long __stacksize = 0x00004000; // force 16 kilobytes of stack

extern unsigned char image[]; // the name within the 'image.h' file that was converted from the .TIM using BIN2H or WinBin2Src (http://psxdev.net/tools.html)

long int image1_addr; // DRAM address storage of TIM file

// --------
// integers
// --------
int currentbuffer; // double buffer holder
int x = 0;
char version[5]="V69"; // version number

// ----------
// prototypes
// ----------
void initAddresses(); // initialise the addresses
void initGraphics(); // initialise the graphics
void clearVRAM(); // clear the VRAM
void initFont(); // initialise the font
void initImage(); // initialise the TIM image
void display(); // draw everything

// ----------------
// system debugging
// ----------------
const DEBUG = 1; // light/general debugging

void initAddresses()
{
	// the address to load to IF we are using hard coded address loading over the Xplorer
	// in the memp.map file that is defined in the makefile, our bss is 80048FC7.
	// we can load our image one bit after this as seen below...
	image1_addr = 0x80048FC8;

	if (DEBUG) printf("TIM=%x\n",image1_addr);
}

void initFont()
{
	FntLoad(960, 256); // load basic font pattern
	if (DEBUG) printf("Font Loaded (Framebuffer x=960, y=256)\n");
  
	// screen X,Y | max text length X,Y | automatic background clear 0,1 | max characters (eg: 50).
	SetDumpFnt(FntOpen(10, 10, 320, 240, 0, 512));
}

void initGraphics()
{
	int SCREEN_WIDTH, SCREEN_HEIGHT;

	// puts display mask into the status specified by the mask (0 not displayed, 1 displayed)
	SetDispMask(1);
   
	// resets the graphic system (0 full reset, 1 cancels the current drawing and flushes the command buffer, 3 initialises the drawing engine while preserving the current display environment)
	ResetGraph(0);

	// clear all VRAM contents
	clearVRAM();

	// automatic video mode control
	if (*(char *)0xbfc7ff52=='E') // SCEE string address
	{
		// PAL MODE
		SCREEN_WIDTH = 320;
		SCREEN_HEIGHT = 256;
		if (DEBUG) printf("Setting the PlayStation Video Mode to (PAL %dx%d)\n",SCREEN_WIDTH,SCREEN_HEIGHT,")");
		SetVideoMode(1);
		if (DEBUG) printf("Video Mode is (%d)\n",GetVideoMode());
	}
	else
	{
		// NTSC MODE
		SCREEN_WIDTH = 320;
		SCREEN_HEIGHT = 240;
		if (DEBUG) printf("Setting the PlayStation Video Mode to (NTSC %dx%d)\n",SCREEN_WIDTH,SCREEN_HEIGHT,")");
		SetVideoMode(0);
		if (DEBUG) printf("Video Mode is (%d)\n",GetVideoMode());
	}

	// set the graphics mode resolutions
	GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsNONINTER|GsOFSGPU, 1, 0);

	// set the top left coordinates of the two buffers in video memory
	GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT);

	// initialise the ordering tables
	WorldOrderingTable[0].length = OT_LENGTH;
	WorldOrderingTable[1].length = OT_LENGTH;
	WorldOrderingTable[0].org = OrderingTable[0];
	WorldOrderingTable[1].org = OrderingTable[1];
   
	GsClearOt(0,0,&WorldOrderingTable[0]);
	GsClearOt(0,0,&WorldOrderingTable[1]);

	if (DEBUG) printf("Graphics Initilised!\n");
}


void clearVRAM()
{
	RECT rectTL;
	
	setRECT(&rectTL, 0, 0, 1024, 512);
	ClearImage2(&rectTL, 0, 0, 0);
	
	DrawSync(0); // ensure that the VRAM is clear before exiting
    
	if (DEBUG) printf("VRAM cleared!\n");
	return;
}

void display()
{
	// refresh the font
	if (DEBUG) FntFlush(-1);

	// get the current buffer
	currentbuffer=GsGetActiveBuff();

	// setup the packet workbase
	GsSetWorkBase((PACKET*)GPUOutputPacket[currentbuffer]);

	// clear the ordering table
	GsClearOt(0,0,&WorldOrderingTable[currentbuffer]);

	// insert sprites into the ordering table
	for (x; x <= 50; x++){
		GsSortSprite(&image1[x], &WorldOrderingTable[currentbuffer], 0);
	}
	x = 0;

	// wait for all drawing to finish
	DrawSync(0);

	// wait for v_blank interrupt
	VSync(0);

	// flip double buffers
	GsSwapDispBuff();

	// clear the ordering table with a background color
	GsSortClear(0,0,0,&WorldOrderingTable[currentbuffer]);

	// Draw the ordering table for the currentbuffer
	GsDrawOt(&WorldOrderingTable[currentbuffer]);
}