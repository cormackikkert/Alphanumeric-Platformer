// Include the features of the arduino (pitches.h is no longer used, songchecker replaced NOTE with the actual number)
#include <ed1.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(6, 7, 8, 2, 3, 4, 5);

// Define game constants
#define VISIONRADIUS (7)          // how far the player can see to each side (7 pixels in every direction)
#define JUMPHEIGHT (3)
#define GRAV (0.5)                // the amount of pixels the character falls each frame (the player is not moved distances that have decimals)
#define STARTX (3)                // the player's initial x-position
#define STARTY (9)                // the player's initial y-position
#define TERMVELO (3)              // terminal velocity; the fastest speed that the player can move downwards

// Declare global music variables (the ones that keep track of your progress through the song)
bool skip_note;                   // whether you can skip the note as its audible duration is complete
byte note = -1;                   // which note of the melody to play (this starts at -1 because 1 is added to it the first time playMusic() is called)
float noteLength;                 // the duration of the note, both the audible sound and its silence
float audibleLength;              // the duration of the note, while it is audible (noteLength * audibleTime)
byte durationsLeft = 0;           // the number of times the loop must play in order for noteLength to be achieved
bool disableMusic = 0;            // if this is set to 1, no music will ever play

// This 2D array contains each row of 5 pixels on the LCD screen (these are used to create custom chars later in the code)
byte screen[16][16] = {
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0},
  {B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0}
};

// Initialise main variables
byte state = 0;
/*  This variable controls what screen the game is on (The variable name 'screen' is taken)
  State 0: Title screen     (display game logo and play music)  void title()
  State 1: Music menu       (allow the user to toggle music)    void musicmenu()
  State 2: Main menu        (select between different options)  void menu()
  State 3: The actual game  (where the gameplay is done)        void game()
  State 4: Highscores       (displays the highscores)           void highscores()
*/
bool push[] = {1, 1};             // push[0] is button D14, push[1] is button D15.
//                                   if the value is 1 the button was not held on the last frame and can register in the buttonPressed function; if the value is 0 it has been held for more than 1 frame


// Initialise game globals
byte leveldata[8][4];             // there are 8 blocks, each of which has 4 values: xpos, ypos, width and height (this is the leveldata for Level 0)

byte killdata[6][5][5] = {                                                                        // there are 6 levels (0-5), with 5 blocks each containing 5 values: xpos, ypos, width, height and whether it moves
  {},                                                                                             // Level 0 (there is no killdata)
  {{10, 7, 3, 3, 0}, {39, 6, 3, 3, 0}, {62, 0, 3, 3, 0}, {62, 7, 3, 3, 0}},                       // Level 1
  {{15, 13, 6, 3, 0}, {25, 13, 14, 3, 0}, {43, 13, 6, 3, 0}, {51, 0, 8, 3, 1}},                   // Level 2
  {{34, 3, 3, 3, 0}, {26, 4, 3, 3, 0}, {34, 3, 3, 3, 0}, {69, 2, 4, 4, 1}, {74, 3, 4, 4, 1}},     // Level 3
  {{11, 8, 3, 3, 1}, {25, 4, 3, 3, 1}, {53, 3, 3, 3, 1}, {62, 10, 17, 3, 0}},                     // Level 4
  {{37, 3, 3, 3, 0}, {37, 9, 3, 7, 0}, {0, 0, 0, 0, 0}, {24, 6, 3, 3, 0}}                         // Level 5
};

byte level = 0;                   // the level is used to determine the blocks to be placed, as well as the text to be printed

short playerx = 3;                // the player's x-position (0-79)
short playery = 9;                // the player's y-position (0-15)  (these are shorts because they can sometimes be negative)

short hsp = 0;                    // the player's horizontal speed for that frame, can be from -3 to 3
float vsp = 0;                    // the player's vertical speed for that frame (because of such factors as gravity, this must be a float)
float shock = 0;                  // determine how long the player should 'squish' for while landing




