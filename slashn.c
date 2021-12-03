/**
 * This is a fighting game that is played using a fightstick setup
 * and is played entirely at the terminal window using ANSI escape
 * characters.
 */

#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <time.h>
#include <pthread.h>

/**
 * Used to clarify which attack is being used.
 * They start with the number of the attack and trail
 * with a 1 so addition can be used to get the animation number.
 * 1 = Light
 * 2 = Medium
 * 3 = Heavy
 */
#define LATTACK 11
#define MATTACK 21
#define HATTACK 31

/* Marks the maximum number of hit points that a player has. */
#define MAXHEALTH 93

/* Used to clarify what frame is being drawn/ referenced. */
#define STANDING 1000
#define WALK1 1001
#define WALK2 1002
#define WALK3 1003
#define WALK4 1004
#define WALK5 1005
#define WALK6 1006
#define WALK7 1007
#define WALK8 1008
#define LIGHT1 1011
#define LIGHT2 1012
#define LIGHT3 1013
#define LIGHT4 1014
#define LIGHT5 1015

#define HITSTUN 1040

/* Used to mark the size of the full character sprite. */
#define HEIGHT 22
#define WIDTH 36

/* Boolean to flag while the fight is happening and the game should be ticking. */
int fighting;

/* Clocks to track the game ticks and time length. */
clock_t startTime;
clock_t lastTickTime;
clock_t currTime;


/**
 * Contains all of the information for the fighters.
 *
 * character: Marks the character that the fighter is currently playing as.
 * currFrame: Counts the current frame on an animation that they are on.
 * xPos: Integer for the horizontal position.
 * yPos: Integer for the vertical position.
 * health: Maintains the player's health.
 * btnState: These are arrays that contain the state of each button as a boolean.
 * 		Has a 0 if the button is released and a 1 if the button is pressed.
 * 		The ordering is: 0=R 1=W 2=B.
 * stickPos: This holds the current position of the stick for the player.
 * 		It is stored in the typical format of:
 * 		7 8 9
 * 		4 5 6
 * 		1 2 3
 * inputBuffer: This string holds the inputs that the player has recently executed.
 * 		This is used in the input display on the sides as well as for checking
 * 		for combos and specials.
 */
typedef struct {

int player;
int currFrame;
int xPos;
int yPos;
int health;
int btnState[3];
int stickPos;
char* inputBuffer;

} fighter;


//char* makePrintable(File f) {

//}



/* Clears the Screen */
void clearScreen() {
  printf("\x1b[2J");
}


/* Clears the input position of the screen. */
void clearPos(int xPos, int yPos) {
  printf("\x1b[%d;%dH ", yPos, xPos);
}


/* Clears the player sized area starting at the input position. */
void clearPlayers() {
  int i;
  int j;
  for (i = 6; i < 50; i++) {
    for (j = 8; j < 192; j++) {
      printf("\x1b[%d;%dH ", i, j);
    }
  }
}
/* Resets the ANSI Escape colors and effects to the default. */
void resetANSI() {
  printf("\x1b[0");
}


