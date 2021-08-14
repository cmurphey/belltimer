
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

// Relay constants
const int relayPin = 2;

// Adjust to set amount of time the relay should be on for each ring (in milliseconds)
const int bellRelayOnTimeSec = 1825;

// Adjust to set hours that will ring
const int numDailyRingHours = 2;
const int bellDailyRingHours[numDailyRingHours] = {12, 18};

// Adjust to set days that should ring - Sunday = 1, Saturday = 7
const int numDailyRingDays = 7;
const int bellDailyRingDays[numDailyRingDays] = {1, 2, 3, 4, 5, 6, 7};

// Adjust to set Sunday ring time and count
const int bellSundayRingHour = 9;
const int bellSundayRingMinute = 30;
const int bellSundayRingCount = 3;

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

// Ring variables
int lastRingHour = 0;
int lastSundayDay = 0;

void setup() {
  // Begin sending data to the serial port
  //Serial.begin(9600);

  // Initialize the LCD
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);

  // Initialize Relay
  pinMode(relayPin, OUTPUT);

  // Start the I2C interface
  Wire.begin();

  // Run if RTC gets zeroed out somehow - this needs to be left commented out otherwise
//  clock.setClockMode(false);  // set to 24h
//  clock.setYear(21); // 2 digit
//  clock.setMonth(8);
//  clock.setDate(13);
//  clock.setDoW(6); // 1 = Sun, 7 = Sat
//  clock.setHour(22); // 24h
//  clock.setMinute(15);
}

void loop() {
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

void ringBell(int count) {
  lcd.clear();
  // Bell needs to be left on for it to ring multiple times
  for (int i = 1; i <= count; i++) {
    lcd.setCursor(0, 0);
    lcd.print("Ring ");
    lcd.print(i);
    lcd.print(" of ");
    lcd.print(count);

    // Relay on
    digitalWrite(relayPin, HIGH);
    delay(bellRelayOnTimeSec);
  }
  // Relay off
  digitalWrite(relayPin, LOW);
  lcd.clear();
}

void checkRingTimer() {
  int currentHour = clock.getHour(h12Flag, pmFlag);
  int currentMinute = clock.getMinute();
  int currentSecond = clock.getSecond();
  if (isRingDay() 
      && isRingHour() 
      && currentMinute == 0 
      && currentSecond == 0) {
    int count = currentHour;
    if (count > 12) {
      count -= 12;
    }
    ringBell(count);
  }
  if (isChurchTime()) {
     ringBell(bellSundayRingCount);
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
  int currentHour = clock.getHour(h12Flag, pmFlag);
  for (int i = 0; i < numDailyRingHours; i++) {
    if (bellDailyRingHours[i] == currentHour) {
      return true;
    }
  }
  return false;
}

boolean isChurchTime() {
  // Check if it's church time
  int currentHour = clock.getHour(h12Flag, pmFlag);
  int currentMinute = clock.getMinute();
  int currentSecond = clock.getSecond();
  if (dow == 1 
      && currentHour == bellSundayRingHour 
      && currentMinute == bellSundayRingMinute 
      && currentSecond == 0) {
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
  year = clock.getYear();
  month = clock.getMonth(century);
  day = clock.getDate();
  dow = clock.getDoW();
  hour = clock.getHour(h12Flag, pmFlag); //24-hr
  minute = clock.getMinute();
  second = clock.getSecond();

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
    case 1:
      lcd.print("Sun");
      break;
    case 2:
      lcd.print("Mon");
      break;
    case 3:
      lcd.print("Tue");
      break;
    case 4:
      lcd.print("Wed");
      break;
    case 5:
      lcd.print("Thu");
      break;
    case 6:
      lcd.print("Fri");
      break;
    case 7:
      lcd.print("Sat");
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
    if (dow == 7) {
      dow = 1;
    } else {
      dow = dow + 1;
    }
  }
  if (lcdKey == btnDOWN) {
    if (dow == 1) {
      dow = 7;
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
  lcd.clear();

  clock.setClockMode(false);  // set to 24h
  clock.setYear(year);
  clock.setMonth(month);
  clock.setDate(day);
  clock.setDoW(dow);
  clock.setHour(hour);
  clock.setMinute(minute);

  lcd.setCursor(0, 0);
  lcd.print("Saved");
  
  delay(500);
}