// Setup for the features of the board
void setup() {
  lcd.begin(16, 2);
  EEPROM_ed1.begin();
  pinMode(9, OUTPUT);             // Piezo buzzer

  pinMode(14, INPUT);             // Button D14
  pinMode(15, INPUT);             // Button D15
  digitalWrite(14, HIGH);
  digitalWrite(15, HIGH);         // Activate pull-up resistors

  lcd.print("Loading...");        // display a loading screen while the game sends values to the EEPROM

  /*for (byte i = 0; i < 35; i++) {
    EEPROM_ed1.sendB(13200 + i, (i % 7 == 0) ? 0 : 65);
    }*/ // Edwin if the values at this point in the EEPROM are screwed, uncomment this to clear the scores back to AAAAAA: 0



  /*  Write level data to the EEPROM, in this format
    x-coord of left side
    y-coord of top side
    width of rectangle
    height of rectangle
  */

  // Level 0
  EEPROM_ed1.sendB(3, 0);     //Block1    (this is the x-coordinate of the top-left)
  EEPROM_ed1.sendB(4, 0);     //          (this is the y-coordinate of the top-left)
  EEPROM_ed1.sendB(5, 2);     //          (this is the width of the rectangle)
  EEPROM_ed1.sendB(6, 14);    //          (this is the height of the rectangle)
  EEPROM_ed1.sendB(7, 0);     //Block2
  EEPROM_ed1.sendB(8, 14);
  EEPROM_ed1.sendB(9, 80);
  EEPROM_ed1.sendB(10, 2);
  EEPROM_ed1.sendB(11, 54);   //Block3
  EEPROM_ed1.sendB(12, 7);
  EEPROM_ed1.sendB(13, 27);
  EEPROM_ed1.sendB(14, 7);
  EEPROM_ed1.sendB(15, 2);    //Block4
  EEPROM_ed1.sendB(16, 0);
  EEPROM_ed1.sendB(17, 13);
  EEPROM_ed1.sendB(18, 5);
  EEPROM_ed1.sendB(19, 2);    //Block5
  EEPROM_ed1.sendB(20, 0);
  EEPROM_ed1.sendB(21, 39);
  EEPROM_ed1.sendB(22, 6);

  for (byte i = 23; i < 35; i++) {
    EEPROM_ed1.sendB(i, 0);
  }

  // Level 1
  EEPROM_ed1.sendB(35, 0);    //Block1
  EEPROM_ed1.sendB(36, 12);
  EEPROM_ed1.sendB(37, 80);
  EEPROM_ed1.sendB(38, 4);
  EEPROM_ed1.sendB(39, 10);   //Block2
  EEPROM_ed1.sendB(40, 10);
  EEPROM_ed1.sendB(41, 3);
  EEPROM_ed1.sendB(42, 2);
  EEPROM_ed1.sendB(43, 28);   //Block3
  EEPROM_ed1.sendB(44, 0);
  EEPROM_ed1.sendB(45, 3);
  EEPROM_ed1.sendB(46, 7);
  EEPROM_ed1.sendB(47, 39);   //Block4
  EEPROM_ed1.sendB(48, 9);
  EEPROM_ed1.sendB(49, 3);
  EEPROM_ed1.sendB(50, 3);
  EEPROM_ed1.sendB(51, 52);   //Block5
  EEPROM_ed1.sendB(52, 0);
  EEPROM_ed1.sendB(53, 3);
  EEPROM_ed1.sendB(54, 5);
  EEPROM_ed1.sendB(55, 62);   //Block6
  EEPROM_ed1.sendB(56, 10);
  EEPROM_ed1.sendB(57, 3);
  EEPROM_ed1.sendB(58, 2);
  EEPROM_ed1.sendB(59, 0);    //Block7
  EEPROM_ed1.sendB(60, 0);
  EEPROM_ed1.sendB(61, 0);
  EEPROM_ed1.sendB(62, 0);
  EEPROM_ed1.sendB(63, 0);    //Block8
  EEPROM_ed1.sendB(64, 0);
  EEPROM_ed1.sendB(65, 0);
  EEPROM_ed1.sendB(66, 0);

  // Level 2
  EEPROM_ed1.sendB(67, 0);    //Block1
  EEPROM_ed1.sendB(68, 13);
  EEPROM_ed1.sendB(69, 15);
  EEPROM_ed1.sendB(70, 3);
  EEPROM_ed1.sendB(71, 21);   //Block2
  EEPROM_ed1.sendB(72, 13);
  EEPROM_ed1.sendB(73, 4);
  EEPROM_ed1.sendB(74, 3);
  EEPROM_ed1.sendB(75, 39);   //Block3
  EEPROM_ed1.sendB(76, 13);
  EEPROM_ed1.sendB(77, 4);
  EEPROM_ed1.sendB(78, 3);
  EEPROM_ed1.sendB(79, 49);   //Block4
  EEPROM_ed1.sendB(80, 13);
  EEPROM_ed1.sendB(81, 21);
  EEPROM_ed1.sendB(82, 3);
  EEPROM_ed1.sendB(83, 64);   //Block5
  EEPROM_ed1.sendB(84, 5);
  EEPROM_ed1.sendB(85, 16);
  EEPROM_ed1.sendB(86, 8);
  EEPROM_ed1.sendB(87, 0);    //Block6
  EEPROM_ed1.sendB(88, 0);
  EEPROM_ed1.sendB(89, 0);
  EEPROM_ed1.sendB(90, 0);
  EEPROM_ed1.sendB(91, 0);    //Block7
  EEPROM_ed1.sendB(92, 0);
  EEPROM_ed1.sendB(93, 0);
  EEPROM_ed1.sendB(94, 0);
  EEPROM_ed1.sendB(95, 0);    //Block8
  EEPROM_ed1.sendB(96, 0);
  EEPROM_ed1.sendB(97, 0);
  EEPROM_ed1.sendB(98, 0);

  // Level 3
  EEPROM_ed1.sendB(99, 0);    //Block1
  EEPROM_ed1.sendB(100, 13);
  EEPROM_ed1.sendB(101, 30);
  EEPROM_ed1.sendB(102, 3);
  EEPROM_ed1.sendB(103, 26);  //Block2
  EEPROM_ed1.sendB(104, 0);
  EEPROM_ed1.sendB(105, 3);
  EEPROM_ed1.sendB(106, 4);
  EEPROM_ed1.sendB(107, 34);  //Block3
  EEPROM_ed1.sendB(108, 6);
  EEPROM_ed1.sendB(109, 3);
  EEPROM_ed1.sendB(110, 3);
  EEPROM_ed1.sendB(111, 54);  //Block4
  EEPROM_ed1.sendB(112, 13);
  EEPROM_ed1.sendB(113, 26);
  EEPROM_ed1.sendB(114, 3);
  EEPROM_ed1.sendB(115, 0);  //Block5
  EEPROM_ed1.sendB(116, 0);
  EEPROM_ed1.sendB(117, 0);
  EEPROM_ed1.sendB(118, 0);
  EEPROM_ed1.sendB(119, 0);  //Block6
  EEPROM_ed1.sendB(120, 0);
  EEPROM_ed1.sendB(121, 0);
  EEPROM_ed1.sendB(122, 0);
  EEPROM_ed1.sendB(123, 0);   //Block7
  EEPROM_ed1.sendB(124, 0);
  EEPROM_ed1.sendB(125, 0);
  EEPROM_ed1.sendB(126, 0);
  EEPROM_ed1.sendB(127, 0);   //Block8
  EEPROM_ed1.sendB(128, 0);
  EEPROM_ed1.sendB(129, 0);
  EEPROM_ed1.sendB(130, 0);

  // Level 4
  EEPROM_ed1.sendB(131, 0);   //Block1
  EEPROM_ed1.sendB(132, 13);
  EEPROM_ed1.sendB(133, 11);
  EEPROM_ed1.sendB(134, 3);
  EEPROM_ed1.sendB(135, 16);  //Block2
  EEPROM_ed1.sendB(136, 13);
  EEPROM_ed1.sendB(137, 12);
  EEPROM_ed1.sendB(138, 3);
  EEPROM_ed1.sendB(139, 39);  //Block3
  EEPROM_ed1.sendB(140, 13);
  EEPROM_ed1.sendB(141, 21);
  EEPROM_ed1.sendB(142, 3);
  EEPROM_ed1.sendB(143, 50);  //Block4
  EEPROM_ed1.sendB(144, 0);
  EEPROM_ed1.sendB(145, 3);
  EEPROM_ed1.sendB(146, 8);
  EEPROM_ed1.sendB(147, 60);  //Block5
  EEPROM_ed1.sendB(148, 6);
  EEPROM_ed1.sendB(149, 20);
  EEPROM_ed1.sendB(150, 10);
  EEPROM_ed1.sendB(151, 0);   //Block6
  EEPROM_ed1.sendB(152, 0);
  EEPROM_ed1.sendB(153, 0);
  EEPROM_ed1.sendB(154, 0);
  EEPROM_ed1.sendB(155, 0);   //Block7
  EEPROM_ed1.sendB(156, 0);
  EEPROM_ed1.sendB(157, 0);
  EEPROM_ed1.sendB(158, 0);
  EEPROM_ed1.sendB(159, 0);   //Block8
  EEPROM_ed1.sendB(160, 0);
  EEPROM_ed1.sendB(161, 0);
  EEPROM_ed1.sendB(162, 0);

  // Level 5
  EEPROM_ed1.sendB(163, 0);   //Block1
  EEPROM_ed1.sendB(164, 13);
  EEPROM_ed1.sendB(165, 27);
  EEPROM_ed1.sendB(166, 3);
  EEPROM_ed1.sendB(167, 24);  //Block2
  EEPROM_ed1.sendB(168, 0);
  EEPROM_ed1.sendB(169, 3);
  EEPROM_ed1.sendB(170, 6);
  EEPROM_ed1.sendB(171, 37);  //Block3
  EEPROM_ed1.sendB(172, 6);
  EEPROM_ed1.sendB(173, 3);
  EEPROM_ed1.sendB(174, 3);
  EEPROM_ed1.sendB(175, 56);  //Block4
  EEPROM_ed1.sendB(176, 12);
  EEPROM_ed1.sendB(177, 24);
  EEPROM_ed1.sendB(178, 4);
  EEPROM_ed1.sendB(179, 0);   //Block5
  EEPROM_ed1.sendB(180, 0);
  EEPROM_ed1.sendB(181, 0);
  EEPROM_ed1.sendB(182, 0);
  EEPROM_ed1.sendB(183, 0);   //Block6
  EEPROM_ed1.sendB(184, 0);
  EEPROM_ed1.sendB(185, 0);
  EEPROM_ed1.sendB(186, 0);
  EEPROM_ed1.sendB(187, 0);   //Block7
  EEPROM_ed1.sendB(188, 0);
  EEPROM_ed1.sendB(189, 0);
  EEPROM_ed1.sendB(190, 0);
  EEPROM_ed1.sendB(191, 0);   //Block8
  EEPROM_ed1.sendB(192, 0);
  EEPROM_ed1.sendB(193, 0);
  EEPROM_ed1.sendB(194, 0);
}

void loop() {
  lcd.clear();

  // Determine which screen to run
  switch (state) {
    case 0: title();       break;
    case 1: musicmenu();   break;
    case 2: menu();        break;
    case 3: game();        break;
    case 4: highscores();  break;
  }
}

// Declare some useful functions:
bool buttonPressed(byte id) {                              // digitalRead() triggers when a button is being held. This function triggers only on the frame that the button is pressed (mainly to prevent flying through menus)
  bool r = false;                                          // a temporary variable that only exists because 'return' exits the code
  if ((digitalRead(id) == LOW) && (push[id - 14] == 1))    // if you press the button and you are allowed to (it wasn't held on the last frame)
    r = true;                                              // essentially 'return true'

  push[id - 14] = (digitalRead(id) == HIGH);               // if the button is not being held, make it so the function can trigger next frame,
  return r; // return true or false                           if the button is being held, it is either already 0, or it must be set to 0
}

