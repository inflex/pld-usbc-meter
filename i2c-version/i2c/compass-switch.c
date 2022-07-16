/*
 * Compass switch (for linux)
 *
 * Uses a compass IC to help us switch Open Broadcaster
 * between scenes so that we do not get left with the
 * dreaded failure of doing a bunch of work under the 
 * microscope that isn't being recorded (I need to work
 * on one for sound recording too!)
 *
 * We're using the HMC5833L compass IC which uses i2c
 * to communicate.  Normally this is a walk in the park
 * for when you're dealing with AVR/uC/RPi sysetms that
 * have a nice exposed API and clean ways of talking i2c
 * but in linux/PCs... it's a little bit more fun.
 *
 * A few things learned doing this project, which is
 * always a great outcome!
 *
 * February 14, 2019
 *		0.0.1 - Happy Valentine's
 *
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <linux/i2c-dev.h>
#include <linux/limits.h>
#include <linux/uinput.h>
#include <dirent.h>

#include "i2c.h"
#include "keycode-decode.h"

#define SCENE_INACTIVE 0
#define SCENE_ACTIVE 1

#define VERBOSE if (g->verbose > 0)
#define CCDBG  if (gdebug > 0)
#define QUIET if (g->quiet > 0)

#define HMC5883L_ID 0x1e
#define HMC5833L_FRAME_SLEEP 80000 // a little more than the 67ms suggested by datasheet

#define MAX_KEYS 5
#define TOKENISING_CHAR ':'   // some people might prefer '_' or '+' etc

struct glb {
	int keyboard;  // file handle for keyboard
	struct uinput_setup usetup;  // input dev struct

	int debug; // debugging *duh*
	int verbose;
	int quiet;

	int ubus; // user defined bus

	int scene_state;
	int scene_pause;

	int start_angle;
	int end_angle;
	int hysteresis;

	int kmods; // 0:ctrl, 1:alt, 2:shift
	int key; // key scan code

	int keys_active[MAX_KEYS+1];
	int keys_inactive[MAX_KEYS+1];

};

int gdebug = 0;
int gkeyboard = -1; // global uinput keyboard device handle, needed so we can clean up via atexit()
int exit_program = 0;

void int_handler(int dummy) {
	CCDBG fprintf(stderr,"\nExit requested\n");
	exit_program = 1;
}

void uinput_done( void ) {
	if (gkeyboard > 0) {
		ioctl(gkeyboard, UI_DEV_DESTROY);
		close(gkeyboard);
		gkeyboard = -1;
	}
}

int uinput_init( struct glb *g ) {
	int *kv;
	g->keyboard = open("/dev/uinput", O_WRONLY|O_NONBLOCK);

	ioctl(g->keyboard, UI_SET_EVBIT, EV_KEY);
	ioctl(g->keyboard, UI_SET_EVBIT, EV_SYN);

	/*
	 * For ever key we need to simulate, we need
	 * to add it to the list of UI_SET_KEYBITs.
	 *
	 * There will be some duplication here but there's
	 * no point in worrying about it.
	 *
	 */
	kv = g->keys_active;
	while (*kv) { ioctl(g->keyboard, UI_SET_KEYBIT, *kv); kv++; }

	kv = g->keys_inactive;
	while (*kv) { ioctl(g->keyboard, UI_SET_KEYBIT, *kv); kv++; }


	/*
	 * Setup the uinput device.  This should show up
	 * in dmesg;
	 *	 input: HMC5833L Compass Switch as /devices/virtual/input/input67
	 *
	 */
	memset(&(g->usetup), 0, sizeof(g->usetup));
	g->usetup.id.bustype = BUS_USB;
	g->usetup.id.vendor = 0x1234;
	g->usetup.id.product = 0x5678;
	strcpy(g->usetup.name, "HMC5833L Compass Switch");

	ioctl(g->keyboard, UI_DEV_SETUP, &g->usetup);
	ioctl(g->keyboard, UI_DEV_CREATE);

	return 0;
}

/*
 * Linux uinput module code
 *
 */

