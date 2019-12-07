//bubbule clock display using 8bit valves
//MCP23017でIOを増設する　wireでI2C制御、8ポート増設（最大16ポートまで可能）
//esp8266 起動時にwifi接続できたときは、NTP時刻を取得しRTC時刻セット

int bubbleDelay = 15; // delay time in m sec to keep solenoid valves open to define the air bubble volume
int bubbleSeparateDealy = 1000; // delay time in m sec to define the vertical gap beteen air bubbles

int bubbleclockINTERVAL = 10000; // bubble clockの新規表示までの隙間を設定＝m sec
uint64_t bubbleClockTimestamp = 0;
#define heartbeatINTERVAL 1000 // oled clockの表示更新周期＝m sec
uint64_t heartbeatTimestamp = 0;
#define wifiINTERVAL 10000 // 10 sec 
uint64_t wifiIntervalTimestamp = 0;

#include <time.h>
#include <Wire.h>

// デバイスアドレス(スレーブ) MCP23017
uint8_t DEVICE_ADDRESS = 0x20;  // 2進 100000

#define JST     3600*9

//const int ledPWMPin = 13;  //バブル照明用LED D7 = GPIO 13
int ledMax = 255;  //neopixelバブル照明用LEDの点灯最高値
int ledMin = 10;  //neopixelバブル照明用LEDの点灯最小値

#include <U8g2lib.h>
//U8g2 Contructor
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 5, /* data=*/ 4);

u8g2_uint_t offset;     // current offset for the scrolling text
u8g2_uint_t width;      // pixel width of the scrolling text (must be lesser than 128 unless U8G2_16BIT is defined
const char *text = "12:08:08";


// A basic everyday NeoPixel strip test program.
// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN 15  //D8 = GPIO15

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 8

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

time_t t;
struct tm *tm;
static const char *wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};


#include <DS3231.h>
DS3231 Clock;
bool Century = false;
bool h12;
bool PM;
//byte ADay, AHour, AMinute, ASecond, ABits;
//bool ADy, A12h, Apm;

byte year, month, date, DoW, hour, minute, second;

/*
  https://www.envirodiy.org/ds3231-real-time-clock-rtc-date-conversion/
  Real Time Clock (RTC) modules like the DS3231 chip (found on the ChronoDot and Seeeduino Stalker v2.3) keep track of the date and time for a datalogger circuit.  The module can output the number of seconds since January 1, 2000 as an integer, which can be very handy for recording a time stamp with your sensor data.  However, 1/1/2000 is not a common starting point for most epoch calculations.  They usually use what most people call Unix Time, which is the number of seconds since 1/1/1970.   So there can be issues if you’re trying to convert between DS3231 epoch time and Unix Time.  The simplest method is to add 946684800 seconds (the number of seconds between 1/1/1970 and 1/1/2000) to the DS3231 time to get Unix Time.

  So:  DS3231Time + 946684800 = UnixTime
*/

//sin波形状表示用delay time
int sinDelay[7] = {1642, 1717, 2020, 4442, 4442, 2020, 1717};


