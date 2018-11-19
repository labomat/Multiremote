/**************************************************************************************
 * 
 * Multiremote
 * ===========
 * a remote for rc function models with multiple digital and analog inputs via serial transmission (433 Mhz or 2,4 GHz).
 * Custom tx modules provide special controls for different use cases (crane, fire monitor, nautical lights, etc.)
 * Digital Servo functionality by Pikoder SSC (www.pikoder.de)
 * 
 * 
 * Slave module #02 for Multiremote (lights)
 * ==========================================
 * 
 * Slave module gathers input from digital or analog sources and sends it to a master device via i2c.
 * Module layout can be individual configured (configuration must be shared with master unit)
 * 
 * Module displays selection of lighting options on an oled display to choose via rotary dial. Selected options are marked.  
 * 
 * INPUTS:
 * 1x rotary encoder
 * 2x digital in / push buttons
 * 
 * OUTPUT:
 * 1,3" oled display
 * 24 digital on/off / 3 bytes of data
 *  
 * Written by Kai Laborenz with the help of all the beautiful examples out there
 * This is Open Source: GNU GPL v3 https://choosealicense.com/licenses/gpl-3.0/
 * 
 **************************************************************************************/

#include "U8glib.h"
#include <Rotary.h>

// states and messages
#define MENU_NONE 0
#define MENU_PREV 1
#define MENU_NEXT 2
#define MENU_SELECT 3
#define MENU_LEFT_ACTIVE 4
#define MENU_RIGHT_ACTIVE 5

#define PIN_LED 9     // Activity LED
#define PIN_SWITCH 8  // Switch Menu
#define PIN_SELECT 3  // Select button on rotary encoder

U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);  // define Display

Rotary r = Rotary(2, 4);                     // define rotary encoder at pin 2 und 4 (push button at pin 3)

#define MENU_ITEMS 5
const char *menu_left_default[MENU_ITEMS] = { "Fahrt", "Anker", "Taucher", "Schlepp", "UW Arbeit" };

char *menu_left[MENU_ITEMS] = { "", "", "", "", "" };
char *menu_right[MENU_ITEMS] = { "", "", "", "", "" };

uint8_t menuCodeFirst = MENU_NONE;  // menu code states
uint8_t menuCodeSecond = MENU_NONE; //
uint8_t menuCode = MENU_NONE;       //

uint8_t menu_current = 0;                 // current menu item
uint8_t menu_active = MENU_LEFT_ACTIVE;   // wich menu is active (left or right)

int buttonPushCounter = 0;          // counter for the number of button presses
int buttonState = 0;                // current state of the button
int lastButtonState = 0;            // previous state of the button

uint8_t menu_redraw_required = 0;
uint8_t last_code = MENU_NONE;

uint8_t menu_right_size = 0;
uint8_t menu_left_size = MENU_ITEMS;


void uiStep(void) {

  unsigned char result = r.process();

  if (result == DIR_CW) {                     // rotary encoder dial
    menuCodeFirst = MENU_NEXT;
  }
  else if (result == DIR_CCW) {
    menuCodeFirst = MENU_PREV;
  }
  else if ( digitalRead(PIN_SELECT) == LOW ){ // rotary encoder switch
    menuCodeFirst = MENU_SELECT;
    digitalWrite(PIN_LED, 1);
  }
  else {
    menuCodeFirst = MENU_NONE;
    digitalWrite(PIN_LED, 0);
  }

  menuCodeSecond = menuCodeFirst;
  
  if ( menuCodeSecond == menuCodeFirst )
    menuCode = menuCodeFirst;
  else
    menuCode = MENU_NONE;


}


