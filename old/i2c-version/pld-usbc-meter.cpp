/*
 * USB C current meter tap to PC via I2C / USB
 *
 * v1 20220705
 *
 * Written by Paul L Daniels (pldaniels@gmail.com)
 *
 */

#include <SDL.h>
#include <SDL_ttf.h>

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "i2c.h"

#define FL __FILE__,__LINE__

/*
 * Should be defined in the Makefile to pass to the compiler from
 * the github build revision
 *
 */
#ifndef BUILD_VER 
#define BUILD_VER 000
#endif

#ifndef BUILD_DATE
#define BUILD_DATE " "
#endif

#define SSIZE 1024

#define INTERFRAME_SLEEP	200000 // 0.2 seconds
											 //
#define INA219_ID 0x40
#define INA219_FRAME_SLEEP 80000 // a little more than the 67ms suggested by datasheet

char SEPARATOR_DP[] = ".";

#define CCDBG if (1)

struct glb {
	uint8_t debug;
	uint8_t quiet;
	uint16_t flags;
	uint16_t error_flag;
	char *output_file;
	char *device;

	char get_func[20];
	char meas_value[20];

	int busid;
	int interval;
	int font_size;
	int window_width, window_height;
	int wx_forced, wy_forced;
	SDL_Color font_color_volts, font_color_amps, background_color;
};

/*
 * A whole bunch of globals, because I need
 * them accessible in the Windows handler
 *
 * So many of these I'd like to try get away from being
 * a global.
 *
 */
struct glb *glbs;

/*
 * Test to see if a file exists
 *
 * Does not test anything else such as read/write status
 *
 */
bool fileExists(const char *filename) {
	struct stat buf;
	return (stat(filename, &buf) == 0);
}


char digit( unsigned char dg ) {

	int d;
	char g;

	switch (dg) {
		case 0x30: g = '0'; d = 0; break;
		case 0x31: g = '1'; d = 1; break;
		case 0x32: g = '2'; d = 2; break;
		case 0x33: g = '3'; d = 3; break;
		case 0x34: g = '4'; d = 4; break;
		case 0x35: g = '5'; d = 5; break;
		case 0x36: g = '6'; d = 6; break;
		case 0x37: g = '7'; d = 7; break;
		case 0x38: g = '8'; d = 8; break;
		case 0x39: g = '9'; d = 9; break;
		case 0x3E: g = 'L'; d = 0; break;
		case 0x3F: g = ' '; d = 0; break;
		default: g = ' ';
	}

	return g;
}