#define fontNum 2 //バブル描画フォントを定義した数
#define fontVdigit 8 //バブル描画データ1文字の上昇方向のビット数
#define fontHdigit 8 //バブル描画データ1文字の水平方向のビット数
#define bubbleLetter 5 //バブル描画文字1セットの上昇方向文字数 例12:45
#define fontDataNum 11 //文字フォント毎のデータ数
uint8_t fontDataRev[fontNum][fontDataNum][fontVdigit];//左右反転後のフォントデータ
uint8_t fontData[fontNum][fontDataNum][fontVdigit] =  //オリジナルのフォントデータ
{
  { //fontNum 0
    { //1
      0b00000000,
      0b01101100,
      0b10010010,
      0b00000000,
      0b01000100,
      0b00101000,
      0b00000000,
      0b00010000
    }
    ,
    { //2
      0b10000000,
      0b01000000,
      0b00100000,
      0b00010000,
      0b00001000,
      0b00000100,
      0b00000010,
      0b00000001
    }
    ,
    { //3
      0b10010010,
      0b01001001,
      0b00100100,
      0b10010010,
      0b01001001,
      0b00100100,
      0b10010010,
      0b01001001
    }
    ,
    { //4
      0b10101010,
      0b01010101,
      0b10101010,
      0b01010101,
      0b10101010,
      0b01010101,
      0b10101010,
      0b01010101
    }
    ,
    { //5
      0b11001100,
      0b00110011,
      0b11001100,
      0b00110011,
      0b11001100,
      0b00110011,
      0b11001100,
      0b00110011
    }
    ,
    { //6
      0b11110000,
      0b00001111,
      0b11110000,
      0b00001111,
      0b11110000,
      0b00001111,
      0b11110000,
      0b00001111
    }
    ,
    { //7
      0b11111111,
      0b00000000,
      0b11111111,
      0b00000000,
      0b11111111,
      0b00000000,
      0b11111111,
      0b00000000
    }
    ,
    { //8
      0b10000000,
      0b00100000,
      0b00001000,
      0b00000010,
      0b00000100,
      0b00010000,
      0b01000000,
      0b10000000
    }
    ,
    { //9
      0b00000001,
      0b00000100,
      0b00010000,
      0b01000000,
      0b00100000,
      0b00001000,
      0b00000010,
      0b00000001
    }    ,
    { //10
      0b11111111,
      0b11111111,
      0b11111111,
      0b11111111,
      0b11111111,
      0b11111111,
      0b11111111,
      0b11111111
    }
    ,
    { //11
      0b00000000,
      0b01101100,
      0b10010010,
      0b00000000,
      0b01000100,
      0b00101000,
      0b00000000,
      0b00010000
    }
  }
  ,
  { //fontNum 1
    { //0
      0b00011000,
      0b00100100,
      0b01000010,
      0b00000000,
      0b01000010,
      0b00100100,
      0b00011000,
      0b00000000
    }
    ,
    { //1
      0b00001000,
      0b00010000,
      0b00001000,
      0b00010000,
      0b00001000,
      0b00010000,
      0b00000000,
      0b00111100
    }
    ,
    { //2
      0b00011000,
      0b00100100,
      0b00000000,
      0b00000100,
      0b00011000,
      0b00100000,
      0b00000000,
      0b01111100
    }
    ,
    { //3
      0b00011000,
      0b00100100,
      0b00000000,
      0b00001100,
      0b00000010,
      0b00000000,
      0b00100010,
      0b00011100
    }
    ,
    { //4
      0b00000000,
      0b00001100,
      0b00010000,
      0b00100100,
      0b00000000,
      0b01111110,
      0b00000000,
      0b00000100
    }
    ,
    { //5
      0b01111100,
      0b00000000,
      0b01000000,
      0b00111000,
      0b01000100,
      0b00000000,
      0b00000100,
      0b00111000
    }
    ,
    { //6
      0b00001000,
      0b00010000,
      0b00100000,
      0b00000000,
      0b01011100,
      0b00000000,
      0b01000010,
      0b00111100
    }
    ,
    { //7
      0b01111110,
      0b00000000,
      0b00000100,
      0b00001000,
      0b00010000,
      0b00100000,
      0b01000000,
      0b00000000
    }
    ,
    { //8
      0b00011000,
      0b00100100,
      0b00000000,
      0b00111100,
      0b01000010,
      0b00000000,
      0b01000010,
      0b00111100
    }
    ,
    { //9
      0b00111100,
      0b01000010,
      0b00000000,
      0b00111010,
      0b00000100,
      0b00001000,
      0b00010000,
      0b00100000
    }
    ,
    { //:
      0b00000000,
      0b00000000,
      0b00011000,
      0b00000000,
      0b00000000,
      0b00011000,
      0b00000000,
      0b00000000
    }
  }
};  //end of fontData