short sign(short inp) {                                    // this function returns whether the number is positive, negative or zero (shorts are returned because bytes cannot be negative)
  if (inp < 0)
    return -1;
  if (inp > 0)
    return 1;
  if (inp == 0)
    return 0;                                              // this entire code can be simplifed to:           return (2 * (inp > 0) - (inp != 0));           (but it looks messy)
}

void title() {                                             // because of the while loop, this function is 'locked' and never runs loop() repeatedly (the state machine will only trigger once, until D15 is pressed and the function is complete)
  musicReset();                                            // musicReset() is used throughout the code to ensure that each song starts from the beginning, and that a monotone note does not play indefinitely

  // Print title text
  lcd.setCursor(2, 1);
  lcd.print("D15 to start");

  while (!buttonPressed(15)) {
    playMusic(0, 46.875);                                  // the first argument is which song to play, the second is how many frames to play that song for (see just above where the function is created for more info)
  } //                                                        (a value of 46.875 is used because the function will not work unless 'amount' is a multiple of 'tempo' (tempo in this case is 187.5) (my solution to fix this on the last day didn't work)

  state = 1;                                               // there is no musicReset() or delay() because the music should play continuously from this screen to the next
}

void musicmenu() {
  static byte arrowpos = 0;                                // static so that it doesn't set itself to 0 every time this function loops (this is a looping function that triggers the state machine each time)

  byte arrow[8] {                                          // the arrow character for the main menu (this is in musicmenu because you can only have 8 custom chars, and they are all used in game(), so this must be re-written every time the musicmenu is accessed)
    B11000,
    B11100,
    B11110,
    B11111,
    B11111,
    B11110,
    B11100,
    B11000
  };
  lcd.createChar(0, arrow);                                // actually create the custom char

  /*   'arrowpos'
    0: Music On
    1: Music Off
  */

  // These 4 lines draw both options onto the screen:
  lcd.setCursor(2, 0);
  lcd.print("Music On");
  lcd.setCursor(2, 1);
  lcd.print("Music Off");

  // Draw the menu arrow at the right place
  lcd.setCursor(0, arrowpos);
  lcd.write(byte(0));

  // Check for inputs
  if (buttonPressed(15)) {                                 // this is first so that when both buttons are pressed at once, you go to the current selection screen and not the next one
    disableMusic = arrowpos;
    state = 2;
    musicReset();
    delay(250);                                            // delays are used after some instances of musicReset() so that the music doesn't transition badly to other screens with music
  }

  if (buttonPressed(14))
    arrowpos = 1 - arrowpos;                               // this toggles arrowpos from 0 to 1

  playMusic(0, 46.875);                                    // the delay prevents obnoxious flashing while the lcd screen is cleared and written to repeatedly, while also playing music. The music sounds continuous because only this line has a delay
}

// The main menu (state == 2)
void menu() {                                              // this function is very similar to musicmenu()
  static byte arrowpos = 0;

  byte arrow[8] {                                          // the arrow character for the main menu (this is re-written because it is not saved in the musicmenu)
    B11000,
    B11100,
    B11110,
    B11111,
    B11111,
    B11110,
    B11100,
    B11000
  };
  lcd.createChar(0, arrow);

  /*   'arrowpos'
    0: Play  2: Quit
    1: Hi-score
  */

  // These 6 lines draw each option onto the screen:
  lcd.setCursor(2, 0);
  lcd.print("Play");
  lcd.setCursor(2, 1);
  lcd.print("Hi-score");
  lcd.setCursor(10, 0);
  lcd.print("Quit");

  // Draw the menu arrow at the right place (see table below this comment)
  lcd.setCursor(8 * (arrowpos == 2), (arrowpos == 1));     //   arrowpos  |  cursorpos   //
  lcd.write(byte(0));                                      //       0     |    (0,0)     //
  // (these are here to stop                               //       1     |    (0,1)     //
  //  auto-format messing up the table)                    //       2     |    (8,0)     //

  if (buttonPressed(15)) {
    state = (arrowpos + 3 - 5 * (arrowpos == 2));          // this can probably be replaced with "state = (arrowpos + 3) % 5;", but it won't be because I have no access to an arduino to test it (the same thing happens on line 438)
    /*  Because the states are numbers, calculations can be performed easily without the need for a switch statement:
      arrowpos 0 (Play)     -> state 3
      arrowpos 1 (Hi-score) -> state 4
      arrowpos 2 (Quit)     -> state 0
    */
  }

  if (buttonPressed(14))
    arrowpos = (arrowpos + 1) % 3;
  arrowpos += 1 + ((arrowpos == 2) * -3);                  // arrowpos will toggle (0>1>2>0>1>2 etc.) (can probably be "arrowpos = (arrowpos + 1) % 3;")

  playMusic(1, 62.5);                                      // see musicmenu() for comments (this is half of the tempo [125])
}