void uinput_emit( int fd, int type, int code, int val ) {
	struct input_event ie;
	ssize_t bc;

	ie.type = type;
	ie.code = code;
	ie.value = val;
	ie.time.tv_sec = 0;
	ie.time.tv_usec = 0;

	bc = write(fd, &ie, sizeof(ie));
	if (bc != sizeof(ie)) {
		fprintf(stderr,"Attempted to write %ld bytes to keyboard event queue, only wrote %ld\n", sizeof(ie), bc);
	}
}

int uinput_press_keys( int fd, int *keyvalues ) {

	/*
	 * For every key code in the array of keyvalues
	 * we add it to the block of events first as
	 * a 'press down' code '1' before pausing
	 * slighly and then 'releasing' as code '0'
	 *
	 */
	int *kv = keyvalues;
	while (*kv) {
		CCDBG fprintf(stderr,"%d ", *kv);
		uinput_emit(fd, EV_KEY, *kv, 1);
		kv++;
	}
	uinput_emit(fd, EV_SYN, SYN_REPORT, 0);
	CCDBG fprintf(stderr,"\n");

	usleep(150000);

	kv = keyvalues;
	while (*kv) {
		CCDBG fprintf(stderr,"%d ", *kv);
		uinput_emit(fd, EV_KEY, *kv, 0);
		kv++;
	}
	uinput_emit(fd, EV_SYN, SYN_REPORT, 0);
	CCDBG fprintf(stderr,"\n");

	return 0;

}

int is_hmc5883( int file ) {
	/*
	 * ID check the device to see if it is 
	 * the HMC5883 compass device
	 *
	 * It should have H43 as the ID starting
	 * at register 0x0A.
	 *
	 */
	uint8_t buf[3];
	int rc = 0;

	rc = i2c_read_register(file, 0x0A, buf, 3); // A, B and C contain our ID
	CCDBG fprintf(stderr,"%c %c %c\n", buf[0], buf[1], buf[2]);
	if (rc != 3) return 0;

	return (buf[0] == 'H' && buf[1] == '4' && buf[2] == '3');
}

void show_help(void) {
	fprintf(stdout,"\n"
			"compass-switch -s <start angle> -e <end angle> <--active-key <keys>> <--inactive-key <keys>> [-b <bus id>] [-v] [-d] \n"
			"\n"
			"\tYou may have to run this as root/sudo to gain access to the i2c bus\n"
			"\n"
			"\t-s <start angle> : Start angle of the active area, 0~360 degrees\n"
			"\t-e <end angle> : End angle of the active area, 0~360 degrees\n"
			"\t-y <hysteresis> : Degrees of hysteresis to add to start/end angles (default 5)\n"
			"\t-p <pause> : How many seconds to wait before permitting another scene change (default 2)\n"
			"\n"
			"\t--active-key <keys>\n"
			"\t--inactive-key <keys>\n"
			"\t\tKeys can be up to 5 combination such as CTRL:ALT:SHIFT:F6.\n"
			"\t\tNOTE: Non-modifier key must be the last one in sequence\n"
			"\n"
			"\t-b <bus> : Bus is integer ID of i2c bus in linux\n"
			"\n"
			"\t-d : enable debug output\n"
			"\t-v : enable verbose output\n"
			"\n"
			"example usage: sudo ./compass-switch -s 90 -e 180 --active-key CTRL:ALT:F6 --inactive-key CTRL:ALT:F7\n"
			"\n");

}


int tokenise_keycodes( int *keyvalues, int maxvalues, char *keycodes ) {

	char *p, *hit;
	int i = 0;

	p = keycodes;

	CCDBG fprintf(stderr,"Decoding %s\n", keycodes);

	do {

		if (!p) break;

		hit = strchr(p,TOKENISING_CHAR);
		if ( (hit) || ((!hit) && (*p != '\0')) ) {
			int temp_keyvalue = -1;
			if (hit) *hit = '\0';
			temp_keyvalue = keycode_decode( p );
			if (temp_keyvalue == -1) {
				CCDBG fprintf(stderr,"Unknown keycode '%s'\n", p );
			} else {
				keyvalues[i] = temp_keyvalue;
				CCDBG fprintf(stderr," '%s' converted to '%d'\n", p, keyvalues[i]);
				i++;
				if (hit) p = hit+1;
			}
		} // hit

	} while ((i < maxvalues) && (hit) && (p) && (*p != '\0'));

	return i;
}