void OTA_setup() {
  /*
    Serial.begin(115200);
    delay(100);
    Serial.print("\n\nReset:\n");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(500);
    }
  */

  Serial.println();
  Serial.printf("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  u8g2.begin();

  u8g2.setFont(u8g2_font_logisoso20_tr);
  width = u8g2.getUTF8Width(text);    // calculate the pixel width of the text
  u8g2.setFontMode(0);    // enable transparent mode, which is faster

  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 0, 128, 32);
  u8g2.setDrawColor(1);

  Serial.begin(115200);

  // マスタとしてI2Cバスに接続する
  Wire.begin();

  // 初期設定(IODIRA/IODIRB)
  Wire.beginTransmission(DEVICE_ADDRESS);
  // 先頭アドレスへ
  Wire.write(0x00);
  // I/O設定Aの全てを出力設定にする
  Wire.write(0x00);
  // I/O設定Bの全てを出力設定にする
  Wire.write(0x00);
  Wire.endTransmission();

  //  ledFade(1);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  rainbow(1);
  strip.setBrightness(255); // Set BRIGHTNESS (max = 255)
  NeoPixelFade(1, 1, 1, 1); // R, G, B, 1=fade_in or -1=fade_out

  for (int i = 0; i < 10; i++)
  {
    purgeAir(bubbleDelay * 10);//空気追い出しパージ
    delay(bubbleSeparateDealy * 2);
  }
  delay(bubbleSeparateDealy);

  //データを左右反転
  for (int k = 0; k < fontNum; k++) {
    for (int j = 0; j < fontDataNum; j++) {
      for (int i = 0; i < fontVdigit; i++) {
        fontDataRev[k][j][i] = dataReverse(fontData[k][j][i]);
        // fontDataRev[k][j][i] = fontData[k][j][i];//反転しないでそのまま格納の場合
      }
    }
  }
  /*
    //データ左右反転後のプリンアウト
    for (int k = 0; j < fontNum; k++) {
    for (int j = 0; j < fontDataNum; j++) {
      for (int i = 0; i < fontVdigit; i++) {
        Serial.println("fontDataRev");
        Serial.print(j);
        Serial.print(" ");
        Serial.print(i);
        Serial.print(" ");
        Serial.println(fontDataRev[k][j][i]);
      }
    }
    }
  */

  //ledFade(-1);
  NeoPixelFade(1, 1, 1, -1); // R, G, B, 1=fade_in or -1=fade_out

    //set DS3231 time
  /*
          Clock.setSecond(00);//Set the second
          Clock.setMinute(18);//Set the minute
          Clock.setHour(18);  //Set the hour
          Clock.setDoW(0);    //Set the day of the week
          Clock.setDate(10);  //Set the date of the month
          Clock.setMonth(11);  //Set the month of the year
          Clock.setYear(19);  //Set the year (Last two digits of the year)

    ReadDS3231();
  */

}

void OTA_loop() {
  uint64_t now = millis();

  dispTimeInterval();

  //display bubble clock
  now = millis();
  if ((now - bubbleClockTimestamp) > bubbleclockINTERVAL) {
    NeoPixelFade(ledMax / 2, ledMax / 2, ledMax / 2, 1); // R, G, B, 1=fade_in or -1=fade_out
    dispBubbleCenter(4, 1); //センターラインを表示（表示バブル数、表示間隔短縮除数）
    dispBubbleSin(2, 4); //サイン波ラインを表示（表示周期数、周期短縮除数）
    dispBubbleSin(1, 8); //サイン波ラインを表示（表示周期数、周期短縮除数）
    delay((bubbleSeparateDealy + bubbleDelay) * (fontVdigit - 7));
    dispBubbleCenter(4, 1); //センターラインを表示（表示バブル数、表示間隔短縮除数）
    NeoPixelFade(ledMax / 2, ledMax / 2, ledMax / 2, -1); // R, G, B, 1=fade_in or -1=fade_out

    colorWipe(strip.Color(0, 0, ledMin), 1); //最小値青の初期値に戻す
    strip.show();  // Update strip with new contents

    delay((bubbleSeparateDealy + bubbleDelay) * (fontVdigit - 6));
    dispBubbleTime(); // the time

    delay((bubbleSeparateDealy + bubbleDelay) * (fontVdigit - 6));
    dispBubbleFontDemo(0);  // demo font0 : test pattern

    delay((bubbleSeparateDealy + bubbleDelay) * (fontVdigit - 6));
    dispBubbleFontDemo(1);  // demo font1 : number

    delay((bubbleSeparateDealy + bubbleDelay) * (fontVdigit - 6));
    bubbleClockTimestamp = now;
  }

}


