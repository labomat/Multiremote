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

//#include "U8glib.h"
#include <U8g2lib.h>
#include <Rotary.h>
#include <Bounce2.h>
#include <Wire.h>

// states and messages
#define MENU_NONE 0
#define MENU_PREV 1
#define MENU_NEXT 2
#define MENU_SELECT 3
#define MENU_LEFT_ACTIVE 4
#define MENU_RIGHT_ACTIVE 5

#define PIN_LED 9     // Activity LED
#define PIN_SWITCH 8  // Switch Menu
#define PIN_SELECT 4  // Select button on rotary encoder

//U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);  // define Display for u8G
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

Rotary r = Rotary(2, 3);                     // define rotary encoder at pin 2 und 3 (push button at pin 4)

// Instantiate a Bounce object
Bounce menuSwitch = Bounce(); 
Bounce menuSelect = Bounce(); 

#define MENU_ITEMS 5
const char *menu_left_default[MENU_ITEMS] = { "Fahrt", "Anker", "Taucher", "Schlepp", "UW Arbeit" };

char *menu_left[MENU_ITEMS+1] = { "", "", "", "", "", "" };
char *menu_right[MENU_ITEMS+1] = { "", "", "", "", "", "" };

uint8_t menuCodeFirst = MENU_NONE;  // menu code states
uint8_t menuCodeSecond = MENU_NONE; //
uint8_t menuCode = MENU_NONE;       //

uint8_t menu_current = 0;                 // current menu item
uint8_t menu_active = MENU_LEFT_ACTIVE;   // wich menu is active (left or right)

int switchCounter = 0;              // counter for the number of button presses
int switchState = 0;                // current state of the button
int lastSwitchState = 0;            // previous state of the button
int wantedpos = -1;

uint8_t menu_redraw_required = 0;
uint8_t last_code = MENU_NONE;

uint8_t menu_right_size = 0;
uint8_t menu_left_size = MENU_ITEMS;