void game() {
  musicReset();

  // Initialise game local variables
  byte hspdisable = 0;
  bool play = 1;                                           // if play is 0, the game is over
  byte lives = 100;                                        // lives also doubles as the player's score
  bool d15pressed = 0;                                     // because of the way buttonPressed is designed, if it is to be checked multiple times in a single frame only the first will work (this variable fixes that)
  level = 0;                                               // so that when the game is played again, you start from the first level

  for (byte i = 0; i < 32; i++) {                          // this loads the data from the EEPROM to leveldata[8][4] for the first level
    leveldata[i / 4][i % 4] = EEPROM_ed1.readB(i + 3);
  }

  noTone(9);                                               // probably unnecessary because of musicReset(), but no access to an arduino board

  // The opening text sequence
  lcd.print("Welcome");
  delay(2500);
  lcd.clear();

  lcd.print("You");
  delay(1500);
  lcd.setCursor(3, 1);
  lcd.print("are not alone");
  delay(2000);

  lcd.clear();
  delay(1000);
  lcd.setCursor(1, 1);
  lcd.print("We are watching");
  delay(3000);

  lcd.clear();
  lcd.print("Some things");
  lcd.setCursor(7, 1);
  delay(3000);
  lcd.print("kill you");
  delay(3000);

  lcd.clear();
  lcd.print("And some...");
  delay(3000);
  lcd.setCursor(6, 1);
  lcd.print("     don't");
  delay(3000);

  lcd.clear();
  lcd.print("Enjoy");
  delay(3000);
  lcd.clear();

  // The actual game
  while (play == 1) {

    // Check if the player has reached the end of the level
    if (playerx >= 75) {
      noTone(9);                                           // noTone() prevents a continuous note from being played during the transition screens
      level++;                                             // proceed to the next level
      lcd.setCursor(0, 0);
      switch (level) {                                     // print different text for each transition screen (or end the game on the final level)
        case 1: lcd.print("Avoid outlines");  break;
        case 2: lcd.print("Be careful...");   break;
        case 3: lcd.print("Meteors!");        break;
        case 4: lcd.print("I sense wind");    break;
        case 5: lcd.print("The last one!");   break;
        case 6: {
            writehighscores(lives);
            play = 0;                                      // game over
            state = 4;                                     // send player to highscores screen
            break;
          }
      }
      delay(2000);                                         // the transition screen will hold for 2 seconds

      // Initialise the player's variables
      playerx = STARTX;
      playery = STARTY;
      hsp = 0;
      vsp = 0;
      shock = 0;

      // Level data retrieves the block positions and sizes from the EEPROM (this has already happened for Level 0)
      for (byte i = 0; i < 32; i++) {
        leveldata[i / 4][i % 4] = EEPROM_ed1.readB(35 + ((level - 1) * 32) + i);
      }
    }

    // Check if the player should be killed
    if ((kill_meeting(playerx, playery, 3, 3)) || (!(boxoverlap(playerx, playery, 3, 3, 0, -10, 80, 26)))) {        // you won't die off of the top

      if (lives == 1) {                                                             // if the player dies on their last life, game over
        writehighscores(lives);
        play = 0;
        state = 4;
        break;                                                                      // this breaks the while loop
      }
      lives--;

      // Reset the player's variables
      playerx = STARTX;
      playery = STARTY;
      hsp = 0;
      vsp = 0;
    }

    moveobjects();                                                                  // moves all the objects that kill you (in killdata)

    // Get inputs
    short hsp_reading = map(analogRead(3), 0, 1023, 3, -3);                         // this next code is meant to improve the walljumping, as it felt clunky and awkward previously
    if (hspdisable == 0)
      hsp = hsp_reading - (2 * (level == 4));                                       // there is wind on level 4
    else
      hsp -= sign(hsp_reading);                                                     // reduce hsp if walljumped (or a new level)

    d15pressed = (buttonPressed(15));

    if (place_meeting(playerx, playery + 1, 3, 3)) {                                // if the player is grounded
      if (digitalRead(15) == LOW)                                                   // if D15 (jump) is pressed
        vsp = -JUMPHEIGHT;
    }
    else {                                                                          // if the player is airborne
      if ((vsp < 0) && (digitalRead(15) == HIGH))                                   // if the player is moving upwards and is not holding the jump button
        vsp += 0.5;                                                                 // keep adding extra fallspeed to the player if the button is being held, this leads to a controllabe jumpheight
      if ((place_meeting(playerx + sign(hsp), playery, 3, 3)) && (d15pressed)) {    // because hsp dictates where you will move next frame, this will only activate if you are holding towards a wall while having pressed jump
        // Jump off of the wall
        vsp = -JUMPHEIGHT;                                                          // move upwards
        hsp = -(sign(hsp)) * 3;                                                     // move away from the wall
        hspdisable = 1;                                                             // Stop horizontal inputs so you dont immedietly return to the wall
      }
    }
    vsp += GRAV;                                                                    // fall down with gravity (this cannot be placed inside the airborne section because of a bug with jumping and whether you are grounded/airborne)
    vsp = min(vsp, TERMVELO);

    // Vertical Collision Detection
    if (place_meeting(playerx, playery + (short)vsp, 3, 3)) {                       // same code, except for vertical distance with small additions (vsp is casted as a 'short' to prevent floats from causing potential math problems)
      if (place_meeting(playerx, playery + sign((short)vsp), 3, 3))
        vsp = 0;
      else {
        while (!(place_meeting(playery, playery + sign((short)vsp), 3, 3))) {
          playery += sign((short)vsp);
        }
        if (vsp == TERMVELO)
          shock = vsp * 0.1;                                                        // this measures the force, which is used for the squishy box animation
        vsp = 0;
      }
    }
    playery += (short)vsp;                                                          // playery will never be decimal, but there are 'short' casts here just in case

    // Horizontal Collision Dectection
    if (place_meeting(playerx + hsp, playery, 3, 3)) {                              // if a collision is about to occur
      if (place_meeting(playerx, playery + sign(hsp), 3, 3))                        // if the wall is directly in front of you, set the potential movement to 0 (hsp represents the potential movement forwards, and the player only actually moves after these checks)
        hsp = 0;
      else {
        while (!(place_meeting(playerx + sign(hsp), playery, 3, 3))) {              // move as close to the object as possible without overlapping with it
          playerx += sign(hsp);
        }
        hsp = 0;                                                                    // the player is now directly next to the object, so don't continue moving
      }
    }
    playerx += hsp;                                                                 // actually move the player left/right



    // Draw the game to the LCD (now that the variables were just modified)
    drawplayer(shock, hsp + 2 * (level == 4));                                      // draw the player
    drawBad();                                                                      // draw the objects that kill you
    draw();                                                                         // draw the leveldata to the screen
    drawtolcd();                                                                    // draw everything to the lcd
    shock *= -0.9;                                                                  // reduce shock, while toggling it from positive to negative to toggle the shape from

    lcd.setCursor((playerx + 1 < 40) ? (15 - (lives >= 10) - (lives >= 100)) : (0), 0);     // this convoluted code displays the number of lives in either the top-left or the top-right, depending on the position of the player
    lcd.print(lives);
    lcd.setCursor((playerx + 1 < 40) ? (15 - (hsp_reading < 0)) : (0), 1);          // this convoluted code displays the potentiometer position in either the bottom-left or the bottom-right, depending on the position of the player
    lcd.print(hsp_reading);

    // Clear the screen variable (only the places in the VISIONRADIUS)
    for (short y = max(playery + 1 - VISIONRADIUS, 0); y <= min(playery + 1 + VISIONRADIUS, 15); y++) {
      for (short x = max((playerx + 1 - VISIONRADIUS) / 5, 0); x <= min((playerx + 1 + VISIONRADIUS) / 5, 15); x++) {
        screen[y][x] = 0;
      }
    }

    if (hspdisable > 0)                                                             // slowly lower hspdisable to 0, unless it is already at 0
      hspdisable--;

    playMusic(2, 95 - (30 * (level == 5)));                                         // because the music must be related to the tempo, when the music speeds up on the last level, the loop must follow
    // there must be a delay to avoid the obnoxious flashing. Because playMusic() is like a better delay() function, this was easily replaced and now music can be run during the game
    // At this point the code loops back to the start of the while loop
  }
}




// Game functions:
void draw() {
  // Draws each box to the screen
  for (short i = 0; i < sizeof(leveldata) / sizeof(leveldata[0]); i++) {
    drawtoscreen(leveldata[i][0], leveldata[i][1], leveldata[i][2], leveldata[i][3]);
  }
}

void drawBad() {                                                                    // outlines are used to represent the deadly boxes (this is why they must be at minimum 3x3)
  // Draws each killing box to the screen
  for (short i = 0; i < sizeof(killdata[level]) / sizeof(killdata[level][0]); i++) {
    drawOutline(killdata[level][i][0], killdata[level][i][1], killdata[level][i][2], killdata[level][i][3]);
  }
}

bool place_meeting(short x, short y, short w, short h) {                            // inspired by the function in Game Maker with the same name and purpose
  // Returns true if the box overlaps with any leveldata
  for (short i = 0; i < sizeof(leveldata) / sizeof(leveldata[0]); i++) {
    if (boxoverlap(x, y, w, h, leveldata[i][0], leveldata[i][1], leveldata[i][2], leveldata[i][3]))
      return true;
  }
  return false;
}

bool kill_meeting(short x, short y, short w, short h) {
  // Returns true if the box overlaps with any killdata
  for (short i = 0; i < sizeof(killdata[level]) / sizeof(killdata[level][0]); i++) {
    if (boxoverlap(x, y, w, h, killdata[level][i][0], killdata[level][i][1], killdata[level][i][2], killdata[level][i][3]))
      return true;
  }
  return false;
}

bool shoulddraw(short x, short y, short x2, short y2) {
  // Returns true if 2 pixels (x,y) and (x2,y2) are within VISIONRADIUS of each other
  if ((abs(x2 - x) > VISIONRADIUS) || (abs(y2 - y) > VISIONRADIUS))                 // this narrows the calculations down in some cases
    return false;

  if ((sqrt(pow(x - x2, 2) + pow(y - y2, 2))) > VISIONRADIUS)                       // this uses a modified pythagoras theorem to check for the distance between 2 points
    return false;
  return true;
}


void drawOutline(short x, short y, short w, short h) {
  drawtoscreen(x, y, w, 1);            // draws top part of box
  drawtoscreen(x, y, 1, h);            // draws left part of box
  drawtoscreen(x + w - 1, y, 1, h);    // draws right part of box
  drawtoscreen(x, y + h - 1, w, 1);    // draws bottom part of box
}

void drawtoscreen(short x, short y, short width, short height) {
  // This function draws a given box to the screen
  for (short pixely = y; pixely < y + height; pixely++) {
    for (short pixelx = x; pixelx < x + width; pixelx++) {
      if ((shoulddraw(pixelx, pixely, playerx + 1, playery + 1)) && (inscreen(pixelx, pixely)))     // this checks if the pixel is within a distance of the player, and is on the LCD
        screen[pixely][pixelx / 5] |= (B10000 >> pixelx % 5);                                       // ors the pixel where it should be in the array
    }
  }
}

bool inscreen(short x, short y) {
  // Returns true if the box is within the boundaries of the screen
  if ((x < 0) || (y < 0) || (x >= 80) || (y >= 16))
    return false;
  return true;
}

