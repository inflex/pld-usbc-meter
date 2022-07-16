#include <stdlib.h>
#include <string.h>
#include <linux/uinput.h>

#include "keycode-decode.h"

// --key-active KEY_LEFTCTRL:KEY_LEFTALT:KEY_F5
//

int is_match( char *a, char *b ) {
	int result;

	result = strcmp(a,b);

	if (result == 0) return 1;
	else return 0;
}

int keycode_decode( char *input ) {
	int keyvalue = -1;

	if (is_match(input,"RESERVED")) keyvalue = 0; 
	else if (is_match(input,"ESC")) keyvalue = 1; 
	else if (is_match(input,"1")) keyvalue = 2; 
	else if (is_match(input,"2")) keyvalue = 3; 
	else if (is_match(input,"3")) keyvalue = 4; 
	else if (is_match(input,"4")) keyvalue = 5; 
	else if (is_match(input,"5")) keyvalue = 6; 
	else if (is_match(input,"6")) keyvalue = 7; 
	else if (is_match(input,"7")) keyvalue = 8; 
	else if (is_match(input,"8")) keyvalue = 9; 
	else if (is_match(input,"9")) keyvalue = 10; 
	else if (is_match(input,"0")) keyvalue = 11; 
	else if (is_match(input,"MINUS")) keyvalue = 12; 
	else if (is_match(input,"EQUAL")) keyvalue = 13; 
	else if (is_match(input,"BACKSPACE")) keyvalue = 14; 
	else if (is_match(input,"TAB")) keyvalue = 15; 
	else if (is_match(input,"Q")) keyvalue = 16; 
	else if (is_match(input,"W")) keyvalue = 17; 
	else if (is_match(input,"E")) keyvalue = 18; 
	else if (is_match(input,"R")) keyvalue = 19; 
	else if (is_match(input,"T")) keyvalue = 20; 
	else if (is_match(input,"Y")) keyvalue = 21; 
	else if (is_match(input,"U")) keyvalue = 22; 
	else if (is_match(input,"I")) keyvalue = 23; 
	else if (is_match(input,"O")) keyvalue = 24; 
	else if (is_match(input,"P")) keyvalue = 25; 
	else if (is_match(input,"LEFTBRACE")) keyvalue = 26; 
	else if (is_match(input,"RIGHTBRACE")) keyvalue = 27; 
	else if (is_match(input,"ENTER")) keyvalue = 28; 
	else if (is_match(input,"LEFTCTRL")) keyvalue = 29; 
	else if (is_match(input,"CTRL")) keyvalue = KEY_LEFTCTRL; 
	else if (is_match(input,"A")) keyvalue = 30; 
	else if (is_match(input,"S")) keyvalue = 31; 
	else if (is_match(input,"D")) keyvalue = 32; 
	else if (is_match(input,"F")) keyvalue = 33; 
	else if (is_match(input,"G")) keyvalue = 34; 
	else if (is_match(input,"H")) keyvalue = 35; 
	else if (is_match(input,"J")) keyvalue = 36; 
	else if (is_match(input,"K")) keyvalue = 37; 
	else if (is_match(input,"L")) keyvalue = 38; 
	else if (is_match(input,"SEMICOLON")) keyvalue = 39; 
	else if (is_match(input,"APOSTROPHE")) keyvalue = 40; 
	else if (is_match(input,"GRAVE")) keyvalue = 41; 
	else if (is_match(input,"LEFTSHIFT")) keyvalue = 42; 
	else if (is_match(input,"SHIFT")) keyvalue = KEY_LEFTSHIFT; 
	else if (is_match(input,"BACKSLASH")) keyvalue = 43; 
	else if (is_match(input,"Z")) keyvalue = 44; 
	else if (is_match(input,"X")) keyvalue = 45; 
	else if (is_match(input,"C")) keyvalue = 46; 
	else if (is_match(input,"V")) keyvalue = 47; 
	else if (is_match(input,"B")) keyvalue = 48; 
	else if (is_match(input,"N")) keyvalue = 49; 
	else if (is_match(input,"M")) keyvalue = 50; 
	else if (is_match(input,"COMMA")) keyvalue = 51; 
	else if (is_match(input,"DOT")) keyvalue = 52; 
	else if (is_match(input,"SLASH")) keyvalue = 53; 
	else if (is_match(input,"RIGHTSHIFT")) keyvalue = 54; 
	else if (is_match(input,"KPASTERISK")) keyvalue = 55; 
	else if (is_match(input,"LEFTALT")) keyvalue = 56; 
	else if (is_match(input,"ALT")) keyvalue = KEY_LEFTALT; 
	else if (is_match(input,"SPACE")) keyvalue = 57; 
	else if (is_match(input,"CAPSLOCK")) keyvalue = 58; 
	else if (is_match(input,"F1")) keyvalue = 59; 
	else if (is_match(input,"F2")) keyvalue = 60; 
	else if (is_match(input,"F3")) keyvalue = 61; 
	else if (is_match(input,"F4")) keyvalue = 62; 
	else if (is_match(input,"F5")) keyvalue = 63; 
	else if (is_match(input,"F6")) keyvalue = 64; 
	else if (is_match(input,"F7")) keyvalue = 65; 
	else if (is_match(input,"F8")) keyvalue = 66; 
	else if (is_match(input,"F9")) keyvalue = 67; 
	else if (is_match(input,"F10")) keyvalue = 68; 
	else if (is_match(input,"NUMLOCK")) keyvalue = 69; 
	else if (is_match(input,"SCROLLLOCK")) keyvalue = 70; 
	else if (is_match(input,"KP7")) keyvalue = 71; 
	else if (is_match(input,"KP8")) keyvalue = 72; 
	else if (is_match(input,"KP9")) keyvalue = 73; 
	else if (is_match(input,"KPMINUS")) keyvalue = 74; 
	else if (is_match(input,"KP4")) keyvalue = 75; 
	else if (is_match(input,"KP5")) keyvalue = 76; 
	else if (is_match(input,"KP6")) keyvalue = 77; 
	else if (is_match(input,"KPPLUS")) keyvalue = 78; 
	else if (is_match(input,"KP1")) keyvalue = 79; 
	else if (is_match(input,"KP2")) keyvalue = 80; 
	else if (is_match(input,"KP3")) keyvalue = 81; 
	else if (is_match(input,"KP0")) keyvalue = 82; 
	else if (is_match(input,"KPDOT")) keyvalue = 83; 

	else if (is_match(input,"ZENKAKUHANKAKU")) keyvalue = 85; 
	else if (is_match(input,"102ND")) keyvalue = 86; 
	else if (is_match(input,"F11")) keyvalue = 87; 
	else if (is_match(input,"F12")) keyvalue = 88; 
	else if (is_match(input,"RO")) keyvalue = 89; 
	else if (is_match(input,"KATAKANA")) keyvalue = 90; 
	else if (is_match(input,"HIRAGANA")) keyvalue = 91; 
	else if (is_match(input,"HENKAN")) keyvalue = 92; 
	else if (is_match(input,"KATAKANAHIRAGANA")) keyvalue = 93; 
	else if (is_match(input,"MUHENKAN")) keyvalue = 94; 
	else if (is_match(input,"KPJPCOMMA")) keyvalue = 95; 
	else if (is_match(input,"KPENTER")) keyvalue = 96; 
	else if (is_match(input,"RIGHTCTRL")) keyvalue = 97; 
	else if (is_match(input,"KPSLASH")) keyvalue = 98; 
	else if (is_match(input,"SYSRQ")) keyvalue = 99; 
	else if (is_match(input,"RIGHTALT")) keyvalue = 100; 
	else if (is_match(input,"LINEFEED")) keyvalue = 101; 
	else if (is_match(input,"HOME")) keyvalue = 102; 
	else if (is_match(input,"UP")) keyvalue = 103; 
	else if (is_match(input,"PAGEUP")) keyvalue = 104; 
	else if (is_match(input,"LEFT")) keyvalue = 105; 
	else if (is_match(input,"RIGHT")) keyvalue = 106; 
	else if (is_match(input,"END")) keyvalue = 107; 
	else if (is_match(input,"DOWN")) keyvalue = 108; 
	else if (is_match(input,"PAGEDOWN")) keyvalue = 109; 
	else if (is_match(input,"INSERT")) keyvalue = 110; 
	else if (is_match(input,"DELETE")) keyvalue = 111; 
	else if (is_match(input,"MACRO")) keyvalue = 112; 
	else if (is_match(input,"MUTE")) keyvalue = 113; 
	else if (is_match(input,"VOLUMEDOWN")) keyvalue = 114; 
	else if (is_match(input,"VOLUMEUP")) keyvalue = 115; 
	else if (is_match(input,"POWER")) keyvalue = 116; 
	else if (is_match(input,"KPEQUAL")) keyvalue = 117; 
	else if (is_match(input,"KPPLUSMINUS")) keyvalue = 118; 
	else if (is_match(input,"PAUSE")) keyvalue = 119; 
	else if (is_match(input,"SCALE")) keyvalue = 120; 

	else if (is_match(input,"KPCOMMA")) keyvalue = 121; 
	else if (is_match(input,"HANGEUL")) keyvalue = 122; 
	else if (is_match(input,"HANGUEL")) keyvalue = KEY_HANGEUL; 
	else if (is_match(input,"HANJA")) keyvalue = 123; 
	else if (is_match(input,"YEN")) keyvalue = 124; 
	else if (is_match(input,"LEFTMETA")) keyvalue = 125; 
	else if (is_match(input,"RIGHTMETA")) keyvalue = 126; 
	else if (is_match(input,"META")) keyvalue = KEY_LEFTMETA; 

	else if (is_match(input,"COMPOSE")) keyvalue = 127; 

	else if (is_match(input,"STOP")) keyvalue = 128; 
	else if (is_match(input,"AGAIN")) keyvalue = 129; 
	else if (is_match(input,"PROPS")) keyvalue = 130; 
	else if (is_match(input,"UNDO")) keyvalue = 131; 
	else if (is_match(input,"FRONT")) keyvalue = 132; 
	else if (is_match(input,"COPY")) keyvalue = 133; 
	else if (is_match(input,"OPEN")) keyvalue = 134; 
	else if (is_match(input,"PASTE")) keyvalue = 135; 
	else if (is_match(input,"FIND")) keyvalue = 136; 
	else if (is_match(input,"CUT")) keyvalue = 137; 
	else if (is_match(input,"HELP")) keyvalue = 138; 
	else if (is_match(input,"MENU")) keyvalue = 139; 
	else if (is_match(input,"CALC")) keyvalue = 140; 
	else if (is_match(input,"SETUP")) keyvalue = 141; 
	else if (is_match(input,"SLEEP")) keyvalue = 142; 
	else if (is_match(input,"WAKEUP")) keyvalue = 143; 
	else if (is_match(input,"FILE")) keyvalue = 144; 
	else if (is_match(input,"SENDFILE")) keyvalue = 145; 
	else if (is_match(input,"DELETEFILE")) keyvalue = 146; 
	else if (is_match(input,"XFER")) keyvalue = 147; 
	else if (is_match(input,"PROG1")) keyvalue = 148; 
	else if (is_match(input,"PROG2")) keyvalue = 149; 
	else if (is_match(input,"WWW")) keyvalue = 150; 
	else if (is_match(input,"MSDOS")) keyvalue = 151; 
	else if (is_match(input,"COFFEE")) keyvalue = 152; 
	else if (is_match(input,"SCREENLOCK")) keyvalue = KEY_COFFEE; 
	else if (is_match(input,"ROTATE_DISPLAY")) keyvalue = 153; 
	else if (is_match(input,"DIRECTION")) keyvalue = KEY_ROTATE_DISPLAY; 
	else if (is_match(input,"CYCLEWINDOWS")) keyvalue = 154; 
	else if (is_match(input,"MAIL")) keyvalue = 155; 
	else if (is_match(input,"BOOKMARKS")) keyvalue = 156; 
	else if (is_match(input,"COMPUTER")) keyvalue = 157; 
	else if (is_match(input,"BACK")) keyvalue = 158; 
	else if (is_match(input,"FORWARD")) keyvalue = 159; 
	else if (is_match(input,"CLOSECD")) keyvalue = 160; 
	else if (is_match(input,"EJECTCD")) keyvalue = 161; 
	else if (is_match(input,"EJECTCLOSECD")) keyvalue = 162; 
	else if (is_match(input,"NEXTSONG")) keyvalue = 163; 
	else if (is_match(input,"PLAYPAUSE")) keyvalue = 164; 
	else if (is_match(input,"PREVIOUSSONG")) keyvalue = 165; 
	else if (is_match(input,"STOPCD")) keyvalue = 166; 
	else if (is_match(input,"RECORD")) keyvalue = 167; 
	else if (is_match(input,"REWIND")) keyvalue = 168; 
	else if (is_match(input,"PHONE")) keyvalue = 169; 
	else if (is_match(input,"ISO")) keyvalue = 170; 
	else if (is_match(input,"CONFIG")) keyvalue = 171; 
	else if (is_match(input,"HOMEPAGE")) keyvalue = 172; 
	else if (is_match(input,"REFRESH")) keyvalue = 173; 
	else if (is_match(input,"EXIT")) keyvalue = 174; 
	else if (is_match(input,"MOVE")) keyvalue = 175; 
	else if (is_match(input,"EDIT")) keyvalue = 176; 
	else if (is_match(input,"SCROLLUP")) keyvalue = 177; 
	else if (is_match(input,"SCROLLDOWN")) keyvalue = 178; 
	else if (is_match(input,"KPLEFTPAREN")) keyvalue = 179; 
	else if (is_match(input,"KPRIGHTPAREN")) keyvalue = 180; 
	else if (is_match(input,"NEW")) keyvalue = 181; 
	else if (is_match(input,"REDO")) keyvalue = 182; 

	else if (is_match(input,"F13")) keyvalue = 183; 
	else if (is_match(input,"F14")) keyvalue = 184; 
	else if (is_match(input,"F15")) keyvalue = 185; 
	else if (is_match(input,"F16")) keyvalue = 186; 
	else if (is_match(input,"F17")) keyvalue = 187; 
	else if (is_match(input,"F18")) keyvalue = 188; 
	else if (is_match(input,"F19")) keyvalue = 189; 
	else if (is_match(input,"F20")) keyvalue = 190; 
	else if (is_match(input,"F21")) keyvalue = 191; 
	else if (is_match(input,"F22")) keyvalue = 192; 
	else if (is_match(input,"F23")) keyvalue = 193; 
	else if (is_match(input,"F24")) keyvalue = 194; 

	else if (is_match(input,"PLAYCD")) keyvalue = 200; 
	else if (is_match(input,"PAUSECD")) keyvalue = 201; 
	else if (is_match(input,"PROG3")) keyvalue = 202; 
	else if (is_match(input,"PROG4")) keyvalue = 203; 
	else if (is_match(input,"DASHBOARD")) keyvalue = 204; 
	else if (is_match(input,"SUSPEND")) keyvalue = 205; 
	else if (is_match(input,"CLOSE")) keyvalue = 206; 
	else if (is_match(input,"PLAY")) keyvalue = 207; 
	else if (is_match(input,"FASTFORWARD")) keyvalue = 208; 
	else if (is_match(input,"BASSBOOST")) keyvalue = 209; 
	else if (is_match(input,"PRINT")) keyvalue = 210; 
	else if (is_match(input,"HP")) keyvalue = 211; 
	else if (is_match(input,"CAMERA")) keyvalue = 212; 
	else if (is_match(input,"SOUND")) keyvalue = 213; 
	else if (is_match(input,"QUESTION")) keyvalue = 214; 
	else if (is_match(input,"EMAIL")) keyvalue = 215; 
	else if (is_match(input,"CHAT")) keyvalue = 216; 
	else if (is_match(input,"SEARCH")) keyvalue = 217; 
	else if (is_match(input,"CONNECT")) keyvalue = 218; 
	else if (is_match(input,"FINANCE")) keyvalue = 219; 
	else if (is_match(input,"SPORT")) keyvalue = 220; 
	else if (is_match(input,"SHOP")) keyvalue = 221; 
	else if (is_match(input,"ALTERASE")) keyvalue = 222; 
	else if (is_match(input,"CANCEL")) keyvalue = 223; 
	else if (is_match(input,"BRIGHTNESSDOWN")) keyvalue = 224; 
	else if (is_match(input,"BRIGHTNESSUP")) keyvalue = 225; 
	else if (is_match(input,"MEDIA")) keyvalue = 226; 

	else if (is_match(input,"SWITCHVIDEOMODE")) keyvalue = 227; 
	else if (is_match(input,"KBDILLUMTOGGLE")) keyvalue = 228; 
	else if (is_match(input,"KBDILLUMDOWN")) keyvalue = 229; 
	else if (is_match(input,"KBDILLUMUP")) keyvalue = 230; 

	else if (is_match(input,"SEND")) keyvalue = 231; 
	else if (is_match(input,"REPLY")) keyvalue = 232; 
	else if (is_match(input,"FORWARDMAIL")) keyvalue = 233; 
	else if (is_match(input,"SAVE")) keyvalue = 234; 
	else if (is_match(input,"DOCUMENTS")) keyvalue = 235; 

	else if (is_match(input,"BATTERY")) keyvalue = 236; 

	else if (is_match(input,"BLUETOOTH")) keyvalue = 237; 
	else if (is_match(input,"WLAN")) keyvalue = 238; 
	else if (is_match(input,"UWB")) keyvalue = 239; 

	else if (is_match(input,"UNKNOWN")) keyvalue = 240; 

	else if (is_match(input,"VIDEO_NEXT")) keyvalue = 241; 
	else if (is_match(input,"VIDEO_PREV")) keyvalue = 242; 
	else if (is_match(input,"BRIGHTNESS_CYCLE")) keyvalue = 243; 
	else if (is_match(input,"BRIGHTNESS_AUTO")) keyvalue = 244; 
	else if (is_match(input,"BRIGHTNESS_ZERO")) keyvalue = KEY_BRIGHTNESS_AUTO; 
	else if (is_match(input,"DISPLAY_OFF")) keyvalue = 245; 

	else if (is_match(input,"WWAN")) keyvalue = 246; 
	else if (is_match(input,"WIMAX")) keyvalue = KEY_WWAN; 
	else if (is_match(input,"RFKILL")) keyvalue = 247; 

	else if (is_match(input,"MICMUTE")) keyvalue = 248;

	return keyvalue;
}
