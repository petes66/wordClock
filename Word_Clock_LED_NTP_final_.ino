#include <time.h>
#include <MD_MAX72xx.h>
#include <WiFi.h>
#include <string.h>
/*
 * Written by Arthur Apicella 3/3/2020
 * aapicella@gmail.com
 *
 */
// Network stuff
//Put in your home network here
const char *ssid = "XXXXX";
const char *password = "XXXXX";
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

/* LED Martix Stuff
 *  ESP32 --> LED MATRIX
 *  5v        VCC
 *  GND       GND
 *  G23       DIN
 *  G18       CLK
 *  G15       CS
 */
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define  MAX_DEVICES 4
#define  CLK_PIN   18 // or SCK
#define DATA_PIN  23 // or MOSI
#define CS_PIN    5 // or SS

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
/*
 * Define array of numbers were numbers[1] == "One"
 * and on and on...
 */
const char *numbers[] = {
    "0",           "One",        "Two",          "Three",        "Four",
    "Five",        "Six",        "Seven",        "Eight",        "Nine",
    "Ten",         "Eleven",     "Twelve",       "Thirteen",     "Fourteen",
    "Quarter",     "Sixteen",    "Seventeen",    "Eighteen",     "Nineteen",
    "Twenty",      "Twenty-One", "Twenty-Two",   "Twenty-Three", "Twenty-Four",
    "Twenty-Five", "Twenty-Six", "Twenty-Seven", "Twenty-Eight", "Twenty-Nine",
    "Half Past"};

int x = 0, y = 0; // start top left
String theTime;
String clockTime="hh:mm";
/* 
 *  Since time was set in esp32 via NTP
 *  we can now just get time locally.
 *  We then conver time to string
 *  
 *  It's half past 6 o'Clock in the evening.
 */
void getLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
   theTime="It's ";
  int hour = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;

  int AM = true;
  if (timeinfo.tm_hour > 12) {
    hour = timeinfo.tm_hour - 12;
    AM = false;
  }
  clockTime=" ";
  if(hour <10){
    clockTime="0";
  }
  clockTime+=hour;
  clockTime+=":";
  if(minute < 10){
    clockTime+="0";
  }
  clockTime+=minute;
  //clockTime+=(AM)?" AM":" PM";
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  if (!AM) clockTime+=" .";
  Serial.print("It's ");
  if (minute == 15) {
    Serial.print("Quarter Past ");
    theTime+="Quarter Past ";
  } else if (minute == 30) {
    Serial.print("Half Past ");
    theTime+="Half Past ";
  } else if (minute == 45) {
    Serial.print("Quarter to ");
    theTime+="Quarter to ";
    hour++;
  } else if ((minute > 0) and (minute < 30)) {
    Serial.print(numbers[minute]);
    theTime+=numbers[minute];
    theTime+=" Minute Past ";
    Serial.print(" Minutes Past ");
  } else if (minute > 30) {
    Serial.print(numbers[60 - minute]);
    Serial.print(" Minutes To ");
    theTime+=numbers[60-minute];
    theTime+=" Minutes To ";
    hour++;
  }
  //Bug midnight was 00 not 12
  if ((hour == 12) and (minute == 0)) {
    Serial.print((AM) ? " Noon " : " Midnight ");
    theTime+=(AM) ? " Noon " : " Midnight ";

  } else {
    Serial.print(numbers[hour]);
    theTime+=numbers[hour];
     Serial.print(" O'Clock");
     theTime+=" O'Clock";
    
    /*
     * 12:00am= Midnight (00:00)
     * 12:00am- Dawn (Sunrise)= Early morning
     * Dawn-12:00pm (Noon)= Morning
     * 12:00pm (Noon)
     * Noon-6:00pm = Afternoon or prevening (If you saw the Big Bang Theory
     * Episode) 6:00pm- Sundown / Dusk= (Evening)
     */
    if ((timeinfo.tm_hour > 5) and (timeinfo.tm_hour < 12)) {
      Serial.print(" in the Morning");
      theTime+=" in the Morning";
        
    // Add in Prevening here
    // } else if ((timeinfo.tm_hour > 11) and (timeinfo.tm_hour < 16)){
    //  Serial.print(" in the Prevening");
    //  theTime+=" in the Prevening";

    } else if ((timeinfo.tm_hour > 11) and (timeinfo.tm_hour < 18)) {
      Serial.print(" in the Afternoon");
      theTime+=" in the Afternoon";
    } else if (timeinfo.tm_hour > 17) {
     theTime+=" in the Evening";
      Serial.print(" In the Evening");
    }
  }
  Serial.println("--");
  //Serial.println(theTime.c_str());
}

void setup() {
  Serial.begin(115200);
  // init the display
  mx.begin();

  // connect to WiFi Just long enough to get the current Time.
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");
  // init and get the time

  #define timezone -5 // US Eastern Time Zone

  configTime(timezone * 3600, 0, ntpServer);
  getLocalTime();
  // disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}


int len=0;
void loop() {
  getLocalTime();
  scrollText(theTime.c_str());
  printText(0, MAX_DEVICES-1, clockTime.c_str());
  delay(2000); 
}
const int DELAYTIME=50;

void scrollText(const char *p)
{
  uint8_t charWidth;
  uint8_t cBuf[8];  // this should be ok for all built-in fonts

  Serial.println("Scrolling text");
  mx.clear();

  while (*p != '\0')
  {
    charWidth = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);

    for (uint8_t i=0; i<=charWidth; i++)  // allow space between characters
    {
      mx.transform(MD_MAX72XX::TSL);
      if (i < charWidth)
        mx.setColumn(0, cBuf[i]);
      delay(DELAYTIME);
    }
  }
    for (uint8_t i=0; i<ROW_SIZE; i++)
  {
    mx.transform(MD_MAX72XX::TSU);
    delay(DELAYTIME*2);
  }
}
void printText(uint8_t modStart, uint8_t modEnd, const char *pMsg)
{
  uint8_t   state = 0;
  uint8_t   curLen;
  uint16_t  showLen;
  uint8_t   cBuf[8];
  int16_t   col = ((modEnd + 1) * COL_SIZE) - 1;

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  do     // finite state machine to print the characters in the space available
  {
    switch(state)
    {
      case 0: // Load the next character from the font table
        // if we reached end of message, reset the message pointer
        if (*pMsg == '\0')
        {
          showLen = col - (modEnd * COL_SIZE);  // padding characters
          state = 2;
          break;
        }

        // retrieve the next character form the font file
        showLen = mx.getChar(*pMsg++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state++;
        // !! deliberately fall through to next state to start displaying

      case 1: // display the next part of the character
        mx.setColumn(col--, cBuf[curLen++]);

        // done with font character, now display the space between chars
        if (curLen == showLen)
        {
          showLen = 1;
          state = 2;
        }
        break;

      case 2: // initialize state for displaying empty columns
        curLen = 0;
        state++;
        // fall through

      case 3:  // display inter-character spacing or end of message padding (blank columns)
        mx.setColumn(col--, 0);
        curLen++;
        if (curLen == showLen)
          state = 0;
        break;

      default:
        col = -1;   // this definitely ends the do loop
    }
  } while (col >= (modStart * COL_SIZE));

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}