void uiStep(void) {

  unsigned char result = r.process();
//  if ( result != 0 ) {
//    Serial.println(char(result));
//  }
  menuSelect.update();
  int selectState = menuSelect.read();
  
  if (result == DIR_CW) {                // rotary encoder dial
    menuCodeFirst = MENU_NEXT;
  }
  else if (result == DIR_CCW) {
    menuCodeFirst = MENU_PREV;
  }
  else if ( selectState == LOW ){       // rotary encoder switch
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

// switch from left to right menu by button push
  // to switch between two states button push is counted and divided by two
  // even numbers define state MENU_LEFT_ACTIVE, uneven numbers MENU_RIGHT_ACTIVE

  menuSwitch.update();
  switchState = menuSwitch.read();

  if (switchState != lastSwitchState) {
    if (switchState == LOW) {
      switchCounter++;
      //menu_current = 0;
    } 
    else {
    }
  }
  lastSwitchState = switchState;
  
  if (switchCounter % 2 == 0) {     
    menu_active = MENU_LEFT_ACTIVE;
    digitalWrite(PIN_LED, HIGH);
  } else {
    menu_active = MENU_RIGHT_ACTIVE;
    digitalWrite(PIN_LED, LOW);
  }
  
  //menu_redraw_required = 1;

}


void drawMenu(void) {
  uint8_t i, h;
  u8g2_uint_t w, d, d_right;
  
  h = u8g2.getFontAscent()-u8g2.getFontDescent();       // height of single line
  w = (u8g2.getWidth()/2)-2;                           // width of menu (half of the screen minus 2 pixels)
  d = 7;                                              // left indent (text starts at d pixel )
  d_right = (u8g2.getWidth()/2)+2+d;                   // left indent for right menu

  u8g2.drawVLine(u8g2.getWidth()/2, 0, u8g2.getHeight()); // draw separator line
  
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.setFontRefHeightText();
  u8g2.setFontPosTop();
  
   // left menu    
   for( i = 0; i < menu_left_size; i++ ) {
   
    u8g2.setDrawColor(1);
    
    if ( i == menu_current && menu_active == MENU_LEFT_ACTIVE) {   // if left menu active indicate menu position
        u8g2.setFont(u8g2_font_6x12_tr);                           // set font 
        u8g2.drawStr(0, (i)*h, ">");                              // draw triangle

        u8g2.setFontRefHeightText();
        u8g2.setFontPosTop();
    }
    else {                                                        // else just draw menu
        u8g2.setFont(u8g2_font_6x12_tr);
        u8g2.setFontRefHeightText();
        u8g2.setFontPosTop();
    }
//    Serial.print(i);
//    Serial.print(" | ");
//    Serial.print(menu_current);
//    Serial.print(" | ");
//    Serial.println( menu_left[i]);
    u8g2.drawStr(d, i*h, menu_left[i]);
  }

    // right menu
    for( i = 0; i < menu_right_size; i++ ) {
   
    u8g2.setDrawColor(1);
    
    if ( i == menu_current && menu_active == MENU_RIGHT_ACTIVE){
        u8g2.setFont(u8g2_font_6x12_tr);
        u8g2.drawStr(u8g2.getWidth()/2+2, (i)*h, ">");
        u8g2.setFontRefHeightText();
        u8g2.setFontPosTop();
    }
    else {
        u8g2.setFont(u8g2_font_6x12_tr);
        u8g2.setFontRefHeightText();
        u8g2.setFontPosTop();
    }
    u8g2.drawStr(d_right, i*h, menu_right[i]);
  }
}

void updateMenu(void) {
  if ( menuCode != MENU_NONE && last_code == menuCode ) {
    return;
  }
  last_code = menuCode;

  // menu selection
  
  switch ( menuCode ) {
    
    case MENU_NEXT:
      menu_current++;
      if ( ( menu_current >= MENU_ITEMS && menu_active == MENU_LEFT_ACTIVE ) || ( menu_current >= menu_right_size && menu_active == MENU_RIGHT_ACTIVE ) ) {
        menu_current = 0;
      }
      menu_redraw_required = 1;
    break;
      
    case MENU_PREV:
      if ( menu_current == 0 ) {
        if ( menu_active == MENU_LEFT_ACTIVE ) {
          menu_current = MENU_ITEMS-1;
        }
        else { 
          menu_current = menu_right_size; 
        }
      }
      else menu_current--;
      menu_redraw_required = 1;
    break;
      
    case MENU_SELECT:

    if ( menu_active == MENU_LEFT_ACTIVE){                    // click when left menu (available options) is active

      menu_right[menu_right_size] = menu_left[menu_current];  // current item from left -> right     
      menu_right_size++;                                      // increase right menu size
      
      for (int j=menu_current; j<menu_left_size; j++) {       // move all items after transfered item one position up
        menu_left[j] = menu_left[j+1];
      }
      menu_left_size--;                                       // decrease left menu size
      if (menu_left_size <= 0) menu_left_size = 0;

      Serial.print("1: Sl ");
      Serial.print(menu_left_size);
      Serial.print(" |Sr ");
      Serial.print(menu_right_size);
      Serial.print(" |Mc ");
      Serial.print(menu_current);
      Serial.print(" |Ml ");
      for (int p = 0; p < menu_left_size; p++) {
        Serial.print(menu_left[p]);
        Serial.print(" ");                          
      }
      Serial.println("|Mr ");
      for (int p = 0; p < menu_right_size; p++) {
        Serial.print(menu_right[p]);
        Serial.print(" ");                          
      }
      Serial.println("| ");
      Serial.println(" ");
      
    }
    
    else if ( menu_active == MENU_RIGHT_ACTIVE){                  // click when right menu (selected options) is active
                                                                  // on what original position in left menu was selected item?
      for (int i=0; i<MENU_ITEMS; i++) {
         if (menu_left_default[i] == menu_right[menu_current]) {  // original positions are in default options array
          wantedpos = i;   
          break;
         }
      }

      menu_left_size++;                                            // expand left menu for item 
      
      if ( wantedpos > menu_left_size - 1 ) {
        menu_left[menu_left_size] = menu_right[menu_current];
      }
      else {
        for (int i = menu_left_size; i > wantedpos; i--) {
          menu_left[i] = menu_left[i-1];                             // set all options after transfer option one down
        } 
        menu_left[wantedpos] = menu_right[menu_current];             // transfer option from right to left at original position
      }
      
//      menu_left_size++;                                            // expand left menu for item 
//      for (int i = menu_left_size; i > wantedpos; i--) {
//        menu_left[i] = menu_left[i-1];                             // set all options after transfer option one down
//      }  
      
//      menu_left[wantedpos] = menu_right[menu_current];             // transfer option from right to left at original position

      for (int i = menu_current; i < menu_right_size; i++) {
        menu_right[i] = menu_right[i+1];                             // set all options after transfer option one down
      } 
     
      //menu_right[menu_current] = "";                               // remove option from right menu und shrink menu
      menu_right_size--;
      if (menu_right_size <= 0) menu_right_size = 0;   

      Serial.print("2: Sl ");
      Serial.print(menu_left_size);
      Serial.print(" |Sr ");
      Serial.print(menu_right_size);
      Serial.print(" |wP ");
      Serial.print(wantedpos);
      Serial.print(" |Ml(wP) ");
      Serial.print(menu_left[wantedpos]);
      Serial.print(" |Ml ");
      for (int p = 0; p < menu_left_size; p++) {
        Serial.print(menu_left[p]);
        Serial.print(" ");                          
      }
      Serial.println("|Mr ");
      for (int p = 0; p < menu_right_size; p++) {
        Serial.print(menu_right[p]);
        Serial.print(" ");                          
      }
      Serial.println(" "); 

    }
       
    menu_redraw_required = 1;
    break;  
  }
}


void setup() {
  
  //u8g.setRot180();                        // not used with u8g2 rotate screen, if required
  Serial.begin(9600);

  u8g2.begin();
   
  pinMode(PIN_LED, OUTPUT);  
  pinMode(PIN_SELECT, INPUT_PULLUP);      // select key from rotary encoder
  pinMode(PIN_SWITCH, INPUT_PULLUP);      // switch key push button

  menuSelect.attach(PIN_SELECT);          // debounced push buttons
  menuSelect.interval(5); 
  menuSwitch.attach(PIN_SWITCH);
  menuSwitch.interval(5);   

  for(int i = 0; i < MENU_ITEMS; i++) menu_left[i] = menu_left_default[i]; // copy content from default options to options
      
  menu_redraw_required = 1;     // force initial redraw
}


void loop() {  

  uiStep();                                     // check for key press
    
  if (  menu_redraw_required != 0 ) {
    u8g2.firstPage();
    do  {
      drawMenu();
    } while( u8g2.nextPage() );
    menu_redraw_required = 0;
  }

  updateMenu();                            // update menu bar

//  Serial.print(menu_left[menu_current]);
//  Serial.print(" | ");
//  Serial.print(menu_right[menu_current]);
//  Serial.print(" |Mc: ");
//  Serial.print(menu_current);
//  Serial.print(" |Mls: ");
//  Serial.print(menu_left_size);
//  Serial.print(" |Mrs: ");
//  Serial.print(menu_right_size);
//  Serial.print(" |Ma: ");
//  Serial.print(menu_active);
//  Serial.print(" | ");
//  Serial.println(menuCode);

  
}