/*-----------------------------------------------------------------\
  Date Code:	: 20180127-220248
  Function Name	: init
  Returns Type	: int
  ----Parameter List
  1. struct glb *g ,
  ------------------
  Exit Codes	:
  Side Effects	:
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int init(struct glb *g) {
	g->debug = 0;
	g->quiet = 0;
	g->flags = 0;
	g->error_flag = 0;
	g->output_file = NULL;
	g->interval = 50000;
	g->device = NULL;
	g->busid = -1;

	g->font_size = 60;
	g->window_width = 400;
	g->window_height = 100;
	g->wx_forced = 0;
	g->wy_forced = 0;

	g->font_color_volts =  { 10, 200, 10 };
	g->font_color_amps =  { 200, 200, 10 };
	g->background_color = { 0, 0, 0 };

	return 0;
}

void show_help(void) {
	fprintf(stdout,"PLD USB-C Meter Multimeter OSD\r\n"
			"By Paul L Daniels / pldaniels@gmail.com\r\n"
			"Build %d / %s\r\n"
			"\r\n"
			"\t-h: This help\r\n"
			"\t-d: debug enabled\r\n"
			"\t-q: quiet output\r\n"
			"\t-v: show version\r\n"
			"\t-z <font size in pt>\r\n"
			"\t-cv <volts colour, a0a0ff>\r\n"
			"\t-ca <amps colour, ffffa0>\r\n"
			"\t-cb <background colour, 101010>\r\n"
			"\t-t <interval>, sleep delay between samples, default 100,000us)\r\n"
			"\t--timeout <seconds>, time to wait for serial response, default 3 seconds\r\n"
			"\r\n"
			, BUILD_VER
			, BUILD_DATE 
			);
} 


/*-----------------------------------------------------------------\
  Date Code:	: 20180127-220258
  Function Name	: parse_parameters
  Returns Type	: int
  ----Parameter List
  1. struct glb *g,
  2.  int argc,
  3.  char **argv ,
  ------------------
  Exit Codes	:
  Side Effects	:
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int parse_parameters(struct glb *g, int argc, char **argv ) {
	int i;

	for (i = 0; i < argc; i++) {
		if (argv[i][0] == '-') {
			/* parameter */
			switch (argv[i][1]) {

				case 'b':
					i++;
					g->busid = strtol(argv[i], NULL, 10);
					break;

				case '-':
//					if (strncmp(argv[i], "--timeout", 9)==0) {
//						i++;
//						g->serial_timeout = strtol(argv[i], NULL, 10);
//					}
					break;

				case 'h':
					show_help();
					exit(1);
					break;

				case 'z':
					i++;
					if (i < argc) {
						g->font_size = atoi(argv[i]);
					} else {
						fprintf(stdout,"Insufficient parameters; -z <font size pts>\n");
						exit(1);
					}
					break;

				case 'p':
					/*
					 * com port can be multiple things in linux
					 * such as /dev/ttySx or /dev/ttyUSBxx
					 */
					i++;
					if (i < argc) {
						g->device = argv[i];
					} else {
						fprintf(stdout,"Insufficient parameters; -p <usb TMC port ie, /dev/usbtmc2>\n");
						exit(1);
					}
					break;

				case 'o':
					/* 
					 * output file where this program will put the text
					 * line containing the information FlexBV will want 
					 *
					 */
					i++;
					if (i < argc) {
						g->output_file = argv[i];
					} else {
						fprintf(stdout,"Insufficient parameters; -o <output file>\n");
						exit(1);
					}
					break;

				case 'd': g->debug = 1; break;

				case 'q': g->quiet = 1; break;

				case 'v':
							 fprintf(stdout,"Build %d\r\n", BUILD_VER);
							 exit(0);
							 break;

				case 't':
							 i++;
							 g->interval = atoi(argv[i]);
							 break;

				case 'c':
							 int a,b,c;
							 if (argv[i][2] == 'v') {
								 i++;
								 sscanf(argv[i], "%2hhx%2hhx%2hhx"
										 , &g->font_color_volts.r
										 , &g->font_color_volts.g
										 , &g->font_color_volts.b
										 );

							 } else if (argv[i][2] == 'a') {
								 i++;
								 sscanf(argv[i], "%2hhx%2hhx%2hhx"
										 , &g->font_color_amps.r
										 , &g->font_color_amps.g
										 , &g->font_color_amps.b
										 );

							 } else if (argv[i][2] == 'b') {
								 i++;
								 sscanf(argv[i], "%2hhx%2hhx%2hhx"
										 , &(g->background_color.r)
										 , &(g->background_color.g)
										 , &(g->background_color.b)
										 );

							 }
							 break;

				case 'w':
							 if (argv[i][2] == 'x') {
								 i++;
								 g->wx_forced = atoi(argv[i]);
							 } else if (argv[i][2] == 'y') {
								 i++;
								 g->wy_forced = atoi(argv[i]);
							 }
							 break;


				default: break;
			} // switch
		}
	}

	return 0;
}

uint8_t a2h( uint8_t a ) {
	a -= 0x30;
	if (a < 10) return a;
	a -= 7;
	return a;
}

