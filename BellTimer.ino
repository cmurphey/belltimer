
#include <LiquidCrystal.h>
#include <DS3231.h>
#include <Wire.h>

// Select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Initialize clock library
DS3231 clock;

// LCD button constants
const int btnRIGHT = 0;
const int btnUP = 1;
const int btnDOWN = 2;
const int btnLEFT = 3;
const int btnSELECT = 4;
const int btnNONE = 5;

// Adjust to set amount of time the relay should be on (in milliseconds)
const int bellRelayOnTimeSec = 1000;

// Adjust to set the amount of time between rings (in milliseconds)
const int bellDelayBetweenRingsMs = 1000;

// Adjust to set hours that will ring
const int numDailyRingHours = 2;
const int bellDailyRingHours[numDailyRingHours] = {12, 18};

// Adjust to set days that should ring - Monday = 0, Sunday = 6
const int numDailyRingDays = 7;
const int bellDailyRingDays[numDailyRingDays] = {0, 1, 2, 3, 4, 5, 6};

// Adjust to set Sunday ring time and count
const int bellSundayRingHour = 9;
const int bellSundayRingMinute = 30;
const int bellSundayRingCount = 10;

// Date/Time variables
int hour;
int minute;
int second;
int year;
int month;
int day;
int dow;
int menu = 0;
bool century = false;
bool h12Flag;
bool pmFlag;

// Temporary internal clock
DateTime tempClock = DateTime(2021, 8, 12, 11, 59, 55);
long timeSec = tempClock.unixtime();
long lastMillis;

// Ring variables
int lastRingHour = tempClock.hour();
int lastSundayDay = 0;

void setup() {
  // Begin sending data to the serial port
  Serial.begin(9600);

  // Initialize the LCD
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);

  // Start the I2C interface
  Wire.begin();

  displayDateTime();
}

void loop() {
  // Temporary clock code
  updateTempClock();
  
  // Read the lcd buttons
  int lcdKey = readLCDButtons();

  // Display menu or show time
  displayMenu(lcdKey);

  // Ring the bell on button
  if (lcdKey == btnRIGHT && menu == 0) {
    ringBell(1);
  }

  // Ring based on timer
  checkRingTimer();
}

void updateTempClock() {
  long newMillis = millis();
  int secondsChange = (newMillis - lastMillis) / 1000;
  if (secondsChange > 0) {
    timeSec += secondsChange;
    lastMillis = newMillis;
  }
}

void ringBell(int count) {
  lcd.clear();
  for (int i = 1; i <= count; i++) {
    lcd.setCursor(0, 0);
    lcd.print("Ring ");
    lcd.print(i);
    lcd.print(" of ");
    lcd.print(count);

    // Relay on
    delay(bellRelayOnTimeSec);
    // Relay off
    delay(bellDelayBetweenRingsMs);
  }
}

void checkRingTimer() {

  int currentHour = tempClock.hour();
  int currentDay = tempClock.day();
  
  if (isRingDay() && isRingHour() && lastRingHour != currentHour) {
    int count = currentHour;
    if (count > 12) {
      count -= 12;
    }
    ringBell(count);
    lastRingHour = currentHour;
  }
  if (isChurchTime() && lastSundayDay != currentDay) {
     ringBell(bellSundayRingCount);
     lastSundayDay = currentDay;
  }
}

boolean isRingDay() {
  // Check if this is a ring day
  for (int i = 0; i < numDailyRingDays; i++) {
    if (dow == bellDailyRingDays[i]) {
      return true;
    }
  }
  return false;
}

boolean isRingHour() {
  // Check if this is a ring hour
  int currentHour = tempClock.hour();
  for (int i = 0; i < numDailyRingHours; i++) {
    if (bellDailyRingHours[i] == currentHour) {
      return true;
    }
  }
  return false;
}

boolean isChurchTime() {
  // Check if it's church time
  int currentHour = tempClock.hour();
  int currentMinute = tempClock.minute();
  if (dow == 6 && currentHour == bellSundayRingHour && currentMinute == bellSundayRingMinute) {
    return true;
  }
  return false;
}

int readLCDButtons() {
  // Read the value from the sensor
  int adc_key_in = analogRead(0);
  if (adc_key_in > 1500) return btnNONE;
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 450)  return btnDOWN;
  if (adc_key_in < 700)  return btnLEFT;
  if (adc_key_in < 900)  return btnSELECT;
  return btnNONE;
}

void displayMenu(int lcdKey) {
    // Change menu screen based on the select button
  if (lcdKey == btnSELECT) {
    menu = menu + 1;
  }

  // Display correct menu
  if (menu == 0) {
    displayDateTime();
  }
  if (menu == 1) {
    displaySetHour(lcdKey);
  }
  if (menu == 2) {
    displaySetMinute(lcdKey);
  }
  if (menu == 3) {
    displaySetYear(lcdKey);
  }
  if (menu == 4) {
    displaySetMonth(lcdKey);
  }
  if (menu == 5) {
    displaySetDay(lcdKey);
  }
  if (menu == 6) {
    displaySetDoW(lcdKey);
  }
  if (menu == 7) {
    storeTime();
    menu = 0;
  }
}

