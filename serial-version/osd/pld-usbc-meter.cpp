/*
 * USB C current meter tap to PC via serial
 *
 * v1 20220715
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

char SEPARATOR_DP[] = ".";

struct serial_params_s {
	char *device;
	int fd, n;
	int cnt, size, s_cnt;
	struct termios oldtp, newtp;
};



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

	char *com_address;
	struct serial_params_s serial_params;


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

	g->com_address = NULL;

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

				case 'p':
					/*
					 * com port can be multiple things in linux
					 * such as /dev/ttySx or /dev/ttyUSBxx
					 */
					i++;
					if (i < argc) {
						g->serial_params.device = argv[i];
					} else {
						fprintf(stdout,"Insufficient parameters; -p <com port>\n");
						exit(1);
					}
					break;

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





/*
 * Default parameters are 2400:8n1, given that the multimeter
 * is shipped like this and cannot be changed then we shouldn't
 * have to worry about needing to make changes, but we'll probably
 * add that for future changes.
 *
 */
void open_port(struct serial_params_s *s) {
	int r; 

	s->fd = open( s->device, O_RDWR | O_NOCTTY |O_NDELAY );
	if (s->fd <0) {
		perror( s->device );
	}

	fcntl(s->fd,F_SETFL,0);
	tcgetattr(s->fd,&(s->oldtp)); // save current serial port settings 
	tcgetattr(s->fd,&(s->newtp)); // save current serial port settings in to what will be our new settings
	cfmakeraw(&(s->newtp));
	s->newtp.c_cflag = B9600 | CS8 |  CREAD | CRTSCTS ; // Adjust the settings to suit our BSIDE-ADM20 / 2400-8n1

	r = tcsetattr(s->fd, TCSANOW, &(s->newtp));
	if (r) {
		fprintf(stderr,"%s:%d: Error setting terminal (%s)\n", FL, strerror(errno));
		exit(1);
	}
}


int16_t twos2dec_16( unsigned char *raw ) {

	/*
	 * Convert 2's complement big-endian 16-bit integer
	 * in to a 16-bit int (intel)
	 *
	 * We have to first see if the high bit is set to 
	 * indicate a negative number, if it is, we need
	 * to invert the bits in both bytes, assemble in to
	 * a little-endian integer, and *then* add 1 to the
	 * result and cast back in to a 16-bit integer.
	 *
	 * For non-negative numbers, we can just assemble 
	 * the two bytes in to  little-endian form and 
	 * cast to 16-bit int.
	 *
	 */

	uint32_t tmp = 0;
	int16_t result = 0;

	if ((raw[0] & 0x80) == 0x80) {
		// high bit is set, negative number
		result = -(((raw[0]<<8|raw[1]) ^0xffff) +1); 
		/*
		 * Below is the original code, kept in here so others
		 * can know what's going on with the 'compact' version
		 * above
		 *
		raw[0] = ~(raw[0]);
		raw[1] = ~(raw[1]);
		tmp = (raw[0] << 8) | raw[1];
		result = (int16_t)(tmp +1);
		result = -result;
		 *
		 */

	} else {
		// no high bit set, just assemble normally
		result = (int16_t)((raw[0] << 8) | raw[1]);
	}

	return result;
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


	open_port(&g.serial_params);

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
	int end_of_frame_received = 0;
	int valid_data = 0;

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
		char serial_buffer[20];
		char *d = &serial_buffer[0];


		if (g.debug) { fprintf(stderr,"DATA START: "); }
		end_of_frame_received = 0;
		i = 0;
		do {
			char temp_char = 0;
			bytes_read = read(g.serial_params.fd, &temp_char, 1);
			if (bytes_read > 0) {
				if (temp_char == '\n') {
					end_of_frame_received = 1;
					fprintf(stderr, " *EOF* " );
					d[i] = 0;
					break;
				}
				d[i] = temp_char;
				if (g.debug) { fprintf(stderr,"%02x ", d[i]); }
				i++;
			}
		} while ((bytes_read > 0) && (i < sizeof(serial_buffer)) && (!end_of_frame_received));

		if (g.debug) { fprintf(stderr,":END [%d bytes], EOF=%d, bytes_read=%ld\n", i, end_of_frame_received, bytes_read); }

		if (bytes_read == 0) {
			usleep(250000);
		}

		if (!end_of_frame_received)  {
			continue;
		}


		char tmpc = serial_buffer[4];
		serial_buffer[4] = '\0';
		shunt_voltage = strtol(serial_buffer, NULL, 16);
		serial_buffer[4] = tmpc;
		bus_voltage = strtol(serial_buffer+4, NULL, 16);


		if (bus_voltage & 0x01) valid_data = 0;
		else valid_data = 1;

		bus_voltage = bus_voltage >> 3;

#define VOLTAGE_QUANTA 0.004

		current = shunt_voltage /0.05 /100000;  // 50mR shunt
		voltage = bus_voltage *VOLTAGE_QUANTA;

		if ((valid_data == 0) || (voltage < 0.2)) {
			snprintf(line1, sizeof(line1), " -");
			snprintf(line2, sizeof(line2), " -");
		}
		else {
			snprintf(line1, sizeof(line1), " %6.3fV", voltage);
			snprintf(line2, sizeof(line2), " %6.3fA", current);
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