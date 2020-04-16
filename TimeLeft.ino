#pragma region includes
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Time.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#pragma endregion

// Network details
const char *ssid = "Livebox-EC80";
const char *password = "Your password";

int start_hour = 8;
int start_minute = 30;
int start_second = 0;

int stop_hour = 23;
int stop_minute = 59;
int stop_second = 59;

int total_seconds = 0; //Total seconds in between the start and stop interval
int left_seconds = 0;  //seconds left in that interval

LiquidCrystal_I2C lcd(0x27, 20, 4);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "fr.pool.ntp.org", 3600 * 2);

void setup()
{
    Serial.begin(9600);
    timeClient.begin();

    total_seconds = calculateTotalSeconds();

    lcd.init();
    lcd.backlight();

    WiFi.begin(ssid, password);
    WiFi.mode(WIFI_STA);
    WiFi.hostname("Date");

    while (WiFi.status() != WL_CONNECTED)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Connecting Wifi");
        delay(1000);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wifi connected");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());

    delay(2000);
    lcd.clear();
}

void loop()
{
    timeClient.update();
    left_seconds = calculateLeftSeconds();
    /*if (left_seconds > 0 && left_seconds < total_seconds)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("lol");
    }
    else
    {*/
    updateLCD();
    //}
    delay(2500); //refresh every 2.5 seconds
}

/**
 * Calculates the total seconds between the start and stop bounds
 */
int calculateTotalSeconds()
{
    int h = (stop_hour - start_hour) * 3600;
    int m = (stop_minute - start_minute) * 60;
    int s = (stop_second - start_second);
    return h + m + s;
}

/**
 * Calculates the second left, from the current time to the stop bound
 */
int calculateLeftSeconds()
{
    int h = (stop_hour - timeClient.getHours()) * 3600;
    int m = (stop_minute - timeClient.getMinutes()) * 60;
    int s = (stop_second - timeClient.getSeconds());
    return h + m + s;
}

/**
 * updates the lcd with correct values 
 */
void updateLCD()
{
    lcd.clear();
    //Line 1
    LCD_progress_bar(0, left_seconds, 0, total_seconds);

    //Line 2
    lcd.setCursor(0, 3);
    lcd.print("Time Left: ");
    //Hours
    if (stop_hour - timeClient.getHours() < 10) //leading zero
    {
        lcd.print("0");
    }
    lcd.print(stop_hour - timeClient.getHours());
    lcd.print(":");

    //minutes
    if (stop_minute - timeClient.getMinutes() < 10) //leading zero
    {
        lcd.print("0");
    }
    lcd.print(stop_minute - timeClient.getMinutes());
    lcd.print(":");

    //seconds
    if (stop_second - timeClient.getSeconds() < 10) //leading zero
    {
        lcd.print("0");
    }
    lcd.print(stop_second - timeClient.getSeconds());

    //Line 3
    lcd.setCursor(0, 1);
    lcd.print("Current: ");
    lcd.print(timeClient.getFormattedTime());

    //Line 4
    /*
    lcd.setCursor(0, 2);
    lcd.print(left_seconds);
    lcd.print(" ");
    lcd.print(total_seconds);
    */
}

/**
 * Creates a progress bar
 */
void LCD_progress_bar(int row, int var, int minVal, int maxVal)
{
    if (var > minVal && var < maxVal)
    {
        int block = map(var, minVal, maxVal, 0, 20); // Block represent the current LCD space (modify the map setting to fit your LCD)
        int line = map(var, minVal, maxVal, 0, 100); // Line represent the theoretical lines that should be printed
        int bar = (line - (block * 5));              // Bar represent the actual lines that will be printed

        /* LCD Progress Bar Characters, create your custom bars */

        byte bar1[8] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
        byte bar2[8] = {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18};
        byte bar3[8] = {0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C};
        byte bar4[8] = {0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E};
        byte bar5[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
        lcd.createChar(1, bar1);
        lcd.createChar(2, bar2);
        lcd.createChar(3, bar3);
        lcd.createChar(4, bar4);
        lcd.createChar(5, bar5);

        for (int x = 0; x < block; x++) // Print all the filled blocks
        {
            lcd.setCursor(x, row);
            lcd.write(1023);
        }

        lcd.setCursor(block, row); // Set the cursor at the current block and print the numbers of line needed
        if (bar != 0)
            lcd.write(bar);
        if (block == 0 && line == 0)
            lcd.write(1022); // Unless there is nothing to print, in this case show blank

        for (int x = 20; x > block; x--) // Print all the blank blocks
        {
            lcd.setCursor(x, row);
            lcd.write(1022);
        }
    }
}