void displayDateTime() {
  // Show the current date and time
//  year = clock.getYear();
//  month = clock.getMonth(century);
//  day = clock.getDate();
//  dow = clock.getDoW();
//  hour = clock.getHour(h12Flag, pmFlag); //24-hr
//  minute = clock.getMinute();
//  second = clock.getSecond();

  // Temporary clock based on millis + start time (fixed)
  tempClock = DateTime(timeSec);
  year = tempClock.year();
  month = tempClock.month();
  day = tempClock.day();
  hour = tempClock.hour();
  minute = tempClock.minute();
  second = tempClock.second();

  lcd.setCursor(0, 0);
  lcd.print(month, DEC);
  lcd.print("/");
  lcd.print(day, DEC);
  lcd.print("/");
  lcd.print(year, DEC);
  lcd.print(" ");
  displayDayOfWeek();

  lcd.setCursor(0, 1);
  if (hour <= 9) {
    lcd.print("0");
  }
  lcd.print(hour, DEC);
  lcd.print(":");
  if (minute <= 9) {
    lcd.print("0");
  }
  lcd.print(minute, DEC);
  lcd.print(":");
  if (second <= 9) {
    lcd.print("0");
  }
  lcd.print(second, DEC);
}

void displayDayOfWeek() {
  switch(dow) {
    case 0:
      lcd.print("Mon");
      break;
    case 1:
      lcd.print("Tue");
      break;
    case 2:
      lcd.print("Wed");
      break;
    case 3:
      lcd.print("Thu");
      break;
    case 4:
      lcd.print("Fri");
      break;
    case 5:
      lcd.print("Sat");
      break;
    case 6:
      lcd.print("Sun");
      break;
    default:
      break;
  }
}

void displaySetHour(int lcdKey) {
  // time setting
  lcd.clear();
  if (lcdKey == btnUP) {
    if (hour == 23) {
      hour = 0;
    } else {
      hour = hour + 1;
    }
  }
  if (lcdKey == btnDOWN) {
    if (hour == 0) {
      hour = 23;
    } else {
      hour = hour - 1;
    }
  }
  lcd.setCursor(0, 0);
  lcd.print("Hour:");
  lcd.setCursor(0, 1);
  lcd.print(hour, DEC);
  delay(200);
}

void displaySetMinute(int lcdKey) {
  // Setting the minutes
  lcd.clear();
  if (lcdKey == btnUP) {
    if (minute == 59) {
      minute = 0;
    }
    else {
      minute = minute + 1;
    }
  }
  if (lcdKey == btnDOWN) {
    if (minute == 0) {
      minute = 59;
    } else {
      minute = minute - 1;
    }
  }
  lcd.setCursor(0, 0);
  lcd.print("Minutes:");
  lcd.setCursor(0, 1);
  lcd.print(minute, DEC);
  delay(200);
}

void displaySetYear(int lcdKey) {
  // Setting the year
  lcd.clear();
  if (lcdKey == btnUP) {
    year = year + 1;
  }
  if (lcdKey == btnDOWN) {
    year = year - 1;
  }
  lcd.setCursor(0, 0);
  lcd.print("Year:");
  lcd.setCursor(0, 1);
  lcd.print(year, DEC);
  delay(200);
}

void displaySetMonth(int lcdKey) {
  // Setting the month
  lcd.clear();
  if (lcdKey == btnUP) {
    if (month == 12) {
      month = 1;
    }
    else {
      month = month + 1;
    }
  }
  if (lcdKey == btnDOWN) {
    if (month == 1) {
      month = 12;
    } else {
      month = month - 1;
    }
  }
  lcd.setCursor(0, 0);
  lcd.print("Month:");
  lcd.setCursor(0, 1);
  lcd.print(month, DEC);
  delay(200);
}

void displaySetDay(int lcdKey) {
  // Setting the day
  lcd.clear();
  if (lcdKey == btnUP) {
    if (day == 31) {
      day = 1;
    } else {
      day = day + 1;
    }
  }
  if (lcdKey == btnDOWN) {
    if (day == 1) {
      day = 31;
    } else {
      day = day - 1;
    }
  }
  lcd.setCursor(0, 0);
  lcd.print("Day of month:");
  lcd.setCursor(0, 1);
  lcd.print(day, DEC);
  delay(200);
}

void displaySetDoW(int lcdKey) {
  // Setting the day of the week
  lcd.clear();
  if (lcdKey == btnUP) {
    if (dow == 6) {
      dow = 0;
    } else {
      dow = dow + 1;
    }
  }
  if (lcdKey == btnDOWN) {
    if (dow == 0) {
      dow = 6;
    } else {
      dow = dow - 1;
    }
  }
  lcd.setCursor(0, 0);
  lcd.print("Day of week:");
  lcd.setCursor(0, 1);
  displayDayOfWeek();
  delay(200);
}

void storeTime() {
  // Variable saving
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Saved");

//  clock.setClockMode(false);  // set to 24h
//  clock.setYear(year);
//  clock.setMonth(month);
//  clock.setDate(day);
//  clock.setDoW(dow);
//  clock.setHour(hour);
//  clock.setMinute(minute);

  // Temporary store to local variable
  DateTime newDateTime = DateTime(year, month, day, hour, minute, 0);
  timeSec = newDateTime.unixtime();
  
  delay(500);
}