int is_ina219( int file ) {
	uint8_t config_buf[2];

	i2c_read_register(file, 0x00, config_buf, 2);

	fprintf(stderr,"Testing register 0:  %02x %02x\n", config_buf[0], config_buf[1]);
	if (config_buf[0] == 0x39 && config_buf[1] == 0x9F) return 1;
	else return 0;
}


/*-----------------------------------------------------------------\
  Date Code:	: 20180127-220307
  Function Name	: main
  Returns Type	: int
  ----Parameter List
  1. int argc,
  2.  char **argv ,
  ------------------
  Exit Codes	:
  Side Effects	:
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int main ( int argc, char **argv ) {

	SDL_Event event;
	SDL_Surface *surface, *surface_2;
	SDL_Texture *texture, *texture_2;

	char linetmp[SSIZE]; // temporary string for building main line of text

	struct glb g;        // Global structure for passing variables around
	int i = 0;           // Generic counter
	char temp_char;        // Temporary character
	char tfn[4096];
	bool quit = false;
	int flag_ol = 0;
	int ina219_file;

	glbs = &g;

	/*
	 * Initialise the global structure
	 */
	init(&g);

	/*
	 * Parse our command line parameters
	 */
	parse_parameters(&g, argc, argv);

	fprintf(stdout,"START\n");

	/* 
	 * check paramters
	 *
	 */
	if (g.font_size < 10) g.font_size = 10;
	if (g.font_size > 200) g.font_size = 200;

	if (g.output_file) snprintf(tfn,sizeof(tfn),"%s.tmp",g.output_file);

	// If the user has requested a specific bus
	//

	int bid = g.busid;
	int found = 0;
	if (g.busid >= 0) {
		bid = g.busid;
	} else bid = 0;

	do {
		CCDBG fprintf(stderr,"Trying i2c bus ID %d...", bid);
		ina219_file = i2c_open_bus(bid);
		if (ina219_file > 0) {
			CCDBG fprintf(stderr,"Bus ID %d: OPEN...", bid);
			if (i2c_set_device(ina219_file, INA219_ID) >= 0) {
				CCDBG fprintf(stderr, "Setting device ID to %0x...", INA219_ID);
				if (is_ina219(ina219_file)) {
					CCDBG fprintf(stderr,"Success, INA219 found on i2c bus %d\n", bid);
					found = 1;
					break;
				} else {
					CCDBG fprintf(stderr,"Not able to find INA219\n");
				}
			}
			i2c_close_bus(ina219_file);
		}
		CCDBG fprintf(stderr,"fail\n");
		if (g.busid >= 0) break;
		bid++;
	} while ((bid < 16) && (!found));

	if (!found) {
		fprintf(stderr,"Could not find the INA219 chip.\n");
		exit(1);
	}

	/*
	 * Reset the chip
	 *
	 */
	i2c_write_register_byte(ina219_file, 0x00, 0x80); // 


	/*
	 * Setup SDL2 and fonts
	 *
	 */

	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	TTF_Font *font = TTF_OpenFont("RobotoMono-Regular.ttf", g.font_size);
	TTF_Font *font_small = TTF_OpenFont("RobotoMono-Regular.ttf", g.font_size/4);

	/*
	 * Get the required window size.
	 *
	 * Parameters passed can override the font self-detect sizing
	 *
	 */
	TTF_SizeText(font, " 00.00000V ", &g.window_width, &g.window_height);
	g.window_height *= 1.85;

	if (g.wx_forced) g.window_width = g.wx_forced;
	if (g.wy_forced) g.window_height = g.wy_forced;

	SDL_Window *window = SDL_CreateWindow("PLD USB-C Meter", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, g.window_width, g.window_height, 0);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	if (!font) {
		fprintf(stderr,"Error trying to open font :( \r\n");
		exit(1);
	}

	/* Select the color for drawing. It is set to red here. */
	SDL_SetRenderDrawColor(renderer, g.background_color.r, g.background_color.g, g.background_color.b, 255 );

	/* Clear the entire screen to our selected color. */
	SDL_RenderClear(renderer);

		char spinner[]="|/|\\";
		uint8_t count = 0;

	/*
	 *
	 * Parent will terminate us... else we'll become a zombie
	 * and hope that the almighty PID 1 will reap us
	 *
	 */
	while (!quit) {
		char line1[1024];
		char line2[1024];
		char buf_func[100];
		char buf_value[100];
		char buf_units[10];
		char buf_multiplier[10];

		char *p, *q;
		double td = 0.0;
		double v = 0.0;
		int ev = 0;
		uint8_t range;
		uint8_t dpp = 0;
		ssize_t bytes_read = 0;
		ssize_t sz;


		while (SDL_PollEvent(&event)) {
			switch (event.type)
			{
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_q) quit = true;
					break;
				case SDL_QUIT:
					quit = true;
					break;
			}
		}

		linetmp[0] = '\0';

			/*
			 * Read the 6 bytes of data for the 
			 * x, y and z axis.
			 *
			 * NOTE: with this chip, the y axis
			 * is actually the last pair, not the
			 * second pair that you might assume
			 * by default.
			 *
			 */
			uint8_t config_buf[2];
			uint8_t shunt_voltage_buf[2];
			uint8_t bus_voltage_buf[2];
			int16_t shunt_voltage;
			uint16_t bus_voltage;
			double current;
			double voltage;
			i2c_read_register(ina219_file, 0x00, config_buf, 2); // shunt voltage
			i2c_read_register(ina219_file, 0x01, shunt_voltage_buf, 2); // shunt voltage
			i2c_read_register(ina219_file, 0x02, bus_voltage_buf, 2); // bus voltage
			shunt_voltage = i2c_twos2dec_16(shunt_voltage_buf);
			//bus_voltage = i2c_twos2dec_16(bus_voltage_buf);
			bus_voltage = bus_voltage_buf[1] +(bus_voltage_buf[0]<<8);
			bus_voltage = bus_voltage >> 3;

			current = shunt_voltage *0.188;
			voltage = bus_voltage /250.0;

			if ((current < 0)||(voltage < 0.2)) {
			       	snprintf(line1, sizeof(line1), " -");
				snprintf(line2, sizeof(line2), " -");
			}
			else {
				snprintf(line1, sizeof(line1), " %6.3fV", voltage);
			       	snprintf(line2, sizeof(line2), " %6.3fA", current/1000);
			}

			fprintf(stderr,"%c\r", spinner[count %( sizeof(spinner)-1)]);

		/*
		 *
		 * END OF DATA ACQUISITION
		 *
		 */

		{
			int texW = 0;
			int texH = 0;
			int texW2 = 0;
			int texH2 = 0;
			SDL_RenderClear(renderer);
			surface = TTF_RenderUTF8_Blended(font, line1, g.font_color_volts);
			texture = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
			SDL_Rect dstrect = { 0, 0, texW, texH };
			SDL_RenderCopy(renderer, texture, NULL, &dstrect);

			surface_2 = TTF_RenderUTF8_Blended(font, line2, g.font_color_amps);
			texture_2 = SDL_CreateTextureFromSurface(renderer, surface_2);
			SDL_QueryTexture(texture_2, NULL, NULL, &texW2, &texH2);
			dstrect = { 0, texH -(texH /5), texW2, texH2 };
			SDL_RenderCopy(renderer, texture_2, NULL, &dstrect);

			SDL_RenderPresent(renderer);

			SDL_DestroyTexture(texture);
			SDL_FreeSurface(surface);
			if (1) {
				SDL_DestroyTexture(texture_2);
				SDL_FreeSurface(surface_2);
			}

			if (g.error_flag) {
				sleep(1);
			} else {
				usleep(g.interval);
			}
			
			if (current < 0 || voltage < 0.1) {
				usleep(g.interval *5);
			}


		}


		count++; // will loop, and that's okay
			 //
	} // while(1)

	TTF_CloseFont(font);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();

	return 0;

}