int parse_parameters( struct glb *g, int argc, char **argv ) {
	int i;


	for (i = 0; i < argc; i++) {
		if (argv[i][0] == '-') {
			/* parameter */
			switch (argv[i][1]) {

				case 'h':
					show_help();
					exit(1);
					break;

				case 'b':
					/*
					 * com port can be multiple things in linux
					 * such as /dev/ttySx or /dev/ttyUSBxx
					 */
					i++;
					if (i < argc) {
						g->ubus = atoi(argv[i]);
					} else {
						fprintf(stdout,"Insufficient parameters; -b <bus number>\n");
						exit(1);
					}
					break;

				case 's':
					/* 
					 * start angle of active area/arc
					 *
					 */
					i++;
					if (i < argc) {
						g->start_angle = atoi(argv[i]);
					} else {
						fprintf(stdout,"Insufficient parameters; -s <start angle (0..360)>\n");
						exit(1);
					}
					break;

				case 'e':
					i++;
					if (i < argc) {
						g->end_angle = atoi(argv[i]);
					} else {
						fprintf(stdout,"Insufficient parameters; -e <end angle (0..360)>\n");
						exit(1);
					}
					break;

				case 'y':
					i++;
					if (i < argc) {
						g->hysteresis = atoi(argv[i]);
					} else {
						fprintf(stdout,"Insufficient parameters; -h <hysteresis (0..360)>\n");
						exit(1);
					}
					break;

				case 'p':
					i++;
					if (i < argc) {
						g->scene_pause = atoi(argv[i]);
					} else {
						fprintf(stdout,"Insufficient parameters; -p <scene change sleep/wait>\n");
						exit(1);
					}
					break;

				case '-': // --foo-bar
					if (strncmp(argv[i], "--active-key", sizeof("--active-key")) == 0) {
						i++;
						tokenise_keycodes( g->keys_active, sizeof(g->keys_active), argv[i]);
					}
					if (strncmp(argv[i], "--inactive-key", sizeof("--inactive-key")) == 0) {
						i++;
						tokenise_keycodes( g->keys_inactive, sizeof(g->keys_inactive), argv[i]);
					}
					break;

				case 'd': g->debug = 1; g->verbose = 1;  gdebug = 1; break;

				case 'q': g->quiet = 1; break;

				case 'v': g->verbose = 1; break;

				default: break;
			} // switch
		}
	}


	return 0;
}

