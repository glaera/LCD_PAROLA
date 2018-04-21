#include <MD_Parola.h>              //MD Parola Library
#include <MD_MAX72xx.h>             //MD Maxx72XX Library
#include <SPI.h>                    //SPI library for comms

#define MAX_DEVICES 4
#define CLK_PIN 13
#define DATA_PIN 11
#define CS_PIN 10
//MD_Parola P = MD_Parola(CS_PIN, MAX_DEVICES);
MD_Parola P = MD_Parola(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

int WAIT_TIME=1000;

int scrollSpeed;    // used to set text scroll speed in Parola at start

// sets scrolling direction if slider in middle at start
textEffect_t scrollEffect = PA_SCROLL_RIGHT;

textPosition_t scrollAlign = PA_RIGHT;  // how to aligh the text

int scrollPause = 0; // ms of pause after finished displaying message

#define  BUF_SIZE  75  // Maximum of 75 characters
char curMessage[BUF_SIZE] = { "greta is the best" };  // used to hold current message
#define slider_pin A0  // OTA or OTB pin on slider
int slider_val;  // used to hold the slider analog value
int slide_scroll_speed;   // used when changing scroll speed



void setup()
{

 // pinMode(slider_pin, INPUT);
  
  P.begin();  // Start Parola
  
  // configure Parola
  P.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);

}

void loop(void)
{
slider_val = analogRead(A0);  // check slider analog value
  
  if (slider_val > 600) {  // if slider to the left
    
    scrollEffect = PA_SCROLL_LEFT;  // Change direction to Left
    
    P.setTextEffect(scrollEffect, scrollEffect); // set new direction in Parola (OLD, NEW)
    
    slide_scroll_speed = map(slider_val, 1023, 0, 15, 400); // map slider value to text scroll speed
    
    P.setSpeed(slide_scroll_speed);  // Set new text scroll speed
    
  }
  
  
  if (slider_val < 350) {  // if slider to the right
    
    scrollEffect = PA_SCROLL_RIGHT;
    
    P.setTextEffect(scrollEffect, scrollEffect);  
    
    slide_scroll_speed = map(slider_val, 1023, 0, 400, 15);
    
    P.setSpeed(slide_scroll_speed);
  }
  

  if (slider_val < 600 && slider_val > 350) {  // if slider in middle
    
    slide_scroll_speed = 500;
    
    P.setSpeed(slide_scroll_speed);
  }


  if (P.displayAnimate()) // If finished displaying message
  {
    P.displayReset();  // Reset and display it again
  }
}