//display oled clock
void dispTimeInterval() {
  uint64_t now = millis();
  if ((now - heartbeatTimestamp) > heartbeatINTERVAL) {

    //DS3231Time + 946684800 = UnixTime
    //ReadDS3231();
    t = time(NULL);
    tm = localtime(&t);
    Serial.printf(" %04d/%02d/%02d(%s) %02d:%02d:%02d\n",
                  tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                  wd[tm->tm_wday],
                  tm->tm_hour, tm->tm_min, tm->tm_sec);
    dispTime();
    dispAnalogueClock(128 - 16, 16, tm->tm_hour, tm->tm_min, tm->tm_sec);
    u8g2.sendBuffer();          // transfer internal memory to the display
    heartbeatTimestamp = now;
  }
}

//泡でfontIDのfont dataを順に表示
void dispBubbleFontDemo(uint8_t fontID) {

  for (int i = 0; i < 2; i++)
  {
    purgeAir(bubbleDelay * 10);//空気追い出しパージ
    delay(bubbleSeparateDealy * 2);
  }
  delay((bubbleSeparateDealy + bubbleDelay) * (fontVdigit - 4));
  for (int i = 0; i < fontDataNum; i++) {  // 正順表示
    //for (int i = fontDataNum - 1; i >= 0; i--) {  //逆順表示とする場合
    int rndR = random(100); //0から指示値-1の乱数をred値とする
    int rndG = random(100); //0から指示値-1の乱数をgreen値とする
    int rndB = random(100); //0から指示値-1の乱数をblue値とする
    NeoPixelFade(rndR, rndG, rndB, 1); // R, G, B, 1=fade_in or -1=fade_out
    dispBubbleFont(fontID, i);
    NeoPixelFade(rndR, rndG, rndB, -1); // R, G, B, 1=fade_in or -1=fade_out
  }
}


//泡で一文字表示
void dispBubbleFont(uint8_t fontID, uint8_t dataNum) {
  for (int i = 0; i < fontVdigit; i++) {
    Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x12);// GPIOA(GPA0～GPA7)
    Wire.write(fontDataRev[fontID][dataNum][i]);//データで1となっているバルブのみ開に
    Wire.endTransmission();
    delay(bubbleDelay);// バルブ開放時間

    Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x12);// GPIOA(GPA0～GPA7)
    Wire.write(0);//全て閉じる
    Wire.endTransmission();

    delay(bubbleSeparateDealy);// バブル間の区切りを作る時間
  }
  delay((fontVdigit - 7) * (bubbleDelay + bubbleSeparateDealy));
  dispTimeInterval();
}


//泡で時刻hh:mmを表示
void dispBubbleTime() {
  for (int i = 0; i < 2; i++)
  {
    purgeAir(bubbleDelay * 10);//空気追い出しパージ
    delay(bubbleSeparateDealy * 2);
  }
  delay((bubbleSeparateDealy + bubbleDelay) * (fontVdigit - 4));

  NeoPixelFade(0, 0, 255, 1);

  if ((tm->tm_hour) / 10 != 0) {
    dispBubbleFont(1, (tm->tm_hour) / 10);
    delay((bubbleSeparateDealy + bubbleDelay) * (fontVdigit - 7));
  }
  else {
    delay((bubbleDelay + bubbleSeparateDealy) * fontVdigit);
    //  dispBubbleFont(1, 0);// 一桁時刻において、10桁位置に"0"を表示をする場合は、上行のdelayをコメントアウトしてこちらを有効にする
  }
  dispBubbleFont(1, tm->tm_hour % 10);
  dispBubbleFont(1, 10);// ":"
  dispBubbleFont(1, tm->tm_min / 10);
  delay((bubbleSeparateDealy + bubbleDelay) * (fontVdigit - 7));
  dispBubbleFont(1, tm->tm_min % 10);

  NeoPixelFade(0, 0, 255, -1);
  delay((bubbleSeparateDealy + bubbleDelay) * (fontVdigit - 7));
}