int main(int argc, char **argv) {

	int file;
	int bid = 0;
	int found = 0;
	int current_start_angle, current_end_angle;
	struct glb gg;
	struct glb *g = &gg;

	g->debug = 0;
	g->verbose = 0;
	g->quiet = 0;
	g->ubus = -1;
	g->start_angle = -1;
	g->end_angle = -1;
	g->scene_pause = 2;
	g->hysteresis = 5;

	g->scene_state = SCENE_INACTIVE;
	memset(g->keys_active, 0, sizeof(g->keys_active));
	memset(g->keys_inactive, 0, sizeof(g->keys_inactive));

	signal(SIGINT, int_handler);
	atexit(uinput_done);

	if (argc == 1) {
		show_help();
		return 0;
	}

	parse_parameters(g, argc, argv);

	if ((g->start_angle == -1)||(g->end_angle == -1)) {
		fprintf(stderr,"Start AND End angles must be set\n");
		exit(1);
	}

	current_end_angle = g->end_angle;
	current_start_angle = g->start_angle;

	gdebug = g->debug;

	// If the user has requested a specific bus
	if (g->ubus) bid = g->ubus;

	do {
		CCDBG fprintf(stderr,"Trying i2c bus ID %d...", bid);
		file = i2c_open_bus(bid);
		if (file > 0) {
			if (i2c_set_device(file, HMC5883L_ID) >= 0) {
				if (is_hmc5883(file)) {
					CCDBG fprintf(stderr,"Success\n");
					found = 1;
					break;
				}
			}
			i2c_close_bus(file);
		}
		CCDBG fprintf(stderr,"fail\n");
		if (g->ubus >= 0) break;
		bid++;
	} while ((bid < 16) && (!found));

	if (!found) {
		VERBOSE fprintf(stderr,"Could not find the compass chip.\n");
		exit(1);
	}

	uinput_init(g);
	gkeyboard = g->keyboard;

	{
		unsigned char buf[512];
		int32_t x, y; // z isn't used in planar compass setup

		CCDBG  {
			i2c_read_register(file, 0, buf, 3);
			for (x = 0; x < 3; x++) {
				fprintf(stdout, "%02x ", buf[x]);
			}
			fprintf(stdout, "\r\n"); fflush(stdout);
		}

		/*
		 * Initialise the HMC compass IC, set it
		 * in to automatic generation mode
		 *
		 */
		i2c_write_register_byte(file, 0x00, 0x70);
		i2c_write_register_byte(file, 0x01, 0xA0);
		i2c_write_register_byte(file, 0x02, 0x00);

		CCDBG  {
			i2c_read_register(file, 0, buf, 3);
			for (x = 0; x < 3; x++) {
				fprintf(stdout, "%02x ", buf[x]);
			}

			fprintf(stdout, "\r\n"); fflush(stdout);
		}

		while (!exit_program) { 
			double heading;

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
			i2c_read_register(file, 0x03, buf, 6);
			x = i2c_twos2dec_16(buf);
			y = i2c_twos2dec_16(buf +4);

			/*
			 * Calculate the heading in degrees.
			 *
			 * This is only applicable for when the
			 * sensor is planar/flat to the earth and
			 * does not factor in any declination 
			 *
			 * For the purposes of our task here, it's
			 * not a problem, as we just adjust our 
			 * start/end 'angles' according to what we
			 * see generated when moving the microscope
			 *
			 */
			heading = atan2(x, y) *(180.0 / M_PI);
			if (heading < 0) heading += 360;
			heading = 360 -heading;

			VERBOSE {
				fprintf(stdout,"\r%03.1f ", heading);
				fflush(stdout);
			}

			/*
			 * If we transition through the start or end angle
			 * then generate the required key sequence as a 
			 * one-shot event.
			 *
			 */
			if ((heading > current_start_angle) && (heading < current_end_angle)) {
				if (g->scene_state == SCENE_INACTIVE) {

					current_start_angle = g->start_angle -g->hysteresis;
					if (current_start_angle < 0) current_start_angle = 360 -current_start_angle;
					current_end_angle = (g->end_angle +g->hysteresis) %360;

					g->scene_state = SCENE_ACTIVE;
					VERBOSE fprintf(stdout,"\nACTIVE, pause for %d second(s)\n", g->scene_pause);
					uinput_press_keys( g->keyboard, g->keys_active );
					sleep(g->scene_pause);
				}
			} else {
				if (g->scene_state == SCENE_ACTIVE) {
					g->scene_state = SCENE_INACTIVE;

					current_start_angle = (g->start_angle +g->hysteresis) %360;
					current_end_angle = g->end_angle -g->hysteresis;
					if (current_end_angle < 0) current_end_angle = 360 -current_end_angle;

					VERBOSE fprintf(stdout,"\nIN-ACTIVE, pause for %d second(s)\n", g->scene_pause);
					uinput_press_keys( g->keyboard, g->keys_inactive );
					sleep(g->scene_pause);
				}
			}

			/*
			 * No point thrashing the i2c lines when the chip
			 * itself doesn't update its data registers any faster
			 * than about 15Hz in the setup we use
			 *
			 */
			usleep(HMC5833L_FRAME_SLEEP);
		}
	}

	uinput_done();


	return 0;
}
