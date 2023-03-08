#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
#include <time.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

const char* ssid = "RedDwarf";
const char* password = "punkrocker";

// LCD Screen 16 x 2 rows.
#define SDA 13                     // Define SDA pins
#define SCL 14                     // Define SCL pins
LiquidCrystal_I2C lcd(0x27, 16, 2); // initialize the LCD

//BUZZER & NOTES

#define BUZZZER_PIN  4 // ESP32 pin GIOP18 connected to piezo buzzer
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_B3  247
#define NOTE_C4  262

//NEOPIXELS - Circle of 8 LEDS
#define LED_PIN 12
#define LED_COUNT 8
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Melody for Buzzer and when a sale is detected.
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

// Sales value
int salesValue = 0;
// Time since last sale (in seconds)
int timeSinceLastSale = 0;
bool saleDetected = false;
unsigned long lastSaleDetectedTime = 0;
time_t lastSaleTime = 0;

void setup() {
  //Setup Serial Monitor
  Serial.begin(115200);
  Serial.println("Serial Monitor Active");

  //setup NeoPixel
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  //Setup LCD Screen
  Wire.begin(SDA, SCL);
  lcd.init();
  lcd.backlight();
  Serial.println("LCD Screen Active.");

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" connected");
  
  timeClient.begin();
  while(!timeClient.update()) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Time obtained from NTP server.");
}

void lightUpCircle() {
  for (int i = LED_COUNT - 1; i >= 0; i--) { // iterate over LEDs in reverse order
    strip.setPixelColor(i, 0, 255, 0);  // set pixel color to green
    strip.show();  // Update NeoPixels
    delay(100);
  }

  for (int i = 0; i < LED_COUNT; i++) { // iterate over LEDs in forward order
    strip.setPixelColor(i, 0);  // set pixel color to off
    strip.show();  // Update NeoPixels
    delay(100);
  }
}


void playmusic() {
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BUZZZER_PIN, melody[thisNote], noteDuration);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZZER_PIN);
  }
}

void detectSale() {
  for (int i = 0; i < 5; i++) {
    strip.fill(Adafruit_NeoPixel::Color(0, 255, 0), 0, LED_COUNT); // set all pixels to green
    strip.show();
    delay(200);
    strip.clear(); // turn off all pixels
    strip.show();
    delay(200);
  }
  saleDetected = true; // set flag
}

void loop() {
  static unsigned long lastFetchTime = 0;
  unsigned long currentMillis = millis();
  time_t now = timeClient.getEpochTime();
  Serial.println("Current time: " + String(now));

  // Fetch sales value once a minute
  if (currentMillis - lastFetchTime >= 60000) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fetching.....");
    HTTPClient http;
    http.begin("https://docs.google.com/spreadsheets/d/e/2PACX-1vSnIfxs26iDKFOrYUTsMduy9rukkBsYK4UDTKws5Ap-4IP31JavpDqZS3ouN5wXKrP_Ezu6mJZC04RU/pubhtml");
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String html = http.getString();
      int salesIndex = html.indexOf("My Total Etsy Sales</td><td class=\"s7\">");
      if (salesIndex != -1) {
        int salesValueStartIndex = html.indexOf("<td class=\"s7\">", salesIndex) + 15;
        int salesValueEndIndex = html.indexOf("</td>", salesValueStartIndex);
        int newSalesValue = html.substring(salesValueStartIndex, salesValueEndIndex).toInt();
        if (newSalesValue > salesValue) {  // check if new sale has occurred
          salesValue = newSalesValue;
          timeSinceLastSale = 0; // reset time since last sale
          Serial.println("New sale detected: " + String(salesValue));
          detectSale(); // call detectSale function
          playmusic(); // play music
          saleDetected = true; // set flag
          lastSaleTime = now; // get the current time from NTP server
        }
      }
    }
    http.end();
    lastFetchTime = currentMillis;
  }

  // check if sale detected flag is set and turn off NeoPixel circle after 5 seconds
  if (saleDetected && (currentMillis - lastSaleDetectedTime) <= 5000) {
    // Sale detected, keep circle on
    lightUpCircle();
  } else {
    // Turn off circle after 5 seconds
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, 0);
    }
    strip.show();
    saleDetected = false; // reset flag
  }

  lastSaleDetectedTime = currentMillis;

  // Calculate time since last sale in minutes
  unsigned long timeElapsed = difftime(now, lastSaleTime);
  timeSinceLastSale = timeElapsed / 60;

  int hours = timeSinceLastSale / 60;
  int minutes = timeSinceLastSale % 60;

  // Switch between screens every 10 seconds
  switch (currentMillis / 10000 % 2) {
    case 0:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Total Etsy Sales");
      lcd.setCursor(6, 1);
      lcd.print(salesValue);
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Last Sale Date");
      lcd.setCursor(0, 1);
      char datetimeStr[20];
      struct tm * timeinfo;
      timeinfo = localtime(&lastSaleTime);
      strftime(datetimeStr, 20, "%d/%m/%y %H:%M:%S", timeinfo);
      lcd.print(datetimeStr);
      break;
  }

  delay(3000);
}