//文字データのbit反転　bit0をbit7に、以降全bitを入れ替え
uint8_t dataReverse(uint8_t orgData) {
  uint8_t revData = orgData & 1;
  for (int i = 0; i < 7; i++) {
    revData = revData << 1;
    orgData = orgData >> 1;
    revData += orgData & 1;
  }
  return revData;
}


//全バルブのパイプ内のエアーを順にパージする
void purgeAir(int purgeTime) {
  Serial.print("purge air time= ");
  Serial.println(purgeTime);
  uint8_t data = 0b00000001; //test
  //rainbow(1);

  for (int i = 0; i < 8; i++) {
    strip.setPixelColor(i, strip.Color( ledMax / 2, ledMax / 2, ledMax / 2)); //  Set pixel's color (in RAM)
    strip.show();  // Update strip with new contents
    Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x12);// GPIOA(GPA0～GPA7)
    Wire.write(data);
    Wire.endTransmission();
    delay(purgeTime);// バルブ解放時間 ms
    Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x12);// GPIOA(GPA0～GPA7)
    Wire.write(0);//全て一旦閉じる
    Wire.endTransmission();
    data = data << 1;

    delay(bubbleSeparateDealy / 8);
  }

  colorWipe(strip.Color(0, 0, ledMin), 1);
  strip.show();  // Update strip with new contents
}


//センターライン表示　devider割した周期でcycle数分を表示
void dispBubbleCenter(uint8_t cycle, uint8_t devider) {
  uint8_t data[] = {
    0b00010000,
  };
  //rainbow(1);
  for (int i = 0; i < 1 * cycle; i++) {
    Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x12);// GPIOA(GPA0～GPA7)
    Wire.write(data[i]);
    Wire.endTransmission();
    delay(bubbleDelay);// バルブ開放時間
    Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x12);// GPIOA(GPA0～GPA7)
    Wire.write(0);//全て一旦閉じる
    Wire.endTransmission();
    delay(sinDelay[i % 7]  / devider);// バルブ閉時間 ms
  }
  delay(bubbleSeparateDealy);// バブル間の区切りを作る時間
}


//sin波表示　devider割した周期でcycle数分を表示
void dispBubbleSin(uint8_t cycle, uint8_t devider) {
  uint8_t data[] = {
    0b00010000,
    0b00001000,
    0b00000100,
    0b00000010,
    0b00000001,
    0b00000010,
    0b00000100,
    0b00001000,
    0b00010000,
    0b00100000,
    0b01000000,
    0b10000000,
    0b01000000,
    0b00100000
  };
  //rainbow(1);
  for (int i = 0; i < 14 * cycle; i++) {
    strip.setPixelColor((i % 8), strip.Color( ledMax / 2, ledMax / 2, ledMax / 2)); //  Set pixel's color (in RAM)
    strip.show();  // Update strip with new contents
    Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x12);// GPIOA(GPA0～GPA7)
    Wire.write(data[i % 14]);
    Wire.endTransmission();
    delay(bubbleDelay);// バルブ開放時間
    Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x12);// GPIOA(GPA0～GPA7)
    Wire.write(0);//全て一旦閉じる
    Wire.endTransmission();
    delay(sinDelay[i % 7]  / devider);// バルブ閉時間 ms
  }
}


//sin波を対称表示　devider割した周期でcycle数分を表示
void dispBubbleSinSymmetry(uint8_t cycle, uint8_t devider) {
  uint8_t data[] = {
    0b00000000,
    0b00011000,
    0b00100100,
    0b01000010,
    0b10000001,
    0b01000010,
    0b00100100
  };
  //rainbow(1);
  for (int i = 0; i < 14 * cycle; i++) {
    strip.setPixelColor((i % 8), strip.Color( ledMax / 2, ledMax / 2, ledMax / 2)); //  Set pixel's color (in RAM)
    strip.show();  // Update strip with new contents
    Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x12);// GPIOA(GPA0～GPA7)
    Wire.write(data[i % 7]);
    Wire.endTransmission();
    delay(bubbleDelay);// バルブ開放時間
    Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x12);// GPIOA(GPA0～GPA7)
    Wire.write(0);//全て一旦閉じる
    Wire.endTransmission();
    delay(sinDelay[i % 7] / devider);// バルブ閉時間 ms
  }

  colorWipe(strip.Color(0, 0, ledMin), 1);
  //strip.clear(); // Set all pixels in RAM to 0 (off)
  strip.show();  // Update strip with new contents
}