bool boxoverlap(short x1, short y1, short w1, short h1, short x2, short y2, short w2, short h2) {
  // Returns true if two boxes (x1,y1,w1,h1) and (x2,y2,w2,h2) overlap at any point
  if ((x1 > x2 + w2 - 1) || (x1 + w1 - 1 < x2) || (y1 > y2 + h2 - 1) || (y1 + h1 - 1 < y2))
    return false;
  return true;
}

void drawtolcd() {
  // Split the array into custom chars and print them
  lcd.clear();
  byte char_id = 0;
  for (short row = 0; row < 16; row += 8) { // 0,8
    for (short column = max((playerx - VISIONRADIUS) / 5, 0); column <= min((playerx + VISIONRADIUS) / 5, 15); column++) {
      byte glyph[8] =  {
        screen[row][column],
        screen[row + 1][column],
        screen[row + 2][column],
        screen[row + 3][column],
        screen[row + 4][column],
        screen[row + 5][column],
        screen[row + 6][column],
        screen[row + 7][column]
      };

      lcd.createChar(char_id, glyph);
      lcd.setCursor(column, row / 8);
      lcd.write((byte)char_id);
      char_id++;
    }
  }
}

void drawplayer(float shock, int hsp) {
  if ((abs(shock) > 0.2) && (place_meeting(playerx, playery + 1, 3, 3))) {                          // if the shock value is high enough to be worth using (this value decreases through multiplication, which never actually reaches 0)
    if (shock > 0)
      drawtoscreen(playerx - 1, playery + 1, 5, 2);
    else                                                                                            // shock toggles from positive to negative, so this code alternates the shape of the box from a (width, height) of (5,2) to (1,4), creating a squishy effect
      drawtoscreen(playerx + 1, playery - 1, 1, 4); //                                                 this effect was added simply to make the game look nicer
  }
  else {                                                                                            // if there is no shock, or it has reached a point where it is too low to be affected greatly by 'decreasing-by-multiplication'
    if ((hsp == 0) || (!(place_meeting(playerx, playery + 1, 3, 3))))
      drawtoscreen(playerx, playery, 3, 3);                                                         // draw the player normally if they are grounded or not moving. If they are doing either, use a convoluted single line of code to draw them instead
    else {
      for (byte i = 0; i < (3 - (abs(hsp) < 3)); i++) {                                             // 0,1,2 (if not moving at max speed) and 0,1 (if moving at max speed)
        drawtoscreen(playerx + sign(hsp) * ((3 - (abs(hsp) < 3)) - i - 1), playery + i + ((abs(hsp) == 2) && (i == 1)), 3, ((abs(hsp) < 3) && (abs(hsp) - (((3 - (abs(hsp) < 3)) - i - 1))) == 1) + 1);
      }                                                                                             // the purpose of this code is to create a diagonal effect when the player moves. The effect is more noticeable when moving at a higher speed.
    } //                                                                                               this was done to make the game seem more realistic, and to differentiate the moveable box from its otherwise identical surroundings
  }   //                                                                                               it also somewhat personifies the box, as its top half can be seen as it leaning forward and it seems like it's a person moving quickly (just not a human shape)
}

void moveobjects() {
  for (byte i = 0; i < sizeof(killdata[level]) / sizeof(killdata[level][0]); i++) {
    killdata[level][i][1] = (killdata[level][i][1] + killdata[level][i][4]) % 16;
  }
}

void writehighscores(byte score) {
  lcd.clear();
  musicReset();


  /* EEPROM DIAGRAM
    13200:   #1 highest score                13207:   #2 highest score
    13201:   first letter of #1's name       13208:   first letter of #2's name
    13202:   second letter of #1's name      etc.
    13203:   third letter of #1's name
    13204:   fourth letter of #1's name      13228:   #5 highest score
    13205:   fifth letter of #1's name       13234:   sixth letter of #5's name
    13206:   sixth letter of #1's name       13235:   IRRELEVANT/UNUSED
  */

  if (score > EEPROM_ed1.readB(13228)) {                                            // the 5th best score (if you are worthy of being on the leaderboard run this code)
    lcd.setCursor(2, 0);
    lcd.print("Enter name:");

    // Get name
    byte namepos = 0;                                                               // when this is 0 the first character is being entered, when this is 5 the sixth (the last) character is being entered
    char inputname[] = {0, 0, 0, 0, 0, 0};                                          // the name that the player is entering
    while (namepos < 6) {
      while (!buttonPressed(15)) {
        inputname[namepos] = (map(analogRead(3), 0, 1023, 91, 65) == 91) ? 95 : map(analogRead(3), 0, 1023, 91, 65);                  // 65 is A, 90 is Z, 91 is _
        lcd.setCursor(15 - namepos, 1);                                             // essentially aligns it to the right of the screen (in the bottom row)
        lcd.print(inputname);
        playMusic(3, 11);                                                           // the value is 11 because it is the denominator of the fraction that resulted from the calculation to find this song's tempo, meaning that 11 is a multiple of that tempo
      }
      namepos++;                                                                    // move on to the next letter
    }

    // Update highscore list
    byte pos = 4;                                                                   // 'pos' represents the position of the score on the list. If the score is bigger than the score above it, it will be moved up (pos will decrease). 0 is the highest pos
    while (pos >= 0) {
      if (score <= EEPROM_ed1.readB(13200 + ((pos - 1) * 7))) {                     // if the new score is less than the score above its 'pos': place it in (run the code below). (pos-1) is the score above it
        // At this point, the pos is where the score deserves to be on the final list; 'pos' will never change from here.


        // Slide the scores and names down to make room for the new value (unless it's at the bottom of the list where pos == 4)
        if (pos < 4) {
          for (byte slideS = 4; slideS > pos; slideS--) {                                                                // 4,3 ... pos+2,pos+1  (this variable slides the score values down to make room for the new score)
            EEPROM_ed1.sendB(13200 + (slideS * 7), EEPROM_ed1.readB(13200 + ((slideS - 1) * 7)));                        // to each position under the position where the new score will end up, set it equal to the one above it
            for (byte slideN = 1; slideN < 7; slideN++) {                                                                // 1,2,3,4,5,6  (this variable slides the name values down to make room for the new name)
              EEPROM_ed1.sendB(13200 + (slideS * 7) + slideN, EEPROM_ed1.readB(13200 + ((slideS - 1) * 7) + slideN));    // same as scores, except instead of 0,7,14 ... changing, it is: 1-6,8-13,15-20 ...
            }
          }
        }

        // Now that the scores have moved down by 1 pos, put in the new score
        EEPROM_ed1.sendB(13200 + (pos * 7), score);                                 // the new score is placed
        for (byte n = 1; n < 7; n++) { // 1,2,3,4,5,6
          EEPROM_ed1.sendB(13200 + (pos * 7) + n, (byte)inputname[n - 1]);          // the new name is placed
        }
        break;                                                                      // break so that it only does this whole procedure once
      }
      pos--;                                                                        // if the 'if statement' doesn't trigger (the score must move up), move the 'pos' of the score up and try the 'if statement' again
    }
  }
  else {                                                                            // if  the score is too small to be on the list (this is the else from the 'if statement' ages ago, before the while loop)
    lcd.print("Your score:");
    lcd.setCursor(15 - (score >= 10) - (score >= 100), 0);                          // if the score is double or triple digits, start printing extra characters over (effectively aligning it to the right of the screen)
    lcd.print(score);
    lcd.setCursor(3, 1);
    lcd.print("(Push D15)");

    while (digitalRead(15)) {                                                       // stall until you press the button, then go back to the menu
    }
  }

  musicReset();
  state = 4;                                                                        // send the player to highscores screen
}


