#include <Arduino.h>
#include "debug.h"
#include <YoutubeApi.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <WiFiClientSecure.h>
#include "ntp.h"            // Драйвер NTP (время из интернета)
//#include <ArduinoJson.h>

//#include <Adafruit_GFX.h>
//#include <Adafruit_ILI9341.h>


// Additional UI functions
#include "GfxUi.h"

#include "Orbitron_Light_8.h"
#include "Orbitron_Light_16.h"
#include "Orbitron_Light_26.h"
#include "Orbitron_Light_50.h"
#include "icons.h"

// ILI9341
#define TFT_DC      (5)  // GPIO5 (D1)
#define TFT_CS      (15) // GPIO15 (D8)
#define TFT_RESET   (4) // GPIO4 (D2)
// SPI:
// SCK to 14 (D5)
// MISO to 12 (D6)
// MOSI to 13 (D7)

//#define RELEASE   // Раскоментировать перед выпуском релиза. Это отключит вывод в серийный порт!

//------- Replace the following! ------
char ssid[] = "Bikar6";             // your network SSID (name)
char password[] = "htdjkzwbz1917";  // your network key (password)

//#define API_KEY "AIzaSyDkCSK1AcpaP8lz1v2OFVQewtgTnOvJdy8"   // your google apps API Token
#define API_KEY "AIzaSyDkLWzSWNyD0EWl6iijDAwcYpl1UPi3jzU"   // your google apps API Token
#define CHANNEL_ID "UCXPocNzr-6BsPSc11lzgoCA"               // makes up the url of channel
char CH_NAME[] = "LMS-tech.ru";
//--------------------------------------

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RESET);
GfxUi ui = GfxUi(&tft);

WiFiClientSecure client;
YoutubeApi api(API_KEY, client);
//YoutubeApi *api;

#define API_REQUEST_FIRST 6000
#define API_REQUEST_CONTINUE 300000
int api_mtbs = API_REQUEST_FIRST; //mean time between api requests
unsigned long api_lasttime;   //last time api request has been done
unsigned long cnt_lasttime;   //last time print has been done

unsigned long subs;
unsigned long views;
bool ytc_setup = false;

long view_stat[60];
long subs_stat[60];

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // Init ILI9341
  tft.begin();
  delay(1);
  //tft.setRotation(1);

  tft.fillScreen(ILI9341_RED);
  tft.drawBitmap(70, 50, wifiBitmap, 100, 70, ILI9341_WHITE);
  tft.setCursor(38, 150);
  tft.setFont(&Orbitron_Light_16);
  tft.print("Youtube Channel");
  tft.setCursor(77, 190);
  tft.print("Statistics");

  tft.setFont(&Orbitron_Light_26);
  //tft.setCursor(72, 313);
  //tft.print("coded by Shin2");
  ui.setTextAlignment(CENTER);
  ui.drawString(120, 30, CH_NAME);
  //tft.setCursor(120, 15);
  //tft.print(CH_NAME);

  // disable sleep mode for better data rate
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  delay(3200);
  tft.setFont(&Orbitron_Light_16);
  //ui.setTextAlignment(CENTER);
  char* MY_IP = "IP address:";
  ui.drawString(120, 224, MY_IP);
  ui.drawString(120, 246, ipToString(WiFi.localIP()));

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  get_ip = true;
  //api = new YoutubeApi(API_KEY, client);
  Serial.println("Init YouTube API");
  client.setInsecure();
}

void draw_graf() {
  uint8_t i;
  long min_subs = subs;
  long min_view = views;
  long max_subs = subs;
  long max_view = views;
  if (!ytc_setup) {
    for (i = 0 ; i < 60 ; i++) {
      view_stat[i] = views - (60 - i)/5;
      subs_stat[i] = subs;
    }
    min_view = views - 12;
  } else {
    for (i = 0 ; i < 59 ; i++) {
      view_stat[i] = view_stat[i+1];
      subs_stat[i] = subs_stat[i+1];
      if (view_stat[i] < min_view)
        min_view = view_stat[i];
      if (subs_stat[i] < min_subs)
        min_subs = subs_stat[i];
      if (view_stat[i] > max_view)
        max_view = view_stat[i];
      if (subs_stat[i] > max_subs)
        max_subs = subs_stat[i];
      view_stat[59] = views;
      subs_stat[59] = subs;
    }
  }
  for (i = 0 ; i < 60 ; i++) {
    tft.fillRect(4 * i, 271, 4, map(view_stat[i], min_view, max_view, -1, -75), ILI9341_GREEN);
    tft.drawFastHLine(4*i, 271 - 5 * (subs_stat[i] - min_subs), 4, ILI9341_RED);
  }
  
}

