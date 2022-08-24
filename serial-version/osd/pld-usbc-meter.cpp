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
#include <sys/file.h>
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

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

char SEPARATOR_DP[] = ".";

struct serial_params_s {
	 char device[MAX_PATH];
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

	 double current_threshold = 0.0;
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
								snprintf(g->serial_params.device, sizeof(g->serial_params.device), "%s",argv[i]);
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
						  if (strncmp(argv[i], "--current-threshold", strlen("--current-threshold"))==0) {
								int tmp;
								i++;
								tmp = strtol(argv[i], NULL, 10);
								g->current_threshold = 0.001 * tmp;
								//																fprintf(stderr,"Current threshold set to %dmA ~ %0.3fA\n", tmp, g->current_threshold);
						  }
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
 * Default parameters are 9600:8n1, given that the multimeter
 * is shipped like this and cannot be changed then we shouldn't
 * have to worry about needing to make changes, but we'll probably
 * add that for future changes.
 *
 */

#define PORT_OK 0
#define PORT_CANT_LOCK 10
#define PORT_INVALID 11
#define PORT_CANT_SET 12
#define PORT_NO_SUCCESS -1

int open_port(struct serial_params_s *s) {
	 int r; 

	 s->fd = open( s->device, O_RDWR | O_NOCTTY |O_NDELAY );
	 if (s->fd <0) {
		  return PORT_INVALID;
		  perror( s->device );
	 }

	 r = flock(s->fd, LOCK_EX | LOCK_NB);
	 if (r == -1) {
		  fprintf(stderr, "%s:%d: Unable to set lock on %s, Error '%s'\n", FL, s->device, strerror(errno) );
		  return PORT_CANT_LOCK;
	 }
	 if (r == 0) {
		  fcntl(s->fd,F_SETFL,0);
		  tcgetattr(s->fd,&(s->oldtp)); // save current serial port settings 
		  tcgetattr(s->fd,&(s->newtp)); // save current serial port settings in to what will be our new settings
		  cfmakeraw(&(s->newtp));
		  s->newtp.c_cflag = B9600 | CS8 |  CREAD | CRTSCTS ; // Adjust the settings to suit our BSIDE-ADM20 / 2400-8n1

		  r = tcsetattr(s->fd, TCSANOW, &(s->newtp));
		  if (r) {
				fprintf(stderr,"%s:%d: Error setting terminal (%s)\n", FL, strerror(errno));
				return PORT_CANT_SET;
				//								exit(1);
		  }
	 }
	 return PORT_OK;
}


int find_port( struct serial_params_s *s ) {

	 for ( int port_number = 0; port_number < 10; port_number++ ) {
		  snprintf(s->device, sizeof(s->device), "/dev/ttyUSB%d", port_number);
		  fprintf(stderr,"Testing port %s\n", s->device);
		  int r = open_port( s );
		  if (r == PORT_OK ) {
				int frame_size_count_started = 0;
				int end_of_frame_received = 0;
				int frame_size = 0;
				int bytes_read = 0;
				char serial_buffer[20];
				fd_set set;
				struct timeval timeout;

				FD_ZERO(&set);
				FD_SET(s->fd, &set);

				timeout.tv_sec = 0;
				timeout.tv_usec = 200000;

				do {
					 char temp_char = 0;
					 int rv;

					 bytes_read = 0;
					 rv = select(s->fd +1, &set, NULL, NULL, &timeout);
					 if (rv == -1) {
						  break;
					 } else if (rv == 0 ) {
						  break;
					 } else bytes_read = read(s->fd, &temp_char, 1);

					 if (bytes_read > 0) {
						  if (temp_char == '\n') {
								if (frame_size_count_started == 0) {
									 frame_size_count_started = 1;
									 frame_size = 0;
								} else {
									 end_of_frame_received = 1;
									 break;
								}
						  } else {
								if (frame_size_count_started) frame_size++;
						  }
					 }
				} while ((bytes_read > 0) && (frame_size < sizeof(serial_buffer)) && (!end_of_frame_received));
				if (frame_size == 8) {
					 fprintf(stderr,"Port %s selected\n", s->device);
					 return PORT_OK;
				}

				close(s->fd);
		  } // port OK
	 } // for each port
	 return PORT_NO_SUCCESS;
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

	 uint32_t data_buffer_size;

	 //double vdata[data_buffer_size];
	 //double adata[data_buffer_size];
	 double *vdata;
	 double *adata;
	 int32_t buffer_start, buffer_end;
	 double vscale = 1.0;
	 double ascale = 1.0;

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

	 if (g.debug) fprintf(stdout,"START\n");

	 /* 
	  * check paramters
	  *
	  */
	 if (g.font_size < 10) g.font_size = 10;
	 if (g.font_size > 200) g.font_size = 200;

	 if (g.output_file) snprintf(tfn,sizeof(tfn),"%s.tmp",g.output_file);


	 int find_port_result = find_port(&g.serial_params);
	 if (find_port_result != PORT_OK) {
		  fprintf(stderr,"Unable to locate port with valid device\n");
		  exit(1);
	 }

	 //open_port(&g.serial_params);

	 /*
	  * Setup SDL2 and fonts
	  *
	  */

	 SDL_Init(SDL_INIT_VIDEO);
	 TTF_Init();
	 TTF_Font *font = TTF_OpenFont("RobotoMono-Bold.ttf", g.font_size);
	 if ( !font ) { fprintf(stderr,"Ooops - something went wrong when trying to create the %d px font", g.font_size ); exit(1); }
	 TTF_Font *font_half = TTF_OpenFont("RobotoMono-Bold.ttf", g.font_size/2);
	 if ( !font ) { fprintf(stderr,"Ooops - something went wrong when trying to create the %d px font", g.font_size/2 ); exit(1); }
	 TTF_Font *font_small = TTF_OpenFont("RobotoMono-Regular.ttf", g.font_size/4);
	 if ( !font ) { fprintf(stderr,"Ooops - something went wrong when trying to create the %d px font", g.font_size/4 ); exit(1); }

	 /*
	  * Get the required window size.
	  *
	  * Parameters passed can override the font self-detect sizing
	  *
	  */
	 int data_w, data_h;
	 TTF_SizeText(font_half, "00.000", &data_w, &data_h);
	 TTF_SizeText(font, " 00.00000V", &g.window_width, &g.window_height);
	 g.window_width += data_w;
	 g.window_height *= 1.85;

	 if (g.wx_forced) g.window_width = g.wx_forced;
	 if (g.wy_forced) g.window_height = g.wy_forced;

	 data_buffer_size = g.window_width;
	 vdata = (double *)malloc( sizeof(double) *data_buffer_size );
	 if (!vdata) {
		  fprintf(stderr,"Ooops, cannot allocate %ld bytes for voltage buffer\n", g.window_width *sizeof(double));
		  exit(1);
	 }
	 adata = (double *)malloc( sizeof(double) *data_buffer_size );
	 if (!adata) {
		  fprintf(stderr,"Ooops, cannot allocate %ld bytes for current buffer\n", g.window_width *sizeof(double));
		  exit(1);
	 }
	 if (g.debug) fprintf(stderr,"%ld bytes allocated in %d entries for v and a data\n", sizeof(double)*data_buffer_size, data_buffer_size);
	 for (int x = 0; x < data_buffer_size; x++) {
		  adata[x] = 0.0;
		  vdata[x] = 0.0;
	 }


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

	 vscale = g.window_height / 22; // get pixels per V
	 ascale = g.window_height / 5.5; // get pixels per A

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
	 buffer_start = buffer_end = 0;

	 double v_min, v_max;
	 double a_min, a_max;




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

		  fd_set set;
		  struct timeval timeout;
		  timeout.tv_sec = 0;
		  timeout.tv_usec = 200000;


		  FD_ZERO(&set);
		  FD_SET(g.serial_params.fd, &set);

		  if (g.debug) { fprintf(stderr,"DATA START: "); }

		  end_of_frame_received = 0;
		  i = 0;
		  do {
				char temp_char = 0;
				int rv;

				bytes_read = 0;
				rv = select(g.serial_params.fd +1, &set, NULL, NULL, &timeout);
				if (rv == -1) {
					 break;
				} else if (rv == 0 ) {
					 break;
				} else bytes_read = read(g.serial_params.fd, &temp_char, 1);

				if (bytes_read > 0) {
					 if (temp_char == '\n') {
						  end_of_frame_received = 1;
						  if (g.debug) fprintf(stderr, " *EOF* " );
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


		  if ((valid_data == 0) || (voltage < 0.2) || (current < g.current_threshold )) {
				snprintf(line1, sizeof(line1), " -");
				snprintf(line2, sizeof(line2), " -");
		  }
		  else {


				snprintf(line1, sizeof(line1), " %6.3fV", voltage);
				snprintf(line2, sizeof(line2), " %6.3fA", current);
				vdata[buffer_end] = voltage;
				adata[buffer_end] = current;
				buffer_end++;
				if (buffer_end >= data_buffer_size) buffer_end = 0;

		  }

		  if (g.debug) fprintf(stderr,"%c\r", spinner[count %( sizeof(spinner)-1)]);

		  /*
			*
			* END OF DATA ACQUISITION
			*
			*/

		  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		  SDL_RenderClear(renderer);


		  {
				/*
				 * Draw text
				 *
				 */
				int texW = 0;
				int texH = 0;
				int texW2 = 0;
				int texH2 = 0;
				surface = TTF_RenderUTF8_Blended(font, line1, g.font_color_volts);
				texture = SDL_CreateTextureFromSurface(renderer, surface);
				SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
				SDL_Rect dstrect = { 0, 0, texW, texH };
				SDL_RenderCopy(renderer, texture, NULL, &dstrect);
				SDL_DestroyTexture(texture);
				SDL_FreeSurface(surface);

				surface_2 = TTF_RenderUTF8_Blended(font, line2, g.font_color_amps);
				texture_2 = SDL_CreateTextureFromSurface(renderer, surface_2);
				SDL_QueryTexture(texture_2, NULL, NULL, &texW2, &texH2);
				dstrect = { 0, texH -(texH /5), texW2, texH2 };
				SDL_RenderCopy(renderer, texture_2, NULL, &dstrect);
				SDL_DestroyTexture(texture_2);
				SDL_FreeSurface(surface_2);


				/*
				 * Voltage maximum
				 *
				 */
				{
					 char sss[20];

					 v_min = 100;
					 v_max = 0;
					 a_min = 10;
					 a_max = 0;

					 //										  fprintf(stderr,"checking limits...");
					 int32_t b = buffer_end -g.window_width;
					 if (b < 0) b += data_buffer_size;
					 for (int x = 0; x < g.window_width; x++) {
						  if ( vdata[b] < v_min ) v_min = vdata[b];

						  if ( vdata[b] > v_max ) v_max = vdata[b];
						  if ( adata[b] < a_min ) a_min = adata[b];
						  if ( adata[b] > a_max ) a_max = adata[b];
						  b++;
						  if (b >= data_buffer_size) b = 0;
						  //			 fprintf(stderr,".");
					 }
					 //fprintf(stderr,"done\n");

					 snprintf(sss, sizeof(sss), "%0.3f", v_max);
					 surface = TTF_RenderUTF8_Blended(font_half, sss, g.font_color_volts);
					 texture = SDL_CreateTextureFromSurface(renderer, surface);
					 SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
					 dstrect = { g.window_width -texW, 0, texW, texH };
					 SDL_RenderCopy(renderer, texture, NULL, &dstrect);
					 SDL_DestroyTexture(texture);
					 SDL_FreeSurface(surface);

					 snprintf(sss, sizeof(sss), "%0.3f", v_min);
					 surface = TTF_RenderUTF8_Blended(font_half, sss, g.font_color_volts);
					 texture = SDL_CreateTextureFromSurface(renderer, surface);
					 SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
					 dstrect = { g.window_width -texW, g.window_height /4, texW, texH };
					 SDL_RenderCopy(renderer, texture, NULL, &dstrect);
					 SDL_DestroyTexture(texture);
					 SDL_FreeSurface(surface);

					 snprintf(sss, sizeof(sss), "%0.3f", a_max);
					 surface = TTF_RenderUTF8_Blended(font_half, sss, g.font_color_amps);
					 texture = SDL_CreateTextureFromSurface(renderer, surface);
					 SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
					 dstrect = { g.window_width -texW, g.window_height /2, texW, texH };
					 SDL_RenderCopy(renderer, texture, NULL, &dstrect);
					 SDL_DestroyTexture(texture);
					 SDL_FreeSurface(surface);

					 snprintf(sss, sizeof(sss), "%0.3f", a_min);
					 surface = TTF_RenderUTF8_Blended(font_half, sss, g.font_color_amps);
					 texture = SDL_CreateTextureFromSurface(renderer, surface);
					 SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
					 dstrect = { g.window_width -texW, (g.window_height /4) *3, texW, texH };
					 SDL_RenderCopy(renderer, texture, NULL, &dstrect);
					 SDL_DestroyTexture(texture);
					 SDL_FreeSurface(surface);
				}


				{
					 /*
					  * Draw data graphs
					  *
					  */

					 int y;
					 double vpeak = 0;
					 double apeak = 0;
					 double pvy, pay;
					 int32_t b = buffer_end -g.window_width;
					 if (b < 0) b += data_buffer_size;
					 for (int x = 0; x < g.window_width; x++) {
						  if (vdata[b] > vpeak) vpeak = vdata[b];
						  if (adata[b] > apeak) apeak = adata[b];
						  b++;
						  if (b >= data_buffer_size) b = 0;
					 }

					 if (vpeak > 6 ) vscale = 1.0 *g.window_height / 22; // 22V peak
					 else vscale = 1.0 *g.window_height /6; // 6V peak

					 if (apeak > 1.1) ascale = 1.0 *g.window_height / 6; // 6A peak
					 else ascale = 1.0 *g.window_height /1.1; // 1.1A peak

					 int32_t bx = buffer_end -g.window_width;
					 if (bx < 0) bx += data_buffer_size;
					 //										  fprintf(stderr," %d %d -> %d; height = %d vscale = %f, ascale = %f\n", buffer_end, bx, bx +g.window_width, g.window_height, vscale, ascale);

					 for (int x = 0; x < g.window_width; x++) {
						  y = 1.0 *vdata[bx] *vscale;
						  y = g.window_height -y;
						  if (x > 0) {
								SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
								//					SDL_RenderDrawPoint(renderer, x, y);
								SDL_RenderDrawLine(renderer, x-1, pvy, x, y);
						  }
						  pvy = y;

						  y = 1.0 *adata[bx] *ascale;
						  y = g.window_height -y;
						  if (x > 0) {
								SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
								//oo					SDL_RenderDrawPoint(renderer, x, y);
								SDL_RenderDrawLine(renderer, x-1, pay, x, y);
						  }
						  pay = y;

						  bx++;
						  if (bx >= data_buffer_size) bx = 0;
					 }
					 //			fprintf(stderr,"start = %d, bx = %d,height = %d vscale = %f, ascale = %f\n", g.window_height, vscale, ascale);
				}

				SDL_RenderPresent(renderer);

				/*
					SDL_DestroyTexture(texture);
					SDL_FreeSurface(surface);
					if (1) {
					SDL_DestroyTexture(texture_2);
					SDL_FreeSurface(surface_2);
					}
				 */

				if (g.error_flag) {
					 sleep(1);
				} else {
					 usleep(g.interval);
				}

		  }


		  count++; // will loop, and that's okay
					  //

	 } // while(1)

	 close(g.serial_params.fd);
	 flock(g.serial_params.fd, LOCK_UN);

	 TTF_CloseFont(font);
	 TTF_CloseFont(font_half);
	 TTF_CloseFont(font_small);

	 SDL_DestroyRenderer(renderer);
	 SDL_DestroyWindow(window);
	 TTF_Quit();
	 SDL_Quit();

	 if (vdata) free(vdata);
	 if (adata) free(adata);

	 return 0;

}