void highscores() {                                                                 // there are no cases of musicReset() in either menu() or highscores() because the song for these screens must play continuously between them
  while (!buttonPressed(15)) {
    byte screenpos = map(analogRead(3), 0, 1023, 4, 0);
    lcd.setCursor(0, 0);
    for (byte i = 1; i < 7; i++) {
      lcd.print((char)EEPROM_ed1.readB(13200 + 7 * screenpos + i));
    }
    byte score = EEPROM_ed1.readB(13200 + 7 * screenpos);
    lcd.setCursor(15 - (score >= 10) - (score >= 100), 0);
    lcd.print(score);
    if (screenpos < 4) {
      lcd.setCursor(0, 1);
      for (byte i = 1; i < 7; i++) {
        lcd.print((char)EEPROM_ed1.readB(13200 + 7 * screenpos + 7 + i));
      }
      score = EEPROM_ed1.readB(13200 + 7 * screenpos + 7);
      lcd.setCursor(15 - (score >= 10) - (score >= 100), 1);                        // this (score >= 10 or 100) code is used many times to align code to the right
      lcd.print(score);
    }
    playMusic(1, 62.5);
    lcd.clear();
  }
  state = 2;
}




/*
  This function plays a part of a song (determined by note, audibleLength, durationsLeft and skip_note). This essentially functions as delay(), except the delay is spent playing music.
  When 'amount' is '200', the code will pause on that line and a song will play for 200 frames. When the code is called again, it will continue. If this is done after the previous one, it will play continous sound.
  This function allows for music to be played during any loop of code that doesn't delay between it, only at the end.
*/