void statYT() {
  if(api.getChannelStatistics(CHANNEL_ID)) {
    if (!ytc_setup) {
      tft.fillScreen(ILI9341_BLACK);
      tft.setCursor(16, 40);
      tft.setFont(&Orbitron_Light_16);
      tft.println("Subscribers");
      tft.setCursor(206, 186);
      //tft.setFont(&Orbitron_Light_8);
      tft.println("5h");
      tft.drawFastHLine(175, 180, 24, ILI9341_WHITE);
      tft.drawLine(175, 180, 185, 185, ILI9341_WHITE);
      tft.drawLine(175, 180, 185, 175, ILI9341_WHITE);
    } else {
      tft.fillRect(0, 43, 240, 38, ILI9341_BLACK);
    }
    tft.setCursor(16, 82);
    tft.setFont(&Orbitron_Light_50);
    subs = long(api.channelStats.subscriberCount);
    if (subs > 99999) {
      tft.setCursor(16, 65);
      tft.setFont(&Orbitron_Light_26);
    }
    tft.println(String(subs));
    
    //tft.drawFastHLine(0, 95, 240, ILI9341_WHITE);

    if (!ytc_setup) {
      tft.setCursor(16, 120);
      tft.setFont(&Orbitron_Light_16);
      tft.println("Total Views");
    } else {
      tft.fillRect(0, 125, 240, 21, ILI9341_BLACK);
    }
    tft.setCursor(16, 145);
    tft.setFont(&Orbitron_Light_26);
    views = long(api.channelStats.viewCount);
    if (!ytc_setup)
      tft.println(String(views));
    else {
      if (views > 19999999) {
        tft.setCursor(16, 139);
        tft.setFont(&Orbitron_Light_16);
        tft.println(String(views) + "  ( " + String(views - view_stat[0]) + " )");
      } else if (views > 199999) {
        tft.print(String(views));
        tft.setFont(&Orbitron_Light_16);
        tft.println("  ( " + String(views - view_stat[0]) + " )");
      } else {
        tft.println(String(views) + " (" + String(views - view_stat[0]) + ")");
      }
    }
    
    //tft.drawFastHLine(0, 160, 240, ILI9341_WHITE);

    if (!ytc_setup) {
      tft.setCursor(16, 190);
      tft.setFont(&Orbitron_Light_16);
      tft.println("Total Videos");
    } else {
      //tft.fillRect(0, 195, 240, 21, ILI9341_BLACK);
      tft.fillRect(0, 195, 240, 76, ILI9341_BLACK);
    }
    draw_graf();
    tft.setCursor(16, 215);
    tft.setFont(&Orbitron_Light_26);
    tft.println(String(api.channelStats.videoCount));

    /*tft.setCursor(20, 260);
    tft.setFont(&Orbitron_Light_16);
    tft.println("Comment Count");
    tft.setCursor(20, 285);
    tft.setFont(&Orbitron_Light_26);
    tft.println(String(api->channelStats.commentCount));*/
    
    Serial.println("---------Stats---------");
    Serial.print("Subscriber Count: ");
    Serial.println(api.channelStats.subscriberCount);
    
    Serial.print("View Count: ");
    Serial.println(api.channelStats.viewCount);
    Serial.print("Comment Count: ");
    Serial.println(api.channelStats.commentCount);
    //tft.println(api.channelStats.commentCount);
    Serial.print("Video Count: ");
    Serial.println(api.channelStats.videoCount);
    
    // Probably not needed :)
    //Serial.print("hiddenSubscriberCount: ");
    //Serial.println(api->channelStats.hiddenSubscriberCount);
    Serial.println("------------------------");
    api_mtbs = API_REQUEST_CONTINUE;
    ytc_setup = true;
  } else {
    Serial.println("getChannelStatistics Error");
    api_mtbs = API_REQUEST_FIRST;
  }
  api_lasttime = millis();
}


void showTime() {
  tft.fillRect(0, 272, 240, 40, ILI9341_BLUE);
  if (millis() < 10000000) {
    tft.setCursor(20, 310);
    tft.setFont(&Orbitron_Light_50);
  } else { 
    tft.setCursor(50, 298);
    tft.setFont(&Orbitron_Light_26);
  }
  tft.println(String(millis()/1000) + " s");
}

void prnTime() {
  if (minute()+second() == 0) time_start = false;
  if (time_start)
    tft.fillRect(25, 276, 190, 22, ILI9341_BLUE);
  else {
    tft.fillRect(0, 272, 240, 47, ILI9341_BLUE);
    tft.setFont(&Orbitron_Light_16);
    ui.drawString(120, 314, dateString());
    time_start = true;
  }
  tft.setFont(&Orbitron_Light_26);
  ui.setTextAlignment(CENTER);
  ui.drawString(120, 297, timeString());
}

void loop() {
  if (millis() > api_lasttime + api_mtbs) {
    statYT();  
    Serial.println("Request to API");
  }
  if (millis() > ntp_lasttime + ntp_timer) {
    ntp(25, "ru.pool.ntp.org", 2); // время NTP 
    ntp_lasttime = millis();
    Serial.println("Time request");
  }
  if (millis() > cnt_lasttime + 1000 && ytc_setup) {
    prnTime();
    cnt_lasttime = millis();
    //Serial.println(timeString());
	//ntp(25, param("ntp_srv"), param("tz").toInt()); // время NTP
	//max_display(dateString(), true, 2); // дата
    //max_display(timeString(), false, 3); // время

  }
}