//oledに時刻をデジタル表示
void dispTime() {
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 0, 128, 32);
  u8g2.setDrawColor(1);

  // print the hour, minute and second:
  u8g2.setFont(u8g2_font_6x13_tf);
  //u8g2.setFont(u8g2_font_7x14_mf);
  //u8g2.setFont(u8g2_font_6x13B_tf);
  //  u8g2.setFont(u8g_font_helvB12r);
  //  u8g2.setCursor(0, 12);
  u8g2.setCursor(0, 9);
  u8g2.print(tm->tm_year + 1900);
  u8g2.print('/');
  u8g2.print(tm->tm_mon + 1);
  u8g2.print('/');
  u8g2.print(tm->tm_mday);
  u8g2.print('(');
  u8g2.print(wd[tm->tm_wday]);
  u8g2.print(')');

  //u8g2.setFont(u8g2_font_logisoso20_tr);
  u8g2.setFont(u8g2_font_fub20_tf);
  //  u8g2.setFont(u8g2_font_fub25_tf);
  u8g2.setCursor(0, 32);

  //  u8g2.setDrawColor(0);
  //  u8g2.drawBox(0, 32, width, 20);
  //  u8g2.setDrawColor(1);

  u8g2.setCursor(0, 32);
  u8g2.print(tm->tm_hour);//JST

  u8g2.print(':');
  if ((tm->tm_min) < 10) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    u8g2.print("0");
  }
  u8g2.print(tm->tm_min);

  //  u8g2.setFont(u8g2_font_6x13_tf);
  //  u8g2.print(':');
  //  if (tm->tm_sec < 10) {
  // In the first 10 seconds of each minute, we'll want a leading '0'
  //    u8g2.print("0");
  //  }
  //  u8g2.print(tm->tm_sec);
}


//oledに時刻をアナログ表示
void dispAnalogueClock(int offsetX, int offsetY, int draw_hour, int draw_min, int draw_sec) {
  float r_sec   = 14.0;
  float r_min   = 13.0;
  float r_hour  =  9.0;
  float r_clock = 15.0;
  float rad_sec  = ((float(draw_sec)  + 30.0) / 60.0) * 3.14 * (-2.0);
  float rad_min  = ((float(draw_min)  + (float(draw_sec) / 60.0 ) + 30.0) / 60.0) * 3.14 * (-2.0);
  float rad_hour = ((float(draw_hour) + (float(draw_min) / 60.0 ) +  6.0) / 12.0) * 3.14 * (-2.0);

  u8g2.setDrawColor(0);
  u8g2.drawBox(offsetX - r_clock - 1, offsetY - r_clock - 1, offsetX + r_clock + 1, offsetY + r_clock + 1);

  u8g2.setDrawColor(1);
  u8g2.drawCircle(offsetX, offsetY, r_clock, U8G2_DRAW_ALL);
  u8g2.drawBox(offsetX - 1, offsetY - r_clock - 1, 3, 3);
  u8g2.drawBox(offsetX - 1, offsetY - 1, 3, 3);
  u8g2.drawDisc(offsetX + int(r_clock * sin(rad_sec)), offsetY + int(r_clock * cos(rad_sec)), 2, U8G2_DRAW_ALL);
  u8g2.drawLine(offsetX, offsetY, offsetX + int(r_min * sin(rad_min)), offsetY + int(r_min * cos(rad_min)));
  u8g2.drawLine(offsetX, offsetY, offsetX + int(r_hour * sin(rad_hour)), offsetY + int(r_hour * cos(rad_hour)));
}

/*
  //ledをfade-in, fade-out表示
  void ledFade(int delta) //delta >=0ならLED fade-IN, delta <0ならLED fade-OUT
  {
  if (delta >= 0)
  {
    for (int i = ledMin; i < ledMax; i++)
    {
      analogWrite(ledPWMPin, i);
      delay(1);
    }
  }
  else
  {
    for (int i = ledMax - 1; i >= ledMin; i--)
    {
      analogWrite(ledPWMPin, i);
      delay(2);
    }
  }
  }
*/