/* Draws the display elements. (Health Bars, Timer, and Input Buffer) */
void drawElem() {
  // TODO Draw all input for the menu.
  printf("\x1b[H");
  printf("+--------------------------------------------------------------------------------------------+   TIMER   +---------------------------------------------------------------------------------------------+\n");
  printf("|\x1b[47m                                                                                            \x1b[0m|           |\x1b[47m                                                                                             \x1b[0m|\n");
  printf("|\x1b[47m                                                                                            \x1b[0m|    9 9    |\x1b[47m                                                                                             \x1b[0m|\n");
  printf("|\x1b[47m                                                                                            \x1b[0m|           |\x1b[47m                                                                                             \x1b[0m|\n");
  printf("+--------------------------------------------------------------------------------------------+           +---------------------------------------------------------------------------------------------+\n");
  printf("+-----+                                                                                                                                                                                          +-----+\n");

  int i;
  for (i = 0; i < 45; i++) {
    printf("|     |                                                                                                                                                                                          |     |\n");
  }
  printf("+-----+                                                                                                                                                                                          +-----+\n");
  printf(". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n");
  printf("........................................................................................................................................................................................................\n");
  printf("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
}


/* Erases the Health Bars according to the input health amounts. */
void eraseHealth(int p1health, int p2health) {
  int i;
  int j;

  for (i = 0; i < 3; i++) {
    for (j = p1health; j < MAXHEALTH; i++) {
      printf("\x1b[%d;%d\x1b[0 ", i + 2, j + 2);
    }
  }

  for (i = 0; i < 3; i++) {
    for (j = 110; j < MAXHEALTH - p2health; i++) {
      printf("\x1b[%d;%d\x1b[0 ", i + 2, j + 2);
    }
  }

}


/* Draws the ASCII Art for player 1 at the given position with the given frame. */
void drawPlayer1(int xPos, int yPos, int frame) {

  switch (frame) {
    case STANDING:
      printf("\x1b[%d;%dH                .,.                  ", yPos++, xPos);
      printf("\x1b[%d;%dH               0ddd0.                ", yPos++, xPos);
      printf("\x1b[%d;%dH             ;[do([o.    .           ", yPos++, xPos);
      printf("\x1b[%d;%dH    .;^^^^^^^]*(*[*]^*)^a];;:,.      ", yPos++, xPos);
      printf("\x1b[%d;%dH           ...:.(*]^,,.:i*oi)*(,     ", yPos++, xPos);
      printf("\x1b[%d;%dH           ^..(*,  ,.(. :,^          ", yPos++, xPos);
      printf("\x1b[%d;%dH           , ,[^]^^]]i;..*           ", yPos++, xPos);
      printf("\x1b[%d;%dH          ,,^)^*:^[]))((^.           ", yPos++, xPos);
      printf("\x1b[%d;%dH          ::aa .[)oioo:              ", yPos++, xPos);
      printf("\x1b[%d;%dH          ,^#; ^a]soos0(             ", yPos++, xPos);
      printf("\x1b[%d;%dH         ;([] .*add0dd#d(            ", yPos++, xPos);
      printf("\x1b[%d;%dH         0dd^ [s00ddddd#d[           ", yPos++, xPos);
      printf("\x1b[%d;%dH        a0..  ]#00d&d#&&d0:          ", yPos++, xPos);
      printf("\x1b[%d;%dH      ,0o     o00dd&@@@&#]:          ", yPos++, xPos);
      printf("\x1b[%d;%dH     :0*     *0d00#&@&&&&i           ", yPos++, xPos);
      printf("\x1b[%d;%dH    (0:     ]0000d&&@&@&dd.          ", yPos++, xPos);
      printf("\x1b[%d;%dH   [a.    :00000d&&@@&&&&&0          ", yPos++, xPos);
      printf("\x1b[%d;%dH ,0*      .(&&d#&&&&@&&&[(^          ", yPos++, xPos);
      printf("\x1b[%d;%dH(i:       :d&&&&   i&&&d             ", yPos++, xPos);
      printf("\x1b[%d;%dH          :&@&0^    a&@;             ", yPos++, xPos);
      printf("\x1b[%d;%dH          ]&&^      s&&              ", yPos++, xPos);
      printf("\x1b[%d;%dH          #@:       &&&&0]           ", yPos++, xPos);
      printf("\x1b[%d;%dH         ]&&(                        ", yPos++, xPos);
      break;

    case WALK1:
      printf("\x1b[%d;%dH  .                                   ", yPos++, xPos);
      printf("\x1b[%d;%dH   ,,:,           .();                ", yPos++, xPos);
      printf("\x1b[%d;%dH       :;^^.    :od&ddd;              ", yPos++, xPos);
      printf("\x1b[%d;%dH           ;^^;   ]1[sd:              ", yPos++, xPos);
      printf("\x1b[%d;%dH               ^,.;[]^:.              ", yPos++, xPos);
      printf("\x1b[%d;%dH               ( :::oa1*.             ", yPos++, xPos);
      printf("\x1b[%d;%dH               ] ,[,,*:;*(,           ", yPos++, xPos);
      printf("\x1b[%d;%dH               (.;][* ^.  .           ", yPos++, xPos);
      printf("\x1b[%d;%dH               i; ]o::.               ", yPos++, xPos);
      printf("\x1b[%d;%dH             .]dd);a:                 ", yPos++, xPos);
      printf("\x1b[%d;%dH             :dddd.0a:                ", yPos++, xPos);
      printf("\x1b[%d;%dH            .#d0di^d&@#0*             ", yPos++, xPos);
      printf("\x1b[%d;%dH           .ad1d&&&&@@&&d.            ", yPos++, xPos);
      printf("\x1b[%d;%dH          ^000&@#ddd&@&&[a1)          ", yPos++, xPos);
      printf("\x1b[%d;%dH        ,d&ad#&d0000d@@@d,.(a0*.      ", yPos++, xPos);
      printf("\x1b[%d;%dH      .]ds0&&&&#dddd&@@@@0   .(1a];   ", yPos++, xPos);
      printf("\x1b[%d;%dH     *d0i(;d&@@&&&&&&@@@@,       ,;)^ ", yPos++, xPos);
      printf("\x1b[%d;%dH   ;0do[.   [&@&#10&@@&^^             ", yPos++, xPos);
      printf("\x1b[%d;%dH  ;[d#^    ;@@&&.  i@@:               ", yPos++, xPos);
      printf("\x1b[%d;%dH   :^      [&&i,   d&0                ", yPos++, xPos);
      printf("\x1b[%d;%dH          ,&@^    [&@0                ", yPos++, xPos);
      printf("\x1b[%d;%dH          (&@&d0:  .[d0:              ", yPos++, xPos);
      printf("\x1b[%d;%dH", yPos++, xPos);
      break;

    case WALK2:
      printf("\x1b[%d;%dH                                       ", yPos++, xPos);
      printf("\x1b[%d;%dH  .::;.           .**^.                ", yPos++, xPos);
      printf("\x1b[%d;%dH      .::;;     .i#&ddd(               ", yPos++, xPos);
      printf("\x1b[%d;%dH           :^^;...(iisd;               ", yPos++, xPos);
      printf("\x1b[%d;%dH               ^^..]])^.               ", yPos++, xPos);
      printf("\x1b[%d;%dH               ]: ;:is0[.              ", yPos++, xPos);
      printf("\x1b[%d;%dH              :i) (, ]..^);:           ", yPos++, xPos);
      printf("\x1b[%d;%dH              :[* []i.;.               ", yPos++, xPos);
      printf("\x1b[%d;%dH              ,ii: ](.                 ", yPos++, xPos);
      printf("\x1b[%d;%dH              ]##s: ,                  ", yPos++, xPos);
      printf("\x1b[%d;%dH             ,d#d0d] ,.                ", yPos++, xPos);
      printf("\x1b[%d;%dH            :dd00d#[,i0(               ", yPos++, xPos);
      printf("\x1b[%d;%dH           )#d0&&&#&#&&#ds*,           ", yPos++, xPos);
      printf("\x1b[%d;%dH          *0ad@&ddddd&@&d*^iai(.       ", yPos++, xPos);
      printf("\x1b[%d;%dH        ^d0d&d#d00000d&@dd)  .;][*).   ", yPos++, xPos);
      printf("\x1b[%d;%dH      :0aa#&#&&&&###&&&&o,,            ", yPos++, xPos);
      printf("\x1b[%d;%dH    .id10id&@@@&&&&&@@@@&.             ", yPos++, xPos);
      printf("\x1b[%d;%dH   *0a0[  .o&@&&o::d@&&)^.             ", yPos++, xPos);
      printf("\x1b[%d;%dH .^0#d::@&&&o:.   .d@@&;               ", yPos++, xPos);
      printf("\x1b[%d;%dH  ,(;  ,@&[         0&&                ", yPos++, xPos);
      printf("\x1b[%d;%dH        d0          .@@]               ", yPos++, xPos);
      printf("\x1b[%d;%dH                    ^@&&d0*            ", yPos++, xPos);
      break;

    case WALK3:
      printf("\x1b[%d;%dH .:;,                                 ", yPos++, xPos);
      printf("\x1b[%d;%dH    .,::.         o0a[;               ", yPos++, xPos);
      printf("\x1b[%d;%dH        ,:;;.   :0&#dd#.              ", yPos++, xPos);
      printf("\x1b[%d;%dH            :^):. **:]^               ", yPos++, xPos);
      printf("\x1b[%d;%dH              ,  ^*[(*.               ", yPos++, xPos);
      printf("\x1b[%d;%dH             );,[* .iio*              ", yPos++, xPos);
      printf("\x1b[%d;%dH            .; ((]];];,;((,           ", yPos++, xPos);
      printf("\x1b[%d;%dH            :: [(,][. ,:  .           ", yPos++, xPos);
      printf("\x1b[%d;%dH         ;^  ( ao[0]),                ", yPos++, xPos);
      printf("\x1b[%d;%dH          ^oi* d00i00                 ", yPos++, xPos);
      printf("\x1b[%d;%dH            ]:.dd00&d).               ", yPos++, xPos);
      printf("\x1b[%d;%dH           .ddda&&&&&@&d              ", yPos++, xPos);
      printf("\x1b[%d;%dH           (d0[#&#d#&@d]              ", yPos++, xPos);
      printf("\x1b[%d;%dH          ^0]0&&ddd0d#d[,             ", yPos++, xPos);
      printf("\x1b[%d;%dH        .[ii#&d00ddd0&&#00]:          ", yPos++, xPos);
      printf("\x1b[%d;%dH       *a[d&d&#&&&&&&&&(: ;o0]:       ", yPos++, xPos);
      printf("\x1b[%d;%dH     :aoo:;^d@&&&@&&#&@0     ,(](,    ", yPos++, xPos);
      printf("\x1b[%d;%dH    ;d&*  *@&&[d@&d   ,)              ", yPos++, xPos);
      printf("\x1b[%d;%dH    )*.    &d.:@@@0                   ", yPos++, xPos);
      printf("\x1b[%d;%dH           [o  &&d                    ", yPos++, xPos);
      printf("\x1b[%d;%dH               &&1                    ", yPos++, xPos);
      printf("\x1b[%d;%dH              ,#&&&do                 ", yPos++, xPos);
      break;

    case WALK4:
      printf("\x1b[%d;%dH  .                                   ", yPos++, xPos);
      printf("\x1b[%d;%dH   ,,:,           .();                ", yPos++, xPos);
      printf("\x1b[%d;%dH       :;^^.    :od&ddd;              ", yPos++, xPos);
      printf("\x1b[%d;%dH           ;^^;   ]1[sd:              ", yPos++, xPos);
      printf("\x1b[%d;%dH               ^,.;[]^:.              ", yPos++, xPos);
      printf("\x1b[%d;%dH               ( :::oa1*.             ", yPos++, xPos);
      printf("\x1b[%d;%dH               ] ,[,,*:;*(,           ", yPos++, xPos);
      printf("\x1b[%d;%dH               (.;][* ^.  .           ", yPos++, xPos);
      printf("\x1b[%d;%dH               i; ]o::.               ", yPos++, xPos);
      printf("\x1b[%d;%dH             .]dd);a:                 ", yPos++, xPos);
      printf("\x1b[%d;%dH             :dddd.0a:                ", yPos++, xPos);
      printf("\x1b[%d;%dH            .#d0di^d&@#0*             ", yPos++, xPos);
      printf("\x1b[%d;%dH           .ad1d&&&&@@&&d.            ", yPos++, xPos);
      printf("\x1b[%d;%dH          ^000&@#ddd&@&&[a1)          ", yPos++, xPos);
      printf("\x1b[%d;%dH        ,d&ad#&d0000d@@@d,.(a0*.      ", yPos++, xPos);
      printf("\x1b[%d;%dH      .]ds0&&&&#dddd&@@@@0   .(1a];   ", yPos++, xPos);
      printf("\x1b[%d;%dH     *d0i(;d&@@&&&&&&@@@@,       ,;)^ ", yPos++, xPos);
      printf("\x1b[%d;%dH   ;0do[.   [&@&#10&@@&^^             ", yPos++, xPos);
      printf("\x1b[%d;%dH  ;[d#^    ;@@&&.  i@@:               ", yPos++, xPos);
      printf("\x1b[%d;%dH   :^      [&&i,   d&0                ", yPos++, xPos);
      printf("\x1b[%d;%dH          ,&@^    [&@0                ", yPos++, xPos);
      printf("\x1b[%d;%dH          (&@&d0:  .[d0:              ", yPos++, xPos);
      break;

    case WALK5:
      printf("\x1b[%d;%dH                                       ", yPos++, xPos);
      printf("\x1b[%d;%dH.,::.                                  ", yPos++, xPos);
      printf("\x1b[%d;%dH    :;^;,        ,idd0*                ", yPos++, xPos);
      printf("\x1b[%d;%dH        .::^;.  :*0ddd&:               ", yPos++, xPos);
      printf("\x1b[%d;%dH             :;;:.^]:^(                ", yPos++, xPos);
      printf("\x1b[%d;%dH               ^, ,i0i*.               ", yPos++, xPos);
      printf("\x1b[%d;%dH              ,[[.:.;(((*^.            ", yPos++, xPos);
      printf("\x1b[%d;%dH              ^]o^ *(..  .;.           ", yPos++, xPos);
      printf("\x1b[%d;%dH              :]*]:,;;.                ", yPos++, xPos);
      printf("\x1b[%d;%dH              id0o[,                   ", yPos++, xPos);
      printf("\x1b[%d;%dH             .0#000d)..                ", yPos++, xPos);
      printf("\x1b[%d;%dH            .0dd00d#^.](               ", yPos++, xPos);
      printf("\x1b[%d;%dH           .ad0#&d#&&&&#0(             ", yPos++, xPos);
      printf("\x1b[%d;%dH          (000&&&d#&@@@d*[0].          ", yPos++, xPos);
      printf("\x1b[%d;%dH        (#dad&d0000d&@@do* ^1a(.       ", yPos++, xPos);
      printf("\x1b[%d;%dH      :o&00d&d0000d&&&@@d0^  .^io*(,   ", yPos++, xPos);
      printf("\x1b[%d;%dH    .[dda0d&&&&&#&&&&@&&&]             ", yPos++, xPos);
      printf("\x1b[%d;%dH   ]d#0];s@&&&&&&&&&&@&&@#.            ", yPos++, xPos);
      printf("\x1b[%d;%dH .i&#i^ 0@&&0,.,;;:  1@&&#             ", yPos++, xPos);
      printf("\x1b[%d;%dH.;oa*  o&&0]         .^i&&*            ", yPos++, xPos);
      printf("\x1b[%d;%dH     .d@&.              ,#@d]**.       ", yPos++, xPos);
      printf("\x1b[%d;%dH     ,1d&do.             ]0i]*^.       ", yPos++, xPos);
      break;

    case WALK6:
      printf("\x1b[%d;%dH                                       ", yPos++, xPos);
      printf("\x1b[%d;%dH  .::;.           .**^.                ", yPos++, xPos);
      printf("\x1b[%d;%dH      .::;;     .i#&ddd(               ", yPos++, xPos);
      printf("\x1b[%d;%dH           :^^;...(iisd;               ", yPos++, xPos);
      printf("\x1b[%d;%dH               ^^..]])^.               ", yPos++, xPos);
      printf("\x1b[%d;%dH               ]: ;:is0[.              ", yPos++, xPos);
      printf("\x1b[%d;%dH              :i) (, ]..^);:           ", yPos++, xPos);
      printf("\x1b[%d;%dH              :[* []i.;.               ", yPos++, xPos);
      printf("\x1b[%d;%dH              ,ii: ](.                 ", yPos++, xPos);
      printf("\x1b[%d;%dH              ]##s: ,                  ", yPos++, xPos);
      printf("\x1b[%d;%dH             ,d#d0d] ,.                ", yPos++, xPos);
      printf("\x1b[%d;%dH            :dd00d#[,i0(               ", yPos++, xPos);
      printf("\x1b[%d;%dH           )#d0&&&#&#&&#ds*,           ", yPos++, xPos);
      printf("\x1b[%d;%dH          *0ad@&ddddd&@&d*^iai(.       ", yPos++, xPos);
      printf("\x1b[%d;%dH        ^d0d&d#d00000d&@dd)  .;][*).   ", yPos++, xPos);
      printf("\x1b[%d;%dH      :0aa#&#&&&&###&&&&o,,            ", yPos++, xPos);
      printf("\x1b[%d;%dH    .id10id&@@@&&&&&@@@@&.             ", yPos++, xPos);
      printf("\x1b[%d;%dH   *0a0[  .o&@&&o::d@&&)^.             ", yPos++, xPos);
      printf("\x1b[%d;%dH .^0#d::@&&&o:.   .d@@&;               ", yPos++, xPos);
      printf("\x1b[%d;%dH  ,(;  ,@&[         0&&                ", yPos++, xPos);
      printf("\x1b[%d;%dH        d0          .@@]               ", yPos++, xPos);
      printf("\x1b[%d;%dH                    ^@&&d0*            ", yPos++, xPos);
      break;

    case WALK7:
      printf("\x1b[%d;%dH  ,,,                                 ", yPos++, xPos);
      printf("\x1b[%d;%dH    .:::,         (ao[;               ", yPos++, xPos);
      printf("\x1b[%d;%dH         :;;:   ,a#&dd#.              ", yPos++, xPos);
      printf("\x1b[%d;%dH             :;;;.^];*]               ", yPos++, xPos);
      printf("\x1b[%d;%dH              ., .(i10;:              ", yPos++, xPos);
      printf("\x1b[%d;%dH              (^,[:.[soi(             ", yPos++, xPos);
      printf("\x1b[%d;%dH              * .[[;*],;,^^,          ", yPos++, xPos);
      printf("\x1b[%d;%dH              ) *^]ii.,               ", yPos++, xPos);
      printf("\x1b[%d;%dH              (,.a0s;,                ", yPos++, xPos);
      printf("\x1b[%d;%dH            .^id:;id;                 ", yPos++, xPos);
      printf("\x1b[%d;%dH             ^&d0 ]d&*.               ", yPos++, xPos);
      printf("\x1b[%d;%dH            ^#10d00&&&&d*             ", yPos++, xPos);
      printf("\x1b[%d;%dH           :0i#@&&#&&&&#oaai*;.       ", yPos++, xPos);
      printf("\x1b[%d;%dH          ^]d#@&d0dd0d#&0  .:([oii])^:", yPos++, xPos);
      printf("\x1b[%d;%dH        :*s&d#&#&##d##&@@o        ... ", yPos++, xPos);
      printf("\x1b[%d;%dH      .^id@#&d&&&&&&&&&&&i            ", yPos++, xPos);
      printf("\x1b[%d;%dH     ;[0d;.)^0&&@&@&000d&             ", yPos++, xPos);
      printf("\x1b[%d;%dH   .i#&i    .#&@@&&;                  ", yPos++, xPos);
      printf("\x1b[%d;%dH   ;[o:      ^@&@&#:                  ", yPos++, xPos);
      printf("\x1b[%d;%dH              do@&^                   ", yPos++, xPos);
      printf("\x1b[%d;%dH               ;@@^                   ", yPos++, xPos);
      printf("\x1b[%d;%dH               *&&&&d*                ", yPos++, xPos);
      break;

    case WALK8:
      printf("\x1b[%d;%dH  ,.                                 ", yPos++, xPos);
      printf("\x1b[%d;%dH   .,::           ^;:                ", yPos++, xPos);
      printf("\x1b[%d;%dH       ,;;;.    (d&ddd:              ", yPos++, xPos);
      printf("\x1b[%d;%dH           :^(::;[s[1d,              ", yPos++, xPos);
      printf("\x1b[%d;%dH              ::,;](^;               ", yPos++, xPos);
      printf("\x1b[%d;%dH             ( ,(.:0d[):             ", yPos++, xPos);
      printf("\x1b[%d;%dH            ^ .*(*..]*[](,           ", yPos++, xPos);
      printf("\x1b[%d;%dH           ,,.]*;][o^ .  :;          ", yPos++, xPos);
      printf("\x1b[%d;%dH           ,, oi*a0(.                ", yPos++, xPos);
      printf("\x1b[%d;%dH        .^;.: ]#0i00.                ", yPos++, xPos);
      printf("\x1b[%d;%dH         .)[1),d0a#i:                ", yPos++, xPos);
      printf("\x1b[%d;%dH            o*]d&#&@&#[.             ", yPos++, xPos);
      printf("\x1b[%d;%dH           :s]d&&#dd##&#^            ", yPos++, xPos);
      printf("\x1b[%d;%dH          .^1&&&0dddddd##0s[(,       ", yPos++, xPos);
      printf("\x1b[%d;%dH         ;[#&&dddddddd&&&0..:*ioi]*^ ", yPos++, xPos);
      printf("\x1b[%d;%dH       ,*d&#]d&&&&&&&&&&&]        .  ", yPos++, xPos);
      printf("\x1b[%d;%dH     ,(a0;]])@@&&&&&&#&#0            ", yPos++, xPos);
      printf("\x1b[%d;%dH    *0#(   :d@&d^ ^@@@&.             ", yPos++, xPos);
      printf("\x1b[%d;%dH   ;i1.   ;@@&#    &&0               ", yPos++, xPos);
      printf("\x1b[%d;%dH    .     (@&1:   ^&&:               ", yPos++, xPos);
      printf("\x1b[%d;%dH         .#&i    :d@&;               ", yPos++, xPos);
      printf("\x1b[%d;%dH         )##d#0(   :sd(              ", yPos++, xPos);
      break;

    case LIGHT1:
      printf("\x1b[%d;%dH ;[ii[*;,                                               ", yPos++, xPos);
      printf("\x1b[%d;%dH     ,;*[ooi*;.                                         ", yPos++, xPos);
      printf("\x1b[%d;%dH           ,)[oao[a^,                                   ", yPos++, xPos);
      printf("\x1b[%d;%dH                ,i*.i]^:      :,                        ", yPos++, xPos);
      printf("\x1b[%d;%dH                   :::;^,  ,;[&dd1,                     ", yPos++, xPos);
      printf("\x1b[%d;%dH                    :.;     :0d#d&^                     ", yPos++, xPos);
      printf("\x1b[%d;%dH                    ,])^;:.;:*i)o]                      ", yPos++, xPos);
      printf("\x1b[%d;%dH                     .^**[[^;[[i*;...,.                 ", yPos++, xPos);
      printf("\x1b[%d;%dH                       .[;[;  ;:(*[1[o]*(^^^^^^^^(^:    ", yPos++, xPos);
      printf("\x1b[%d;%dH                        (**[;(**.  :^(,                 ", yPos++, xPos);
      printf("\x1b[%d;%dH                        ;*(]*](                         ", yPos++, xPos);
      printf("\x1b[%d;%dH                     .:(1o1aa0s^;(^:,.                  ", yPos++, xPos);
      printf("\x1b[%d;%dH                   )odd0odd000d&dddda)                  ", yPos++, xPos);
      printf("\x1b[%d;%dH               .:^ddao0000ddddd#&#dd0];.                ", yPos++, xPos);
      printf("\x1b[%d;%dH          :^]id&&0oadd000dddd&&&0ddd##dds,              ", yPos++, xPos);
      printf("\x1b[%d;%dH     :*i00a[(:10sd&dd000#&&&&@@&00d&&&&@&#:             ", yPos++, xPos);
      printf("\x1b[%d;%dH   .(]]^.  .:*sd@&&@&&&&&&&@@@@@d00&@@@&#]              ", yPos++, xPos);
      printf("\x1b[%d;%dH         ,^ia[,;&@@@&@@@@@@@@@@@&so&@@@@&s              ", yPos++, xPos);
      printf("\x1b[%d;%dH       :]00*. ^0@&@@&dd#&&o(]i[*.  .*s:]&&;             ", yPos++, xPos);
      printf("\x1b[%d;%dH       :*, .(d&o]i[.    ..              *@&[;           ", yPos++, xPos);
      printf("\x1b[%d;%dH          ^&&#^                         .1d01[          ", yPos++, xPos);
      printf("\x1b[%d;%dH          .([:                                          ", yPos++, xPos);
      break;

    case LIGHT2:
      printf("\x1b[%d;%dH                                                 ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                 ", yPos++, xPos);
      printf("\x1b[%d;%dH                   ))::...                       ", yPos++, xPos);
      printf("\x1b[%d;%dH     ,:^*][ioaass1o[;,*((:                       ", yPos++, xPos);
      printf("\x1b[%d;%dH  ;]]*^:,            .:                          ", yPos++, xPos);
      printf("\x1b[%d;%dH                     ,.;    a0i*                 ", yPos++, xPos);
      printf("\x1b[%d;%dH                      **:^][##dd0                ", yPos++, xPos);
      printf("\x1b[%d;%dH                       :***)o[(a;                ", yPos++, xPos);
      printf("\x1b[%d;%dH                       :];[ :*:.                 ", yPos++, xPos);
      printf("\x1b[%d;%dH                       ;*(]],(                   ", yPos++, xPos);
      printf("\x1b[%d;%dH                   .;((]^([;[[                   ", yPos++, xPos);
      printf("\x1b[%d;%dH             ,.;&0d#dd0[][[i*^;;;                ", yPos++, xPos);
      printf("\x1b[%d;%dH          .a#0ddddd00000oo0#0ddd000ai^;:;;,.     ", yPos++, xPos);
      printf("\x1b[%d;%dH          *d#0o1a0d000dddd&&#ddd#&&d&#^          ", yPos++, xPos);
      printf("\x1b[%d;%dH      :)([00d&d000ddd0dddd&d00d#&@&&&#:          ", yPos++, xPos);
      printf("\x1b[%d;%dH .^*i00i[(#&&&&&#dd&&@&&&@&&00d&@&&@@0           ", yPos++, xPos);
      printf("\x1b[%d;%dH,[i[*:    ,&&&&&&&&&@@&&&&&&&&o(#@@d]^           ", yPos++, xPos);
      printf("\x1b[%d;%dH       .([0&&&#&&@@do0#&&&#0*.   #&[             ", yPos++, xPos);
      printf("\x1b[%d;%dH      ^&&0a0d0do1o)              0&d:            ", yPos++, xPos);
      printf("\x1b[%d;%dH       o&d:                      [#d0a[          ", yPos++, xPos);
      printf("\x1b[%d;%dH         ,.                                      ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                 ", yPos++, xPos);
      break;

    case LIGHT3:
      printf("\x1b[%d;%dH                 ....,.:..                                 ", yPos++, xPos);
      printf("\x1b[%d;%dH                         .,]ii]):.                         ", yPos++, xPos);
      printf("\x1b[%d;%dH                             .:*o00o];.                    ", yPos++, xPos);
      printf("\x1b[%d;%dH                                  .;[0d0[^,                ", yPos++, xPos);
      printf("\x1b[%d;%dH                                     .^oddd0],             ", yPos++, xPos);
      printf("\x1b[%d;%dH                                        ;sdddda(.          ", yPos++, xPos);
      printf("\x1b[%d;%dH                                          ;0dddddi;        ", yPos++, xPos);
      printf("\x1b[%d;%dH                                           .0dddddda(      ", yPos++, xPos);
      printf("\x1b[%d;%dH                                           ,(ddddddddo,    ", yPos++, xPos);
      printf("\x1b[%d;%dH              ,^^:^*)(^;,    ::.            .sdddddddd0*   ", yPos++, xPos);
      printf("\x1b[%d;%dH             *o]](;::,.,:..]]#dda.           .iddddddddd[  ", yPos++, xPos);
      printf("\x1b[%d;%dH                     .;[^:  :i0d&,             0ddddddddd* ", yPos++, xPos);
      printf("\x1b[%d;%dH                      **;.   ;]::              :dddddddddd,", yPos++, xPos);
      printf("\x1b[%d;%dH                     )]*(**^; ..                addddddddd]", yPos++, xPos);
      printf("\x1b[%d;%dH       :i^.  .[i]^:(ioo][]*s0]..                ]ddddddddd[", yPos++, xPos);
      printf("\x1b[%d;%dH       ,]o000sd&d0sa00o0#d##&d( .               oddddddddd[", yPos++, xPos);
      printf("\x1b[%d;%dH       0##ddd0ood000dd0&&&&&&@&]..             ,dddddddddd;", yPos++, xPos);
      printf("\x1b[%d;%dH      ;d#0aasd#dddd0d#d&@@&@&&&&*           .(*0ddddddddd* ", yPos++, xPos);
      printf("\x1b[%d;%dH :(i000s00&&&d000&&&&&&&&&@@&@0: ,,. ,   .;[0ddddddddddo;  ", yPos++, xPos);
      printf("\x1b[%d;%dH*0000d&&&@&&&&&#dd&@&&&&&&#@&o,][i1,:0iii00oi[]]((;:,      ", yPos++, xPos);
      printf("\x1b[%d;%dH :0&&000&d0d&&&&&&&@&&&&0;:&@(.....::*.                    ", yPos++, xPos);
      printf("\x1b[%d;%dH.^;&&o.     .]0dddd**(:.  ^dd00o.                          ", yPos++, xPos);
      break;

    case LIGHT4:
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                 .,   .:^^^(;.                         :    ", yPos++, xPos);
      printf("\x1b[%d;%dH               ;*]*(;^;;;;:;*(da[,                     ;,   ", yPos++, xPos);
      printf("\x1b[%d;%dH              .;^:^.,,,]^^. :id##d                      .   ", yPos++, xPos);
      printf("\x1b[%d;%dH                     .]^:.   ,[^o;                          ", yPos++, xPos);
      printf("\x1b[%d;%dH                   :^[*(*]*(,.:.                            ", yPos++, xPos);
      printf("\x1b[%d;%dH              [ai[*1aoo10100].,                          )  ", yPos++, xPos);
      printf("\x1b[%d;%dH       ;[o0dd0o10ad#d00#&&&&&[ ;^                        1, ", yPos++, xPos);
      printf("\x1b[%d;%dH     .:o#d01o0dddd0d#d#&#&@@&&o,[(                      ,0. ", yPos++, xPos);
      printf("\x1b[%d;%dH.;*i1011od&&dd00d&&##&&d0#@@&@&]. .                     [a  ", yPos++, xPos);
      printf("\x1b[%d;%dH*0aa0#&&&&&#&&&d0d&@&&&d0d@&#(. ,,.                   .id*  ", yPos++, xPos);
      printf("\x1b[%d;%dH:[&&#00#&dd#&&&&&&&@&&&dso&&0;][i[,;a[*^^::,.     :*([0d0.  ", yPos++, xPos);
      printf("\x1b[%d;%dH.:]&#[......,]0d&&##01]:..dd001^..;)*;;(*][i1iiss0ddddd0;.. ", yPos++, xPos);
      break;

    case LIGHT5:
      printf("\x1b[%d;%dH                                               ", yPos++, xPos);
      printf("\x1b[%d;%dH                                               ", yPos++, xPos);
      printf("\x1b[%d;%dH                                               ", yPos++, xPos);
      printf("\x1b[%d;%dH                                               ", yPos++, xPos);
      printf("\x1b[%d;%dH                                               ", yPos++, xPos);
      printf("\x1b[%d;%dH                                               ", yPos++, xPos);
      printf("\x1b[%d;%dH                         ;;0d0o;               ", yPos++, xPos);
      printf("\x1b[%d;%dH                         ,[#dd#0               ", yPos++, xPos);
      printf("\x1b[%d;%dH                      :..^^o]0d:               ", yPos++, xPos);
      printf("\x1b[%d;%dH                   .)^* .[[[*:                 ", yPos++, xPos);
      printf("\x1b[%d;%dH                  .i*] :(;*^                   ", yPos++, xPos);
      printf("\x1b[%d;%dH                ,:*[] :io0a(.                  ", yPos++, xPos);
      printf("\x1b[%d;%dH              .^10aa] ,o0o[]]^^^,.             ", yPos++, xPos);
      printf("\x1b[%d;%dH            ,o0sd#0d#d].*0ddd1(                ", yPos++, xPos);
      printf("\x1b[%d;%dH          .)o#d0d#d00d#o1d&&&&d(,              ", yPos++, xPos);
      printf("\x1b[%d;%dH        .)i#d000dddd#&&&d#d&&&&&&doo[*):.      ", yPos++, xPos);
      printf("\x1b[%d;%dH      :[0&&000d&#ddd&&&#0d&&&&#dd&]  ,:^*]*;   ", yPos++, xPos);
      printf("\x1b[%d;%dH   .)1&&&#&&ddd0#&&@&&&&00#&&@@&&&;            ", yPos++, xPos);
      printf("\x1b[%d;%dH .idd]^d&&&&&&&&&&@&&&&&#d&&&@@@@[             ", yPos++, xPos);
      printf("\x1b[%d;%dH.[]:[#&&d&0i0dd##^,^(((^:.  ,;0&&              ", yPos++, xPos);
      printf("\x1b[%d;%dH   ;#@d.                      ^@&0i(.          ", yPos++, xPos);
      printf("\x1b[%d;%dH     is1:                      :;()(:          ", yPos++, xPos);
      break;

    case HITSTUN:
      printf("\x1b[%d;%dH", yPos++, xPos);
      break;

    default:
      break;


  }

}


/* Draws the ASCII Art for player 2 at the given position with the given frame. */
void drawPlayer2(int xPos, int yPos, int frame) {

  switch (frame) {

    case STANDING:
      printf("\x1b[%d;%dH                  .,.                ", yPos++, xPos);
      printf("\x1b[%d;%dH                .0ddd0               ", yPos++, xPos);
      printf("\x1b[%d;%dH           .    .o[(od[;             ", yPos++, xPos);
      printf("\x1b[%d;%dH      .,:;;]a^)*^]*[*(*]^^^^^^^;.    ", yPos++, xPos);
      printf("\x1b[%d;%dH     ,(*)io*i:.,,^]*(.:...           ", yPos++, xPos);
      printf("\x1b[%d;%dH          ^,: .(.,  ,*(..^           ", yPos++, xPos);
      printf("\x1b[%d;%dH           *..;i]]^^]^[, ,           ", yPos++, xPos);
      printf("\x1b[%d;%dH           .^(())][^:*^)^,,          ", yPos++, xPos);
      printf("\x1b[%d;%dH              :ooio)[. aa::          ", yPos++, xPos);
      printf("\x1b[%d;%dH             (0soos]a^ ;#^,          ", yPos++, xPos);
      printf("\x1b[%d;%dH            (d#dd0dda*. ][(;         ", yPos++, xPos);
      printf("\x1b[%d;%dH           [d#ddddd00s[ ^dd0         ", yPos++, xPos);
      printf("\x1b[%d;%dH          :0d&&#d&d00#]  ..0a        ", yPos++, xPos);
      printf("\x1b[%d;%dH          :]#&@@@&dd00o     o0,      ", yPos++, xPos);
      printf("\x1b[%d;%dH           i&&&&@&#00d0*     *0:     ", yPos++, xPos);
      printf("\x1b[%d;%dH          .dd&@&@&&d0000]     :0(    ", yPos++, xPos);
      printf("\x1b[%d;%dH          0&&&&&@@&&d00000:    .a[   ", yPos++, xPos);
      printf("\x1b[%d;%dH          ^([&&&@&&&&#d&&(.      *0, ", yPos++, xPos);
      printf("\x1b[%d;%dH             d&&&i   &&&&d:       :i(", yPos++, xPos);
      printf("\x1b[%d;%dH             ;@&a    ^0&@&:          ", yPos++, xPos);
      printf("\x1b[%d;%dH              &&s      ^&&]          ", yPos++, xPos);
      printf("\x1b[%d;%dH           ]0&&&&       :@#          ", yPos++, xPos);
      printf("\x1b[%d;%dH                        (&&]         ", yPos++, xPos);
      break;

/*
333333333333333333333333333333333333333


                                 ,;:. .
               ;[a0o         .::,.    .
              .#dd#&0:   .;;:,        .
               ^]:** .:)^:            .
               .*([*^  ,              .
              *oii. *[,;)             .
           ,((;,;];]](( ;.            .
           .  :, .[],([ ::            .
                ,)]0[oa (  ^;         .
                 00i00d *io^          .
               .)d&00dd.:]            .
              d&@&&&&&addd.           .
              ]d@&#d#&#[0d(           .
             ,[d#d0ddd&&0]0^          .
          :]00#&&0ddd00d&#ii[.        .
       :]0o; :(&&&&&&&&#&d&d[a*       .
    ,(](,     0@&#&&@&&&@d^;:ooa:     .
              ),   d&@d[&&@*  *&d;    .
                   0@@@:.d&    .*)    .
                    d&&  o[           .
                    1&&               .
                 od&&&#,              .


444444444444444444444444444444444444444

.                                   .  '
.                ;)(.           ,:,,   '
.              ;ddd&do:    .^^;:       '
.              :ds[1]   ;^^;           '
.              .:^][;.,^               '
.             .*1ao::: (               '
.           ,(*;:*,,[, ]               '
.           .  .^ *[];.(               '
.               .::o] ;i               '
.                 :a;)dd].             '
.                :a0.dddd:             '
.             *0#@&d^id0d#.            '
.            .d&&@@&&&&d1da.           '
.          )1a[&&@&ddd#@&000^          '
.      .*0a(.,d@@@d0000d&#da&d,        '
.   ;]a1(.   0@@@@&dddd#&&&&0sd].      '
. ^);,       ,@@@@&&&&&&@@&d;(i0d*     '
.             ^^&@@&01#&@&[   .[od0;   '
.               :@@i  .&&@@;    ^#d[;  '
.                0&d   ,i&&[      ^:   '
.                0@&[    ^@&,          '
.              :0d[.  :0d&@&(          '



555555555555555555555555555555555555555

                                       '
                                  .::,.'
                *0ddi,        ,;^;:    '
               :&ddd0*:  .;^::.        '
                (^:]^.:;;:             '
               .*i0i, ,^               '
            .^*(((;.:.[[,              '
           .;.  ..(* ^o]^              '
                .;;,:]*]:              '
                   ,[o0di              '
                ..)d000#0.             '
               (].^#d00dd0.            '
             (0#&&&&#d&#0da.           '
          .]0[*d@@@&#d&&&000(          '
       .(a1^ *od@@&d0000d&dad#(        '
   ,(*oi^.  ^0d@@&&&d0000d&d00&o:      '
             ]&&&@&&&&#&&&&&d0add[.    '
            .#@&&@&&&&&&&&&&@s;]0#d]   '
             #&&@1  :;;,.,0&&@0 ^i#&i. '
            *&&i^.         ]0&&o  *ao;.'
       .**]d@#,              .&@d.     '
       .^*]i0]             .od&d1,     '



666666666666666666666666666666666666666

                                       '
                .^**.           .;::.  '
               (ddd&#i.     ;;::.      '
               ;dsii(...;^^:           '
               .^)]]..^^               '
              .[0si:; :]               '
           :;)^..] ,( )i:              '
               .;.i][ *[:              '
                 .(] :ii,              '
                  , :s##]              '
                ., ]d0d#d,             '
               (0i,[#d00dd:            '
           ,*sd#&&#&#&&&0d#)           '
       .(iai^*d&@&ddddd&@da0*          '
   .)*[];.  )dd@&d00000d#d&d0d^        '
            ,,o&&&&###&&&&#&#aa0:      '
             .&@@@@&&&&&@@@&di01di.    '
             .^)&&@d::o&&@&o.  [0a0*   '
               ;&@@d.   .:o&&&@::d#0^. '
                &&0         [&@,  ;(,  '
               ]@@.          0d        '
            *0d&&@^                    '


777777777777777777777777777777777777777

                                 ,,,  '
               ;[oa(         ,:::.    '
              .#dd&#a,   :;;:         '
               ]*;]^.;;;:             '
              :;01i(. ,.              '
             (ios[.:[,^(              '
          ,^^,;,]*;[[. *              '
               ,.ii]^* )              '
                ,;s0a.,(              '
                 ;di;:di^.            '
               .*&d] 0d&^             '
             *d&&&&00d01#^            '
       .;*iaao#&&&&#&&@#i0:           '
:^)]iio[(:.  0&#d0dd0d&@#d]^          '
 ...        o@@&##d##&#&#d&s*:        '
            i&&&&&&&&&&&d&#@di^.      '
             &d000&@&@&&0^).;d0[;     '
                  ;&&@@&#.    i&#i.   '
                  :#&@&@^      :o[;   '
                   ^&@od              '
                   ^@@;               '
................*d&&&&*...............'



88888888888888888888888888888888888888

.                                 .,  .
.                :;^           ::,.   .
.              :ddd&d(    .;;;,       .
.              ,d1[s[;::(^:           .
.               ;^(];,::              .
.             :)[d0:.(, (             .
.           ,(][*]..*(*. ^            .
.          ;:  . ^o[];*].,,           .
.                .(0a*io ,,           .
.                .00i0#] :.;^.        .
.                :i#a0d,)1[).         .
.             .[#&@&#&d]*o            .
.            ^#&##dd#&&d]s:           .
.       ,([s0##dddddd0&&&1^.          .
. ^*]ioi*:..0&&&dddddddd&&#[;         .
.  .        ]&&&&&&&&&&&d]#&d*,       .
.            0#&#&&&&&&@@)]];0a(,     .
.             .&@@@^ ^d&@d:   (#0*    .
.               0&&    #&@@;   .1i;   .
.               :&&^   :1&@(     .    .
.               ;&@d:    i&#.         .
.              (ds:   (0#d##)         .


*/


      break;

    case WALK1:
      printf("\x1b[%d;%dH", yPos++, xPos);
      break;

    case WALK2:
      printf("\x1b[%d;%dH                                  ,:.  ", yPos++, xPos);
      printf("\x1b[%d;%dH                ,(*(          ,;:,.    ", yPos++, xPos);
      printf("\x1b[%d;%dH               oddd@1*    .;^;.        ", yPos++, xPos);
      printf("\x1b[%d;%dH               (0*[i,  :;::            ", yPos++, xPos);
      printf("\x1b[%d;%dH                ;*[];  :               ", yPos++, xPos);
      printf("\x1b[%d;%dH              :^*d) :*; ;.             ", yPos++, xPos);
      printf("\x1b[%d;%dH             ^][[*,;](): :             ", yPos++, xPos);
      printf("\x1b[%d;%dH           :;,:: ,][:;]^:.^            ", yPos++, xPos);
      printf("\x1b[%d;%dH               .((]0[[0].,.,  ;:       ", yPos++, xPos);
      printf("\x1b[%d;%dH                ,1[[00dd,: ^is;        ", yPos++, xPos);
      printf("\x1b[%d;%dH                 ^0s0dddd1.^(          ", yPos++, xPos);
      printf("\x1b[%d;%dH              :0&&#&0]0&&d0:           ", yPos++, xPos);
      printf("\x1b[%d;%dH              :0&&d#&#d[0&#,           ", yPos++, xPos);
      printf("\x1b[%d;%dH               ^&d#ddddd]i0^           ", yPos++, xPos);
      printf("\x1b[%d;%dH             .[&#ddddd00d1]0d*         ", yPos++, xPos);
      printf("\x1b[%d;%dH          :*1o]&&&&&&&&&#&#i]d(        ", yPos++, xPos);
      printf("\x1b[%d;%dH       ;*[];. a&&&&&&&@@@&&&&1).       ", yPos++, xPos);
      printf("\x1b[%d;%dH             .o[&&&&,)(]]d&@&#0d^      ", yPos++, xPos);
      printf("\x1b[%d;%dH                a@@&:     .:[d&#0(     ", yPos++, xPos);
      printf("\x1b[%d;%dH                .&&[          (d&*     ", yPos++, xPos);
      printf("\x1b[%d;%dH                :&&*           .&)     ", yPos++, xPos);
      printf("\x1b[%d;%dH             )d#&@&i                   ", yPos++, xPos);
      break;

    case WALK3:
      printf("\x1b[%d;%dH", yPos++, xPos);
      break;

    case WALK4:
      printf("\x1b[%d;%dH", yPos++, xPos);
      break;

    case WALK5:
      printf("\x1b[%d;%dH", yPos++, xPos);
      break;

    case WALK6:
      printf("\x1b[%d;%dH", yPos++, xPos);
      break;

    case WALK7:
      printf("\x1b[%d;%dH", yPos++, xPos);
      break;

    case WALK8:
      printf("\x1b[%d;%dH", yPos++, xPos);
      break;

    case LIGHT1:
      printf("\x1b[%d;%dH                                               ,;*[ii[; ", yPos++, xPos);
      printf("\x1b[%d;%dH                                         .;*ioo[*;,     ", yPos++, xPos);
      printf("\x1b[%d;%dH                                   ,^a[oao[),           ", yPos++, xPos);
      printf("\x1b[%d;%dH                        ,:      :^]i.*i,                ", yPos++, xPos);
      printf("\x1b[%d;%dH                     ,1dd&[;,  ,^;:::                   ", yPos++, xPos);
      printf("\x1b[%d;%dH                     ^&d#d0:     ;.:                    ", yPos++, xPos);
      printf("\x1b[%d;%dH                      ]o)i*:;.:;^)],                    ", yPos++, xPos);
      printf("\x1b[%d;%dH                 .,...;*i[[;^[[**^.                     ", yPos++, xPos);
      printf("\x1b[%d;%dH    :^(^^^^^^^^(*]o[1[*(:;  ;[;[.                       ", yPos++, xPos);
      printf("\x1b[%d;%dH                 ,(^:  .**(;[**(                        ", yPos++, xPos);
      printf("\x1b[%d;%dH                         (]*](*;                        ", yPos++, xPos);
      printf("\x1b[%d;%dH                  .,:^(;^s0aa1o1(:.                     ", yPos++, xPos);
      printf("\x1b[%d;%dH                  )adddd&d000ddo0ddo)                   ", yPos++, xPos);
      printf("\x1b[%d;%dH                .;]0dd#&#ddddd0000oadd^:.               ", yPos++, xPos);
      printf("\x1b[%d;%dH              ,sdd##ddd0&&&dddd000ddao0&&di]^:          ", yPos++, xPos);
      printf("\x1b[%d;%dH             :#&@&&&&d00&@@&&&&#000dd&ds01:([a00i*:     ", yPos++, xPos);
      printf("\x1b[%d;%dH              ]#&@@@&00d@@@@@&&&&&&&@&&@ds*:.  .^]](.   ", yPos++, xPos);
      printf("\x1b[%d;%dH              s&@@@@&os&@@@@@@@@@@@&@@@&;,[ai^,         ", yPos++, xPos);
      printf("\x1b[%d;%dH             ;&&]:s*.  .*[i](o&&#dd&@@&@0^ .*00]:       ", yPos++, xPos);
      printf("\x1b[%d;%dH           ;[&@*              ..    .[i]o&d(. ,*:       ", yPos++, xPos);
      printf("\x1b[%d;%dH          [10d1.                         ^#&&^          ", yPos++, xPos);
      printf("\x1b[%d;%dH                                          :[(.          ", yPos++, xPos);
      break;

    case LIGHT2:
      printf("\x1b[%d;%dH                                                 ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                 ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                 ", yPos++, xPos);
      printf("\x1b[%d;%dH                       ...::))                   ", yPos++, xPos);
      printf("\x1b[%d;%dH                       :((*,;[o1ssaaoi[]*^:,     ", yPos++, xPos);
      printf("\x1b[%d;%dH                          :.            ,:^*]];  ", yPos++, xPos);
      printf("\x1b[%d;%dH                 *i0a    ;.,                     ", yPos++, xPos);
      printf("\x1b[%d;%dH                0dd##[]^:**                      ", yPos++, xPos);
      printf("\x1b[%d;%dH                ;a([o)***:                       ", yPos++, xPos);
      printf("\x1b[%d;%dH                 .:*: [;]:                       ", yPos++, xPos);
      printf("\x1b[%d;%dH                   (,]](*;                       ", yPos++, xPos);
      printf("\x1b[%d;%dH                   [[;[(^]((;.                   ", yPos++, xPos);
      printf("\x1b[%d;%dH                ;;;^*i[[][0dd#d0&;.,             ", yPos++, xPos);
      printf("\x1b[%d;%dH     .,;;:;^ia000ddd0#0oo00000ddddd0#a.          ", yPos++, xPos);
      printf("\x1b[%d;%dH          ^#&d&&#ddd#&&dddd000d0a1o0#d*          ", yPos++, xPos);
      printf("\x1b[%d;%dH          :#&&&@&#d00d&dddd0ddd000d&d00[():      ", yPos++, xPos);
      printf("\x1b[%d;%dH           0@@&&@&d00&&@&&&@&&dd#&&&&&#([i00i*^. ", yPos++, xPos);
      printf("\x1b[%d;%dH           ^]d@@#(o&&&&&&&&@@&&&&&&&&&,    :*[i[,", yPos++, xPos);
      printf("\x1b[%d;%dH             [&#   .*0#&&&#0od@@&&#&&&0[(.       ", yPos++, xPos);
      printf("\x1b[%d;%dH            :d&0              )o1od0d0a0&&^      ", yPos++, xPos);
      printf("\x1b[%d;%dH          [a0d#[                      :d&o       ", yPos++, xPos);
      printf("\x1b[%d;%dH                                      .,         ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                 ", yPos++, xPos);
      break;

    case LIGHT3:
      xPos -= 20;

      printf("\x1b[%d;%dH                                 ..:.,....                 ", yPos++, xPos);
      printf("\x1b[%d;%dH                         .:)]ii],.                         ", yPos++, xPos);
      printf("\x1b[%d;%dH                    .;]o00o*:.                             ", yPos++, xPos);
      printf("\x1b[%d;%dH                ,^[0d0[;.                                  ", yPos++, xPos);
      printf("\x1b[%d;%dH             ,]0dddo^.                                     ", yPos++, xPos);
      printf("\x1b[%d;%dH          .(adddds;                                        ", yPos++, xPos);
      printf("\x1b[%d;%dH        ;iddddd0;                                          ", yPos++, xPos);
      printf("\x1b[%d;%dH      (adddddd0.                                           ", yPos++, xPos);
      printf("\x1b[%d;%dH    ,odddddddd(,                                           ", yPos++, xPos);
      printf("\x1b[%d;%dH   *0dddddddds.            .::    ,;^()*^:^^,              ", yPos++, xPos);
      printf("\x1b[%d;%dH  [dddddddddi.           .add#]]..:,.,::;(]]o*             ", yPos++, xPos);
      printf("\x1b[%d;%dH *ddddddddd0             ,&d0i:  :^[;.                     ", yPos++, xPos);
      printf("\x1b[%d;%dH,dddddddddd:              ::];   .;**                      ", yPos++, xPos);
      printf("\x1b[%d;%dH]ddddddddda                .. ;^**(*])                     ", yPos++, xPos);
      printf("\x1b[%d;%dH[ddddddddd]                ..]0s*][]ooi(:^]i[.  .^i:       ", yPos++, xPos);
      printf("\x1b[%d;%dH[dddddddddo               . (d&##d#0o00as0d&ds000o],       ", yPos++, xPos);
      printf("\x1b[%d;%dH;dddddddddd,             ..]&@&&&&&&0dd000doo0ddd##0       ", yPos++, xPos);
      printf("\x1b[%d;%dH *ddddddddd0*(.           *&&&&@&@@&d#d0dddd#dsaa0#d;      ", yPos++, xPos);
      printf("\x1b[%d;%dH  ;odddddddddd0[;.   , .,, :0@&@@&&&&&&&&&000d&&&00s000i(: ", yPos++, xPos);
      printf("\x1b[%d;%dH      ,:;((]][io00iii0:,1i[],o&@#&&&&&&@&dd#&&&&&@&&&d0000*", yPos++, xPos);
      printf("\x1b[%d;%dH                    .*::.....(@&:;0&&&&@&&&&&&&d0d&000&&0: ", yPos++, xPos);
      printf("\x1b[%d;%dH                          .o00dd^  .:(**dddd0].     .o&&;^.", yPos++, xPos);
      break;

    case LIGHT4:
      xPos -= 20;

      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH                                                            ", yPos++, xPos);
      printf("\x1b[%d;%dH    :                         .;(^^^:.   ,.                 ", yPos++, xPos);
      printf("\x1b[%d;%dH   ,;                     ,[ad(*;:;;;;^;(*]*;               ", yPos++, xPos);
      printf("\x1b[%d;%dH   .                      d##di: .^^],,,.^:^;.              ", yPos++, xPos);
      printf("\x1b[%d;%dH                          ;o^[,   .:^].                     ", yPos++, xPos);
      printf("\x1b[%d;%dH                            .:.,(*]*(*[^:                   ", yPos++, xPos);
      printf("\x1b[%d;%dH  )                          ,.]00101ooa1*[ia[              ", yPos++, xPos);
      printf("\x1b[%d;%dH ,1                        ^; [&&&&&#00d#da01o0dd0o[;       ", yPos++, xPos);
      printf("\x1b[%d;%dH .0,                      ([,o&&@@&#&#d#d0dddd0o10d#o:.     ", yPos++, xPos);
      printf("\x1b[%d;%dH  a[                     . .]&@&@@#0d&&##&&d00dd&&do1101i*;.", yPos++, xPos);
      printf("\x1b[%d;%dH  *di.                   .,, .(#&@d0d&&&@&d0d&&&#&&&&&#0aa0*", yPos++, xPos);
      printf("\x1b[%d;%dH  .0d0[(*:     .,::^^*[a;,[i[];0&&osd&&&@&&&&&&&#dd&#00#&&[:", yPos++, xPos);
      printf("\x1b[%d;%dH ..;0ddddd0ssii1i[]*(;;*);..^100dd..:]10##&&d0],......[#&]:.", yPos++, xPos);
      break;

    case LIGHT5:
      printf("\x1b[%d;%dH                                               ", yPos++, xPos);
      printf("\x1b[%d;%dH                                               ", yPos++, xPos);
      printf("\x1b[%d;%dH                                               ", yPos++, xPos);
      printf("\x1b[%d;%dH                                               ", yPos++, xPos);
      printf("\x1b[%d;%dH                                               ", yPos++, xPos);
      printf("\x1b[%d;%dH                                               ", yPos++, xPos);
      printf("\x1b[%d;%dH               ;o0d0;;                         ", yPos++, xPos);
      printf("\x1b[%d;%dH               0#dd#[,                         ", yPos++, xPos);
      printf("\x1b[%d;%dH               :d0]o^^..:                      ", yPos++, xPos);
      printf("\x1b[%d;%dH                 :*[[[. *^).                   ", yPos++, xPos);
      printf("\x1b[%d;%dH                   ^*;(: ]*i.                  ", yPos++, xPos);
      printf("\x1b[%d;%dH                  .(a0oi: ][*:,                ", yPos++, xPos);
      printf("\x1b[%d;%dH             .,^^^]][o0o, ]aa01^.              ", yPos++, xPos);
      printf("\x1b[%d;%dH                (1ddd0*.]d#d0#ds0o,            ", yPos++, xPos);
      printf("\x1b[%d;%dH              ,(d&&&&d1o#d00d#d0d#o).          ", yPos++, xPos);
      printf("\x1b[%d;%dH      .:)*[ood&&&&&&d#d&&&#dddd000d#i).        ", yPos++, xPos);
      printf("\x1b[%d;%dH   ;*]*^:,  ]&dd#&&&&d0#&&&ddd#&d000&&0[:      ", yPos++, xPos);
      printf("\x1b[%d;%dH            ;&&&@@&&#00&&&&@&&#0ddd&&#&&&1).   ", yPos++, xPos);
      printf("\x1b[%d;%dH             [@@@@&&&d#&&&&&@&&&&&&&&&&d^]ddi. ", yPos++, xPos);
      printf("\x1b[%d;%dH              &&0;,  .:^(((^,^##dd0i0&d&&#[:][.", yPos++, xPos);
      printf("\x1b[%d;%dH          .(i0&@^                      .d@#;   ", yPos++, xPos);
      printf("\x1b[%d;%dH          :()(;:                      :1si     ", yPos++, xPos);
      break;

    case HITSTUN:
      printf("\x1b[%d;%dH", yPos++, xPos);
      break;

    default:
      break;
  }
}


/* Draws projectiles. */
//void drawProj() {
  // TODO Draw projectiles
//}


/* Draws the "cutscene" art. */
//void drawScene() {
  // TODO Draw cutscenes
//}


/* Plays the attack animation and checks for and acts on a hit. */
void attack(fighter* attacker, fighter* defender, int attackVal) {
  // TODO PLAY ATTACK ANIMATION
  // TODO CHECK FOR HIT
if (attacker->currFrame != HITSTUN) {
    attacker->currFrame = attackVal + 1000;
    usleep(50000);
    attacker->currFrame++;
    usleep(60000);
    attacker->currFrame++;
    if (abs(attacker->xPos - defender->xPos) < 52) {
      defender->health = defender->health - 4;
      defender->currFrame = HITSTUN;
    }
    usleep(60000);
    attacker->currFrame++;
    usleep(70000);
    attacker->currFrame++;
    usleep(80000);
    attacker->currFrame = STANDING;
  }
}


/**
 * Checks for collision and moves player. Takes the current stick position for
 * the player and moves them accordingly. (Jumps again or moves on the ground.)
 * Returns 0 if they did not move and a 1 if they did. This is used to eliminate
 * flashing when redrawing.
 */
int move(int stickPos, fighter* p) {
  int ret = 1;

  switch (stickPos) {

    case 1:
      if (p->xPos > 14) {
	p->xPos -= 1;
      }
//      if (p->yPos < 50) {
//	p->yPos += 1;
//      }
      break;

    case 2:
//      if (p->yPos < 50) {
//	p->yPos += 1;
//      }
      break;

    case 3:
      if (p->xPos < 192) {
	p->xPos += 1;
      }
//      if (p->yPos < 50) {
//	p->yPos += 1;
//      }
      break;

    case 4:
      if (p->xPos > 8) {
	p->xPos -= 1;
      }
      break;

    case 5:
      ret = 0;
      break;

    case 6:
      if (p->xPos < 192) {
	p->xPos += 1;
      }
      break;

    case 7:
      if (p->xPos > 8) {
	p->xPos -= 1;
      }
//      if (p->yPos > 6) {
//	p->yPos -= 1;
//      }
      break;

    case 8:
//      if (p->yPos > 6) {
//	p->yPos -= 1;
//      }
      break;

    case 9:
      if (p->xPos < 192) {
	p->xPos += 1;
      }
//      if (p->yPos > 6) {
//	p->yPos -= 1;
//      }
      break;

    default:
      ret = 1;
      break;
  }

  return ret;
}


/**
 * Sets the stick position in the player that is referenced during the event
 * and then returns the current 8-way position of the stick.
 * The combination of the x and y values of the stick make the
 * position for the 8-way joystick. In the typical format of:
 * 7 8 9
 * 4 5 6
 * 1 2 3
 */
int getStickPos(struct js_event *event, fighter* p) {
  int num;

  /**
   * This will change the input value to what we want it to be so
   * we can easily deal with it and change the 8-way input.
   * This will only work with fightstick inputs.
   */
  switch(event->value) {

    case 32767:
      num = 1;
      break;

    case 0:
      num = 0;
      break;

    case -32767:
      num = -1;
      break;

    default:
      /* Nothing happens if a different input occurs. (This shouldn't happen.) */
      break;
  }

  /**
   * If the movement is on the x-axis it adds or subtracts 1.
   * If it is on the y-axis it does it with 3. This easily switches
   * the position based on the input.
   */
  if (event->number % 2 == 0) {
    switch (p->stickPos) {

      case 1:
      case 4:
      case 7:
	p->stickPos += 1;
	break;

      case 2:
      case 5:
      case 8:
	p->stickPos += num;
	break;

      case 3:
      case 6:
      case 9:
	p->stickPos -= 1;
	break;

      default:
	break;
    }

  } else {
    switch (p->stickPos) {

      case 1:
      case 2:
      case 3:
	p->stickPos += 3;
	break;

      case 4:
      case 5:
      case 6:
	p->stickPos += num * 3;
	break;

      case 7:
      case 8:
      case 9:
	p->stickPos -= 3;
	break;

      default:
	break;
    }
  }

  return p->stickPos;
}


/* Reads input from the controls. */
int readEvent(int fd, struct js_event* event) {
  ssize_t bytes;

  bytes = read(fd, event, sizeof(*event));

  if (bytes == sizeof(*event)) {
    return 0;
  }

  return 1;
}


fighter* p1;
fighter* p2;


/**
 * Listens to input players controller inputs and sets the button states
 * for them.
 */
void* playerListener(void* arg) {
  fighter* player = (fighter*) arg;
  fighter* opponent;
  char* device;
  int controls;
  struct js_event event;

  // TODO fix this to actually be good
  if (player->player == 1) {
    device = "/dev/input/js0";
  } else {
    device = "/dev/input/js1";
  }

  controls = open(device, O_RDONLY);

  /* Start the stick at the neutral position. */
  player->stickPos = 5;

  /* Set the initial button states to 0. */
  int i;
  for(i = 0; i < 3; i++) {
    player->btnState[i] = (int) calloc(3, sizeof(int));
  }

  if (player->player == 1) {
    opponent = p2;
  } else {
    opponent = p1;
  }

  /* Reads the input while the fight is going on. */
  while (readEvent(controls, &event) == 0 && fighting == 1) {
    switch (event.type) {
      /* Button Press */
      case JS_EVENT_BUTTON:
	player->btnState[event.number] = event.value;
	if (player->currFrame != HITSTUN && player->btnState[0] == 1) {
	  attack(player, opponent, LATTACK);
	}
	break;
      /* Joystick position change. */
      case JS_EVENT_AXIS:
	getStickPos(&event, player);
	break;
      default:
	break;
    }

    fflush(stdout);
  }

  printf("\x1b[H DONE WITH WORK");

  close(controls);

  return NULL;
}


int main (int argc, char* argv[]) {
  // TODO Draw the main menu

  // TODO Draw the character select screen

  // TODO Select Characters

  // TODO Start the fight

  // TODO FIGHT

  // TODO Show results of match

  // TODO Store in high score file

  /* Clear Screen so it starts blank. */
  clearScreen();

  /* Draw the bars and timer. */
  drawElem();

  /* Start Timers */
  startTime = clock();
  lastTickTime = clock();
  currTime = clock();

  /* Fight starts, set flag. */
  fighting = 1;

  /* The player structs. */
  p1 = (fighter*) malloc(sizeof(fighter));
  p2 = (fighter*) malloc(sizeof(fighter));

  /* Sets defaults for the players. */
  p1->health = MAXHEALTH;
  p2->health = MAXHEALTH;

  p1->xPos = 44;
  p1->yPos = 25;

  p2->xPos = 120;
  p2->yPos = 25;

// TODO FIX THIS OR JUST MAKE IT SO IT MAKES SENSE THAT BOTH PLAYERS ARE DIFFERENT
  p1->player = 1;
  p2->player = 2;


  p1->currFrame = STANDING;
  p2->currFrame = STANDING;

  /* Create the two threads that listen for each player's inputs. */
  pthread_t* p1Listener = (pthread_t *) malloc(sizeof(pthread_t));
  pthread_t* p2Listener = (pthread_t *) malloc(sizeof(pthread_t));

  int tRet1 = pthread_create(p1Listener, NULL, playerListener, (void*) p1);

  if (tRet1 != 0) {
    fprintf(stderr, "Could not create thread 1\n");
  }

  int tRet2 = pthread_create(p2Listener, NULL, playerListener, (void*) p2);
  if (tRet2 != 0) {
    fprintf(stderr, "Could not create thread 2\n");
  }


  /**
  * This will tick the game, checking for held buttons and
  * updating the timer. This will also check if the game is
  * over due to the timer reaching the end.
  */
  // TODO check players inputs and deal with them

  /**
   * Checks if there should be a frame update so that the game ticks often
   * but the frames update more slowly.
   */
  int tickCount = 0;

  while (fighting == 1 && startTime - currTime < 99000000) {
    currTime = clock();

    /* If it's time for another game tick. */
    if ((double) (currTime - lastTickTime) >= 100000) {
      lastTickTime = currTime;

      /* Clear the old player sprites and then change their positions if something is input at this frame. */
      clearPlayers();

      if (p1->currFrame < LIGHT1) {
	move(p1->stickPos, p1);
      }
      if (p2->currFrame < LIGHT1) {
	move(p2->stickPos, p2);
      }

      tickCount++;
    }

    if (tickCount > 1) {
      if (p1->stickPos == 4 || p1->stickPos == 6) {
        switch (p1->currFrame) {

	  case STANDING:
	    p1->currFrame = WALK1;
            break;

	  case WALK1:
	    p1->currFrame++;
	    break;

          case WALK2:
	    p1->currFrame++;
	    break;

          case WALK3:
	    p1->currFrame++;
	    break;

          case WALK4:
	    p1->currFrame++;
	    break;

          case WALK5:
	    p1->currFrame++;
	    break;

          case WALK6:
	    p1->currFrame++;
	    break;

          case WALK7:
	    p1->currFrame++;
	    break;

          case WALK8:
	    p1->currFrame = WALK1;
	    break;

          case LIGHT1:
	  case LIGHT2:
	  case LIGHT3:
	  case LIGHT4:
	    break;

          case LIGHT5:
	    p1->currFrame = STANDING;
            break;

	  case HITSTUN:
	    p1->currFrame = HITSTUN;
            break;

	  default:
	    p1->currFrame = STANDING;
	    break;

      }

      }

      if (p2->stickPos == 4 || p2->stickPos == 6) {
       switch (p2->currFrame) {

	 case STANDING:
	    p2->currFrame = WALK1;
            break;

	  case WALK1:
	    p2->currFrame++;
	    break;

          case WALK2:
	    p2->currFrame++;
	    break;

	  case WALK3:
	    p2->currFrame++;
	    break;

	  case WALK4:
	    p2->currFrame++;
	    break;

	  case WALK5:
	    p2->currFrame++;
	    break;

	  case WALK6:
	    p2->currFrame++;
	    break;

	  case WALK7:
	    p2->currFrame++;
	    break;

          case WALK8:
	    p2->currFrame = WALK1;
	    break;

          case LIGHT1:
	  case LIGHT2:
	  case LIGHT3:
	  case LIGHT4:
	    break;

          case LIGHT5:
	    p2->currFrame = STANDING;
            break;

	  case HITSTUN:
	    p2->currFrame = HITSTUN;
            break;

	  default:
	    p2->currFrame = STANDING;
  	    break;

        }

      }

      tickCount = 0;
    }

    /* Draws the players at their position. */
    drawPlayer1(p1->xPos, p1->yPos, p1->currFrame);
    drawPlayer2(p2->xPos, p2->yPos, p2->currFrame);

    /* Updates their health. */
    eraseHealth(p1->health, p2->health);

    /* Checks if the game should be over. */
    if (p1->health <= 0 || p2->health <= 0) {
      fighting = 0;
      clearScreen();
      pthread_cancel(*p1Listener);
      pthread_cancel(*p2Listener);
    }
  }


  /* Thread joins for the end. */
  pthread_join(*p1Listener, NULL);
  pthread_join(*p2Listener, NULL);

  clearScreen();
  printf("GAME OVER\n");
}