void playMusic(byte song, float amount) {
  if (disableMusic == 1)                                                            // even if music is disabled, this must still function as a delay()
    delay(amount);
  else {
    // Declare music variables (these don't determine how the code's progress in the song so they aren't global, and they aren't in the switch otherwise they will be lost after the switch)
    float audibleTime;                                                              // audibleTime is the percentage of the note that is audible (the remaining percent is silent and is used to distinguish notes)
    float tempo;                                                                    // tempo represents the amount of frames a duration of 1 will play for (including the silence after the note)
    short melody[116];                                                              // melody[] is the series of notes
    byte durations[115];                                                            // each duration is the amount of "tempo"s the note will go for (when (duration = 2) and (tempo = 100) the note will play for 200 frames, some of which at the end will be silent)

    /*
      The values of the following are presented in the long way they are because any other previously conceived way contained issues with global and local arrays and declaration of pre-existing arrays.
      These were not typed out by hand, a program (songmaker) took the previous melodies and durations and printed each of these to the serial monitor, where they were copy-pasted into this code.
      Skip to line 1600 if you do not want to read all of the notes
    */

    // This is used instead of a switch statement because the switch kills variables
    if (song == 0) { // childhood memories of winter
      audibleTime = 0.97;
      tempo = 187.5;
      melody[0] = 988;
      melody[1] = 1047;
      melody[2] = 988;
      melody[3] = 880;
      melody[4] = 659;
      melody[5] = 880;
      melody[6] = 988;
      melody[7] = 1047;
      melody[8] = 1319;
      melody[9] = 1175;
      melody[10] = 988;
      melody[11] = 988;
      melody[12] = 1047;
      melody[13] = 988;
      melody[14] = 880;
      melody[15] = 659;
      melody[16] = 880;
      melody[17] = 988;
      melody[18] = 1047;
      melody[19] = 988;
      melody[20] = 784;
      melody[21] = 740;
      melody[22] = 784;
      melody[23] = 740;
      melody[24] = 659;
      melody[25] = 494;
      melody[26] = 659;
      melody[27] = 740;
      melody[28] = 784;
      melody[29] = 988;
      melody[30] = 1047;
      melody[31] = 880;
      melody[32] = 740;
      melody[33] = 784;
      melody[34] = 740;
      melody[35] = 659;
      melody[36] = 494;
      melody[37] = 659;
      melody[38] = 740;
      melody[39] = 784;
      melody[40] = 740;
      melody[41] = 587;
      melody[42] = 784;
      melody[43] = 880;
      melody[44] = 784;
      melody[45] = 698;
      melody[46] = 523;
      melody[47] = 698;
      melody[48] = 784;
      melody[49] = 880;
      melody[50] = 1047;
      melody[51] = 988;
      melody[52] = 784;
      melody[53] = 784;
      melody[54] = 880;
      melody[55] = 784;
      melody[56] = 698;
      melody[57] = 523;
      melody[58] = 698;
      melody[59] = 784;
      melody[60] = 880;
      melody[61] = 784;
      melody[62] = 698;
      melody[63] = 880;
      melody[64] = 988;
      melody[65] = 880;
      melody[66] = 784;
      melody[67] = 587;
      melody[68] = 784;
      melody[69] = 880;
      melody[70] = 988;
      melody[71] = 1175;
      melody[72] = 1047;
      melody[73] = 880;
      melody[74] = 880;
      melody[75] = 988;
      melody[76] = 880;
      melody[77] = 784;
      melody[78] = 587;
      melody[79] = 784;
      melody[80] = 880;
      melody[81] = 988;
      melody[82] = 880;
      melody[83] = 784;
      melody[84] = 0;
      durations[0] = 1;
      durations[1] = 1;
      durations[2] = 1;
      durations[3] = 1;
      durations[4] = 10;
      durations[5] = 1;
      durations[6] = 1;
      durations[7] = 4;
      durations[8] = 4;
      durations[9] = 4;
      durations[10] = 4;
      durations[11] = 1;
      durations[12] = 1;
      durations[13] = 1;
      durations[14] = 1;
      durations[15] = 10;
      durations[16] = 1;
      durations[17] = 1;
      durations[18] = 4;
      durations[19] = 4;
      durations[20] = 8;
      durations[21] = 1;
      durations[22] = 1;
      durations[23] = 1;
      durations[24] = 1;
      durations[25] = 10;
      durations[26] = 1;
      durations[27] = 1;
      durations[28] = 4;
      durations[29] = 4;
      durations[30] = 4;
      durations[31] = 4;
      durations[32] = 1;
      durations[33] = 1;
      durations[34] = 1;
      durations[35] = 1;
      durations[36] = 10;
      durations[37] = 1;
      durations[38] = 1;
      durations[39] = 4;
      durations[40] = 4;
      durations[41] = 8;
      durations[42] = 1;
      durations[43] = 1;
      durations[44] = 1;
      durations[45] = 1;
      durations[46] = 10;
      durations[47] = 1;
      durations[48] = 1;
      durations[49] = 4;
      durations[50] = 4;
      durations[51] = 4;
      durations[52] = 4;
      durations[53] = 1;
      durations[54] = 1;
      durations[55] = 1;
      durations[56] = 1;
      durations[57] = 10;
      durations[58] = 1;
      durations[59] = 1;
      durations[60] = 4;
      durations[61] = 4;
      durations[62] = 8;
      durations[63] = 1;
      durations[64] = 1;
      durations[65] = 1;
      durations[66] = 1;
      durations[67] = 10;
      durations[68] = 1;
      durations[69] = 1;
      durations[70] = 4;
      durations[71] = 4;
      durations[72] = 4;
      durations[73] = 4;
      durations[74] = 1;
      durations[75] = 1;
      durations[76] = 1;
      durations[77] = 1;
      durations[78] = 10;
      durations[79] = 1;
      durations[80] = 1;
      durations[81] = 4;
      durations[82] = 4;
      durations[83] = 8;
    }
    if (song == 1) { // gravity duck song
      audibleTime = 1;
      tempo = 125;
      melody[0] = 392;
      melody[1] = 294;
      melody[2] = 392;
      melody[3] = 294;
      melody[4] = 392;
      melody[5] = 294;
      melody[6] = 392;
      melody[7] = 440;
      melody[8] = 466;
      melody[9] = 349;
      melody[10] = 466;
      melody[11] = 349;
      melody[12] = 466;
      melody[13] = 349;
      melody[14] = 466;
      melody[15] = 440;
      melody[16] = 349;
      melody[17] = 262;
      melody[18] = 349;
      melody[19] = 262;
      melody[20] = 349;
      melody[21] = 262;
      melody[22] = 349;
      melody[23] = 392;
      melody[24] = 311;
      melody[25] = 233;
      melody[26] = 311;
      melody[27] = 233;
      melody[28] = 311;
      melody[29] = 233;
      melody[30] = 311;
      melody[31] = 349;
      melody[32] = 0;
      durations[0] = 2;
      durations[1] = 2;
      durations[2] = 2;
      durations[3] = 2;
      durations[4] = 2;
      durations[5] = 2;
      durations[6] = 2;
      durations[7] = 2;
      durations[8] = 2;
      durations[9] = 2;
      durations[10] = 2;
      durations[11] = 2;
      durations[12] = 2;
      durations[13] = 2;
      durations[14] = 2;
      durations[15] = 2;
      durations[16] = 2;
      durations[17] = 2;
      durations[18] = 2;
      durations[19] = 2;
      durations[20] = 2;
      durations[21] = 2;
      durations[22] = 2;
      durations[23] = 2;
      durations[24] = 2;
      durations[25] = 2;
      durations[26] = 2;
      durations[27] = 2;
      durations[28] = 2;
      durations[29] = 2;
      durations[30] = 2;
      durations[31] = 2;
    }
    if (song == 2) { // ninja gaiden unbreakable determination
      audibleTime = 0.95;
      tempo = 95 - (30 * (level == 5));
      melody[0] = 370;
      melody[1] = 440;
      melody[2] = 370;
      melody[3] = 494;
      melody[4] = 330;
      melody[5] = 370;
      melody[6] = 330;
      melody[7] = 370;
      melody[8] = 554;
      melody[9] = 370;
      melody[10] = 415;
      melody[11] = 440;
      melody[12] = 415;
      melody[13] = 370;
      melody[14] = 415;
      melody[15] = 370;
      melody[16] = 330;
      melody[17] = 494;
      melody[18] = 415;
      melody[19] = 370;
      melody[20] = 415;
      melody[21] = 370;
      melody[22] = 415;
      melody[23] = 554;
      melody[24] = 415;
      melody[25] = 440;
      melody[26] = 494;
      melody[27] = 440;
      melody[28] = 415;
      melody[29] = 370;
      melody[30] = 440;
      melody[31] = 740;
      melody[32] = 494;
      melody[33] = 330;
      melody[34] = 370;
      melody[35] = 330;
      melody[36] = 370;
      melody[37] = 554;
      melody[38] = 370;
      melody[39] = 415;
      melody[40] = 440;
      melody[41] = 415;
      melody[42] = 370;
      melody[43] = 415;
      melody[44] = 370;
      melody[45] = 330;
      melody[46] = 494;
      melody[47] = 415;
      melody[48] = 370;
      melody[49] = 415;
      melody[50] = 370;
      melody[51] = 415;
      melody[52] = 277;
      melody[53] = 415;
      melody[54] = 440;
      melody[55] = 494;
      melody[56] = 440;
      melody[57] = 415;
      melody[58] = 370;
      melody[59] = 330;
      melody[60] = 294;
      melody[61] = 277;
      melody[62] = 277;
      melody[63] = 277;
      melody[64] = 277;
      melody[65] = 277;
      melody[66] = 277;
      melody[67] = 370;
      melody[68] = 415;
      melody[69] = 440;
      melody[70] = 415;
      melody[71] = 440;
      melody[72] = 370;
      melody[73] = 554;
      melody[74] = 494;
      melody[75] = 440;
      melody[76] = 494;
      melody[77] = 440;
      melody[78] = 494;
      melody[79] = 370;
      melody[80] = 440;
      melody[81] = 415;
      melody[82] = 370;
      melody[83] = 330;
      melody[84] = 370;
      melody[85] = 415;
      melody[86] = 659;
      melody[87] = 587;
      melody[88] = 554;
      melody[89] = 494;
      melody[90] = 554;
      melody[91] = 370;
      melody[92] = 415;
      melody[93] = 440;
      melody[94] = 415;
      melody[95] = 440;
      melody[96] = 370;
      melody[97] = 554;
      melody[98] = 494;
      melody[99] = 440;
      melody[100] = 494;
      melody[101] = 440;
      melody[102] = 494;
      melody[103] = 370;
      melody[104] = 440;
      melody[105] = 415;
      melody[106] = 370;
      melody[107] = 330;
      melody[108] = 370;
      melody[109] = 415;
      melody[110] = 659;
      melody[111] = 587;
      melody[112] = 554;
      melody[113] = 494;
      melody[114] = 554;
      melody[115] = 0;        // the final value (melody[115]) is 0, indicating the melody is over
      durations[0] = 2;
      durations[1] = 2;
      durations[2] = 2;
      durations[3] = 2;
      durations[4] = 1;
      durations[5] = 2;
      durations[6] = 1;
      durations[7] = 2;
      durations[8] = 2;
      durations[9] = 6;
      durations[10] = 2;
      durations[11] = 2;
      durations[12] = 2;
      durations[13] = 4;
      durations[14] = 2;
      durations[15] = 1;
      durations[16] = 1;
      durations[17] = 2;
      durations[18] = 2;
      durations[19] = 1;
      durations[20] = 2;
      durations[21] = 1;
      durations[22] = 2;
      durations[23] = 2;
      durations[24] = 6;
      durations[25] = 2;
      durations[26] = 2;
      durations[27] = 2;
      durations[28] = 4;
      durations[29] = 2;
      durations[30] = 2;
      durations[31] = 2;
      durations[32] = 2;
      durations[33] = 1;
      durations[34] = 2;
      durations[35] = 1;
      durations[36] = 2;
      durations[37] = 2;
      durations[38] = 6;
      durations[39] = 2;
      durations[40] = 2;
      durations[41] = 2;
      durations[42] = 4;
      durations[43] = 2;
      durations[44] = 1;
      durations[45] = 1;
      durations[46] = 2;
      durations[47] = 2;
      durations[48] = 1;
      durations[49] = 2;
      durations[50] = 1;
      durations[51] = 2;
      durations[52] = 2;
      durations[53] = 6;
      durations[54] = 2;
      durations[55] = 2;
      durations[56] = 2;
      durations[57] = 4;
      durations[58] = 4;
      durations[59] = 4;
      durations[60] = 4;
      durations[61] = 4;
      durations[62] = 2;
      durations[63] = 2;
      durations[64] = 2;
      durations[65] = 2;
      durations[66] = 4;
      durations[67] = 2;
      durations[68] = 2;
      durations[69] = 2;
      durations[70] = 2;
      durations[71] = 2;
      durations[72] = 4;
      durations[73] = 2;
      durations[74] = 2;
      durations[75] = 2;
      durations[76] = 2;
      durations[77] = 2;
      durations[78] = 2;
      durations[79] = 4;
      durations[80] = 2;
      durations[81] = 2;
      durations[82] = 2;
      durations[83] = 2;
      durations[84] = 2;
      durations[85] = 2;
      durations[86] = 4;
      durations[87] = 2;
      durations[88] = 2;
      durations[89] = 2;
      durations[90] = 12;
      durations[91] = 2;
      durations[92] = 2;
      durations[93] = 2;
      durations[94] = 2;
      durations[95] = 2;
      durations[96] = 4;
      durations[97] = 2;
      durations[98] = 2;
      durations[99] = 2;
      durations[100] = 2;
      durations[101] = 2;
      durations[102] = 2;
      durations[103] = 4;
      durations[104] = 2;
      durations[105] = 2;
      durations[106] = 2;
      durations[107] = 2;
      durations[108] = 2;
      durations[109] = 2;
      durations[110] = 4;
      durations[111] = 2;
      durations[112] = 2;
      durations[113] = 2;
      durations[114] = 16;
    }
    if (song == 3) { // lemmings 2 (PC/DOS) ending theme
      audibleTime = 0.97;
      tempo = 1500 / 11;
      melody[0] = 523;
      melody[1] = 523;
      melody[2] = 466;
      melody[3] = 523;
      melody[4] = 622;
      melody[5] = 622;
      melody[6] = 523;
      melody[7] = 466;
      melody[8] = 523;
      melody[9] = 523;
      melody[10] = 523;
      melody[11] = 466;
      melody[12] = 523;
      melody[13] = 622;
      melody[14] = 622;
      melody[15] = 523;
      melody[16] = 466;
      melody[17] = 523;
      melody[18] = 523;
      melody[19] = 523;
      melody[20] = 466;
      melody[21] = 523;
      melody[22] = 622;
      melody[23] = 622;
      melody[24] = 523;
      melody[25] = 466;
      melody[26] = 523;
      melody[27] = 523;
      melody[28] = 523;
      melody[29] = 466;
      melody[30] = 523;
      melody[31] = 622;
      melody[32] = 622;
      melody[33] = 523;
      melody[34] = 466;
      melody[35] = 523;
      melody[36] = 262;
      melody[37] = 294;
      melody[38] = 311;
      melody[39] = 349;
      melody[40] = 311;
      melody[41] = 349;
      melody[42] = 370;
      melody[43] = 392;
      melody[44] = 370;
      melody[45] = 392;
      melody[46] = 370;
      melody[47] = 349;
      melody[48] = 370;
      melody[49] = 392;
      melody[50] = 370;
      melody[51] = 392;
      melody[52] = 370;
      melody[53] = 262;
      melody[54] = 294;
      melody[55] = 311;
      melody[56] = 349;
      melody[57] = 311;
      melody[58] = 349;
      melody[59] = 370;
      melody[60] = 392;
      melody[61] = 370;
      melody[62] = 392;
      melody[63] = 370;
      melody[64] = 349;
      melody[65] = 370;
      melody[66] = 392;
      melody[67] = 370;
      melody[68] = 392;
      melody[69] = 262;
      melody[70] = 262;
      melody[71] = 392;
      melody[72] = 523;
      melody[73] = 466;
      melody[74] = 466;
      melody[75] = 523;
      melody[76] = 392;
      melody[77] = 466;
      melody[78] = 392;
      melody[79] = 349;
      melody[80] = 311;
      melody[81] = 262;
      melody[82] = 392;
      melody[83] = 523;
      melody[84] = 466;
      melody[85] = 466;
      melody[86] = 523;
      melody[87] = 392;
      melody[88] = 466;
      melody[89] = 392;
      melody[90] = 349;
      melody[91] = 349;
      melody[92] = 262;
      melody[93] = 392;
      melody[94] = 523;
      melody[95] = 466;
      melody[96] = 466;
      melody[97] = 523;
      melody[98] = 392;
      melody[99] = 466;
      melody[100] = 392;
      melody[101] = 349;
      melody[102] = 311;
      melody[103] = 262;
      melody[104] = 392;
      melody[105] = 523;
      melody[106] = 466;
      melody[107] = 466;
      melody[108] = 523;
      melody[109] = 392;
      melody[110] = 466;
      melody[111] = 349;
      melody[112] = 262;
      melody[113] = 262;
      melody[114] = 0;
      durations[0] = 2;
      durations[1] = 2;
      durations[2] = 1;
      durations[3] = 2;
      durations[4] = 2;
      durations[5] = 1;
      durations[6] = 2;
      durations[7] = 1;
      durations[8] = 3;
      durations[9] = 2;
      durations[10] = 2;
      durations[11] = 1;
      durations[12] = 2;
      durations[13] = 2;
      durations[14] = 1;
      durations[15] = 2;
      durations[16] = 1;
      durations[17] = 3;
      durations[18] = 2;
      durations[19] = 2;
      durations[20] = 1;
      durations[21] = 2;
      durations[22] = 2;
      durations[23] = 1;
      durations[24] = 2;
      durations[25] = 1;
      durations[26] = 3;
      durations[27] = 2;
      durations[28] = 2;
      durations[29] = 1;
      durations[30] = 2;
      durations[31] = 2;
      durations[32] = 1;
      durations[33] = 2;
      durations[34] = 1;
      durations[35] = 3;
      durations[36] = 6;
      durations[37] = 6;
      durations[38] = 6;
      durations[39] = 6;
      durations[40] = 4;
      durations[41] = 4;
      durations[42] = 3;
      durations[43] = 3;
      durations[44] = 3;
      durations[45] = 3;
      durations[46] = 2;
      durations[47] = 2;
      durations[48] = 3;
      durations[49] = 3;
      durations[50] = 3;
      durations[51] = 3;
      durations[52] = 4;
      durations[53] = 6;
      durations[54] = 6;
      durations[55] = 6;
      durations[56] = 6;
      durations[57] = 4;
      durations[58] = 4;
      durations[59] = 3;
      durations[60] = 3;
      durations[61] = 3;
      durations[62] = 3;
      durations[63] = 2;
      durations[64] = 2;
      durations[65] = 3;
      durations[66] = 3;
      durations[67] = 3;
      durations[68] = 3;
      durations[69] = 4;
      durations[70] = 1;
      durations[71] = 1;
      durations[72] = 1;
      durations[73] = 1;
      durations[74] = 1;
      durations[75] = 2;
      durations[76] = 1;
      durations[77] = 2;
      durations[78] = 2;
      durations[79] = 2;
      durations[80] = 2;
      durations[81] = 1;
      durations[82] = 1;
      durations[83] = 1;
      durations[84] = 1;
      durations[85] = 1;
      durations[86] = 2;
      durations[87] = 1;
      durations[88] = 2;
      durations[89] = 2;
      durations[90] = 2;
      durations[91] = 2;
      durations[92] = 1;
      durations[93] = 1;
      durations[94] = 1;
      durations[95] = 1;
      durations[96] = 1;
      durations[97] = 2;
      durations[98] = 1;
      durations[99] = 2;
      durations[100] = 2;
      durations[101] = 2;
      durations[102] = 2;
      durations[103] = 1;
      durations[104] = 1;
      durations[105] = 1;
      durations[106] = 1;
      durations[107] = 1;
      durations[108] = 2;
      durations[109] = 1;
      durations[110] = 2;
      durations[111] = 2;
      durations[112] = 2;
      durations[113] = 2;
    }




    // When this code first runs, durationsLeft is initalised to 0 so that this bit runs, and note is -1 (or 255) so that it becomes 0 in this section of code.
    // If the note is complete:
    if (durationsLeft == 0) {

      // Add 1 to 'note', but set it to 0 if the song ends
      note++;
      note *= (melody[note] != 0);                         //  if melody[note] DOES equal 0, note will multiply by 0 and become 0, resetting the song. Otherwise, it simply multiplies by 1 and has no effect.

      // Reset the variables
      skip_note = 1;                                       //  since all notes must be played for any amount of time (the duration is never 0)
      noteLength = durations[note] * tempo;                //  this is how many frames the note will take up
      audibleLength = noteLength * audibleTime;            //  this is how many frames will be audible (the one used in the calculations below)
      durationsLeft = ceil(noteLength / amount);           //  the total duration of the note divided by the time this takes to loop will be the number of times it loops
      //                                                   // (the biggest constaint of this poem is that the song delays too much if 'tempo' is not a multiple of 'amount'
      //                                                   // (another potential issue is that two notes cannot be played in one 'amount', but this is not a problem in this code)
    }

    // The variables have now been set up, actually play the music now
    if (skip_note == 1) {
      tone(9, melody[note]);                               // play the note

      if (audibleLength > amount) {                        // if the note should be played longer than the duration of the 'amount', just keep playing the note
        delay(amount);
        audibleLength -= amount;                           // 'amount' frames of the 'audibleLength' were just played
      }
      else {                                               // if the note will end during the 'amount', only play the note for som of the loop
        delay(audibleLength);
        noTone(9);
        delay(amount - audibleLength);                     // these two delays add up to 'amount'
        skip_note = 0;                                     // now that the note is over, if durationsLeft not being 0 forces this note to play again before the next note, don't even bother playing any sound
      }
    }
    else
      delay(amount);                                       // don't even bother playing the note (there is already no note playing from the noTone() earlier)

    durationsLeft--;                                       // because 1 loop has just occurred, subtract 1 from the amount of loops until the next note begins
  }
}

void musicReset() {                                        // this function resets the variables to prepare for a new song to be played, and to stop a continuous note from playing indefinitely
  noTone(9);
  note = -1;
  durationsLeft = 0;
}
