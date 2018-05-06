#include <MD_Parola.h>              //MD Parola Library
#include <MD_MAX72xx.h>             //MD Maxx72XX Library
#include <SPI.h>                    //SPI library for comms
#include <ESP8266WiFi.h>
#include <WifiUDP.h>
#include <String.h>
#include <Wire.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h" 
#include "pitches.h"  //add note library

//notes in the melody
//int melody[]={NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
int melody[]={NOTE_D2, NOTE_A2, NOTE_G2, NOTE_G2, NOTE_D2, 0, NOTE_D2,NOTE_A2, NOTE_G2, NOTE_G2};
//note durations. 4=quarter note / 8=eighth note
//int noteDurations[]={4, 8, 8, 4, 4, 4, 4, 4};
int noteDurations[]={8, 4, 8, 4, 4, 4, 8, 4, 8, 4};

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "xxx"
#define AIO_KEY         "xxx"

#define MAX_DEVICES 8
//ESP8266
#define CLK_PIN D3 
#define DATA_PIN D1
#define CS_PIN D2 

#define SensorPin D5
#define LedPin D4



WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);


// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe GRETA_ALARM_TRIGGER = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/gretaalarm");


// Define NTP properties
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "ca.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)

// Set up the NTP UDP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

String date;
String t;
const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ;

// arduino mega
// #define CLK_PIN 13
// #define DATA_PIN 11
// #define CS_PIN 10

#define BUZZER_PIN D6

//MD_Parola P = MD_Parola(CS_PIN, MAX_DEVICES);
MD_Parola P = MD_Parola(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

int WAIT_TIME=1000;

int scrollSpeed=35;    // used to set text scroll speed in Parola at start

// sets scrolling direction if slider in middle at start
textEffect_t scrollEffect = PA_SCROLL_LEFT; //parameters from Parola library, sets scrolling from left
textPosition_t scrollAlign = PA_LEFT;

int scrollPause = 1000; // ms of pause after finished displaying message

#define  BUF_SIZE  75  // Maximum of 75 characters
char curMessage[BUF_SIZE] = { "greta is the best" };  // used to hold current message


#define slider_pin A0  // OTA or OTB pin on slider
int slider_val;  // used to hold the slider analog value
int slide_scroll_speed;   // used when changing scroll speed

#define WLAN_SSID       "xxx"
#define WLAN_PASS       "xxx"

void LanConnect() {
    // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
}

void setup()
{

    Serial.begin(9600);  
    timeClient.begin();   // Start the NTP UDP client

    LanConnect();

    pinMode(SensorPin, INPUT);
    pinMode(LedPin,OUTPUT);
    
  
   P.begin();                                //MD Parola parameter - begin
   P.displayClear();                         //MD Parola parameter - clear the display
   P.displaySuspend(false);                  //MD Parola parameter - suspend or not?

   // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&GRETA_ALARM_TRIGGER);

}

void updateTime() 
{
  if (WiFi.status() == WL_CONNECTED) //Check WiFi connection status
  {   
    date = "";  // clear the variables
    t = "";
    
    // update the NTP client and get the UNIX UTC timestamp 
    timeClient.update();
    unsigned long epochTime =  timeClient.getEpochTime();

    // convert received time stamp to time_t object
    time_t local, utc;
    utc = epochTime;

    // Then convert the UTC UNIX timestamp to local time
    TimeChangeRule usEDT = {"CEST", Second, Sun, Mar, 2, 60};  //UTC - 5 hours - change this as needed
    TimeChangeRule usEST = {"CEST", First, Sun, Nov, 2, 60};   //UTC - 6 hours - change this as needed
    Timezone usEastern(usEDT, usEST);
    local = usEastern.toLocal(utc);

    // now format the Time variables into strings with proper names for month, day etc
    date += days[weekday(local)-1];
    date += ", ";
    date += months[month(local)-1];
    date += " ";
    date += day(local);
    date += ", ";
    date += year(local);

    // format the time to 12-hour format with AM/PM and no seconds
    t += hourFormat12(local);
    t += ":";
    if(minute(local) < 10)  // add a zero if minute is under 10
      t += "0";
    t += minute(local);
    t += " ";
    t += ampm[isPM(local)];

    // Display the date and time
  //  Serial.println("");
  //  Serial.print("Local date: ");
  //  Serial.print(date);
  //  Serial.println("");
  //  Serial.print("Local time: ");
  //  Serial.print(t);
    
    String myMessage = t + " "+ date+" ";
    //char charBuf[myMessage.length() + 1];
    myMessage.toCharArray(curMessage, myMessage.length());
    
    //P.print(charBuf);
    //P.displayClear();                         //MD Parola parameter - clear the display
    //P.displaySuspend(false);                  //MD Parola parameter - suspend or not?

    //MD Paroloa display msg using our predefined parameters
    if (P.displayAnimate()) {
      P.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
    }
    delay(0);
  }
  else // attempt to connect to wifi again if disconnected
  {
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    Serial.println("Connected.");
  }
    
}

void loop(void)
{

  MQTT_connect();
  readMQTTMessages();
  int sensorValue = digitalRead(SensorPin);
  bool detectedMovement = sensorValue == HIGH;
  if (detectedMovement) {
    updateTime();
    //playMelody();
    digitalWrite(LedPin, HIGH);
 //   Serial.println("Movement detected");
  } else {
    digitalWrite(LedPin, LOW);
  //  Serial.println("No movement detected");
  }

  if (detectedMovement && P.displayAnimate()) // If finished displaying message
  {
  } else if (!detectedMovement) {
    // Serial.println("can you see?");
    P.displayClear();
    P.displayText("", scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
  }

 // delay(50);

}

void playMelody()
{
    Serial.println("play melody starts");

  //iterate over the notes of the melody
    for (int thisNote=0; thisNote < 10; thisNote++){

      //to calculate the note duration, take one second. Divided by the note type
      int noteDuration = 1000 / noteDurations [thisNote];
      tone(BUZZER_PIN, melody [thisNote], noteDuration);

      //to distinguish the notes, set a minimum time between them
      //the note's duration +30% seems to work well
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);

      //stop the tone playing
      noTone(BUZZER_PIN);
    }
    noTone(BUZZER_PIN);
    analogWrite(BUZZER_PIN, 0);


    Serial.println("play melody ends");

}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}


void readMQTTMessages() {
  
    Adafruit_MQTT_Subscribe *subscription;
    subscription = mqtt.readSubscription(1);
    if (subscription == &GRETA_ALARM_TRIGGER) {
        playMelody();
      }
}