//NeoPixel ledをfade-in, fade-out表示
void NeoPixelFade(int NeoR, int NeoG, int NeoB, int delta) //RGB値は0〜255、delta >=0ならLED fade-IN, delta <0ならLED fade-OUT
{
  if (delta >= 0)
  {
    for (int i = ledMin; i < ledMax; i++) //ledMaxはmax 255
    {
      colorWipe(strip.Color(NeoR * i / 255, NeoG * i / 255, NeoB * i / 255), 1);
    }
  }
  else
  {
    for (int i = ledMax - 1; i >= ledMin; i--)
    {
      colorWipe(strip.Color(NeoR * i / 255, NeoG * i / 255, NeoB * i / 255), 1);
    }
  }
}


// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for (int a = 0; a < 10; a++) { // Repeat 10 times...
    for (int b = 0; b < 3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for (int c = b; c < strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for (int a = 0; a < 30; a++) { // Repeat 30 times...
    for (int b = 0; b < 3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for (int c = b; c < strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

void ReadDS3231()
{
  int second, minute, hour, date, month, year;
  second = Clock.getSecond();
  minute = Clock.getMinute();
  hour = Clock.getHour(h12, PM);
  date = Clock.getDate();
  month = Clock.getMonth(Century);
  year = Clock.getYear();

  Serial.print("20");
  Serial.print(year, DEC);
  Serial.print('-');
  Serial.print(month, DEC);
  Serial.print('-');
  Serial.print(date, DEC);
  Serial.print(' ');
  Serial.print(hour, DEC);
  Serial.print(':');
  Serial.print(minute, DEC);
  Serial.print(':');
  Serial.print(second, DEC);
  Serial.print('\n');
  /*
    tm->tm_year = year;
    tm->tm_mon = month;
    tm->tm_mday = date;
    //wd[tm->tm_wday]
    tm->tm_hour = hour;
    tm->tm_min = minute;
    tm->tm_sec = second;
    Serial.printf("ReadDS3231()  %04d/%02d/%02d(%s) %02d:%02d:%02d\n",tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,wd[tm->tm_wday],tm->tm_hour, tm->tm_min, tm->tm_sec);
  */
}


//RTCの時刻設定
void GetDateStuff(byte & Year, byte & Month, byte & Day, byte & DoW,
                  byte & Hour, byte & Minute, byte & Second) {
  // Call this if you notice something coming in on
  // the serial port. The stuff coming in should be in
  // the order YYMMDDwHHMMSS, with an 'x' at the end.
  boolean GotString = false;
  char InChar;
  byte Temp1, Temp2;
  char InString[20];

  byte j = 0;
  while (!GotString) {
    if (Serial.available()) {
      InChar = Serial.read();
      InString[j] = InChar;
      j += 1;
      if (InChar == 'x') {
        GotString = true;
      }
    }
  }
  Serial.println(InString);
  // Read Year first
  Temp1 = (byte)InString[0] - 48;
  Temp2 = (byte)InString[1] - 48;
  Year = Temp1 * 10 + Temp2;
  // now month
  Temp1 = (byte)InString[2] - 48;
  Temp2 = (byte)InString[3] - 48;
  Month = Temp1 * 10 + Temp2;
  // now date
  Temp1 = (byte)InString[4] - 48;
  Temp2 = (byte)InString[5] - 48;
  Day = Temp1 * 10 + Temp2;
  // now Day of Week
  DoW = (byte)InString[6] - 48;
  // now Hour
  Temp1 = (byte)InString[7] - 48;
  Temp2 = (byte)InString[8] - 48;
  Hour = Temp1 * 10 + Temp2;
  // now Minute
  Temp1 = (byte)InString[9] - 48;
  Temp2 = (byte)InString[10] - 48;
  Minute = Temp1 * 10 + Temp2;
  // now Second
  Temp1 = (byte)InString[11] - 48;
  Temp2 = (byte)InString[12] - 48;
  Second = Temp1 * 10 + Temp2;
}