void drawMenu(void) {
  uint8_t i, h;
  u8g_uint_t w, d, d_right;
  
  h = u8g.getFontAscent()-u8g.getFontDescent();       // height of single line
  w = (u8g.getWidth()/2)-2;                           // width of menu (half of the screen minus 2 pixels)
  d = 7;                                              // left indent (text starts at d pixel )
  d_right = (u8g.getWidth()/2)+2+d;                   // left indent for right menu

  u8g.drawVLine(u8g.getWidth()/2, 0, u8g.getHeight()); // draw separator line
  
  u8g.setFont(u8g_font_6x13);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  
   // left menu    
   for( i = 0; i < menu_left_size; i++ ) {
   
    u8g.setDefaultForegroundColor();
    
    if ( i == menu_current && menu_active == MENU_LEFT_ACTIVE) {  //if left menu active indicate menu position
        u8g.setFont(u8g_font_6x13_75r);                           // set iconfont 
        u8g.drawStr(0, (i+1)*h, "8");                             // draw triangle
        u8g.setFont(u8g_font_6x13B);                              // set font back to normal
        u8g.setFontRefHeightText();
        u8g.setFontPosTop();
    }
    else {                                                        // else just draw menu
        u8g.setFont(u8g_font_6x13);
        u8g.setFontRefHeightText();
        u8g.setFontPosTop();
    }
    u8g.drawStr(d, i*h, menu_left[i]);
  }

    // right menu
    for( i = 0; i < menu_left_size; i++ ) {
   
    u8g.setDefaultForegroundColor();
    
    if ( i == menu_current && menu_active == MENU_RIGHT_ACTIVE){
        u8g.setFont(u8g_font_6x13_75r);
        u8g.drawStr(u8g.getWidth()/2+2, (i+1)*h, "8");
        u8g.setFont(u8g_font_6x13B);
        u8g.setFontRefHeightText();
        u8g.setFontPosTop();
    }
    else {
        u8g.setFont(u8g_font_6x13);
        u8g.setFontRefHeightText();
        u8g.setFontPosTop();
    }
    u8g.drawStr(d_right, i*h, menu_right[i]);
  }
}

void updateMenu(void) {
  if ( menuCode != MENU_NONE && last_code == menuCode ) {
    return;
  }
  last_code = menuCode;

  // switch from left to right menu by button push
  // to switch between two states button push is counted and divided by two
  // even numbers define state MENU_LEFT_ACTIVE, uneven numbers MENU_RIGHT_ACTIVE

  buttonState = digitalRead(PIN_SWITCH);

  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      buttonPushCounter++;
    } 
    else {
    }
  }
  delay(100);
  lastButtonState = buttonState;
  
  if (buttonPushCounter % 2 == 0) {     
    menu_active = MENU_LEFT_ACTIVE;
    digitalWrite(PIN_LED, HIGH);
  } else {
    menu_active = MENU_RIGHT_ACTIVE;
    digitalWrite(PIN_LED, LOW);
  }

  // menu selection
  
  switch ( menuCode ) {
    case MENU_NEXT:
      menu_current++;
      if ( menu_current >= MENU_ITEMS )
        menu_current = 0;
      menu_redraw_required = 1;
      break;
    case MENU_PREV:
      if ( menu_current == 0 )
        menu_current = MENU_ITEMS;
      menu_current--;
      menu_redraw_required = 1;
      break;
    case MENU_SELECT:

    if ( menu_active == MENU_LEFT_ACTIVE){                    // click when left menu (available options) is active
      
      menu_right[menu_right_size] = menu_left[menu_current];  // current item from left -> right     
      menu_right_size++;                                      // increase right menu size
      menu_left_size--;                                       // decrease left menu size

      for (int j=menu_current; j<menu_left_size; j++) {       // move all items after transfered item one position up
        menu_left[j] = menu_left[j+1];
      }
      delay(100);   
    }
    else if ( menu_active == MENU_RIGHT_ACTIVE){                  // click when right menu (selected options) is active

      int wantedpos = -1;                                         // on what original position in left menu was selected item?
      for (int i=0; i<MENU_ITEMS; i++) {                  
         if (menu_right[menu_current] == menu_left_default[i]) {  // original positions are in default options array
          wantedpos = i;
          break;
         }
      }
      menu_left_size++;                                            // expand left menu for item 
      for (int i=menu_left_size; i<wantedpos; i--) {
        menu_left[i+1] = menu_left[i];                             // set all options after transfer option one down
      }    
      menu_left[wantedpos] = menu_right[menu_current];             // transfer option from right to left at original position
      
      menu_right[menu_current] = "";                               // remove option from right menu und shrink menu
      menu_right_size--;
      delay(100);
    }
       
    menu_redraw_required = 1;
    break;  
  }
}


void setup() {
  
  u8g.setRot180();                        // rotate screen, if required
  Serial.begin(9600);
   
  pinMode(PIN_SELECT, INPUT_PULLUP);      // select key from rotary encoder
  pinMode(PIN_SWITCH, INPUT_PULLUP);      // switch key push button
  pinMode(PIN_LED, OUTPUT);  

  for(int i = 0; i < MENU_ITEMS; i++) menu_left[i] = menu_left_default[i];
      
  menu_redraw_required = 1;     // force initial redraw
}


void loop() {  

  uiStep();                                     // check for key press
    
  if (  menu_redraw_required != 0 ) {
    u8g.firstPage();
    do  {
      drawMenu();
    } while( u8g.nextPage() );
    menu_redraw_required = 0;
  }

  updateMenu();                            // update menu bar
  
}
