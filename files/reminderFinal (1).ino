// -- Library for output --
#include <Streaming.h>
// -- library for using strings --
#include <string.h>
// -- library for displaying data on the expansion board --
#include <TM1638plus.h>
// -- libraries for using the clock component --
#include <DS3231.h>
#include <Streaming.h>
#include <Wire.h>

// DS3231 uses I2C bus - device ID 0x68
DS3231 rtc;
bool century = false;
bool h12Flag;
bool pmFlag;


// --Libraries for the OLED screen -------
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Assigns names to constant values for the OLED screen
#define OLED_RESET -1                 // sets the reset pin number
#define OLED_SCREEN_I2C_ADDRESS 0x3C  //sets the OLED address
#define SCREEN_WIDTH 128              // OLED display width, in pixels
#define SCREEN_HEIGHT 64              // OLED display height, in pixels
Adafruit_SSD1306 display(-1);

//states the pins to be used on the wemos board
//Assigns pin D5 to STROBE_TM
#define STROBE_TM D5
//Assigns pin D6 to CLOCK_TM 
#define CLOCK_TM D6  
//Assigns pin D7 to DIO_TM
#define DIO_TM D7    

//sets the value of the boolean variable "high_freq" to false
bool high_freq = false;  

// Constructor object (GPIO STB , GPIO CLOCK , GPIO DIO, use high freq MCU)
TM1638plus tm(STROBE_TM, CLOCK_TM, DIO_TM, high_freq);

//declares the byte used to store the data from the expansion board buttons
byte button = 0;

//creates the struct used to hold the date entered by the user
struct date {
  int day, month, year, minute, hour;
  String description;
};
//creates the array used to hold the 10 structs and initialises all values to 0
struct date dates[10] = { 0 };

//declares the variables used to take in the date from the user - these are global variables so they can be accessed by any function in the program
int apptDay;
int apptMonth;
int apptYear;
int apptMin;
int apptHour;
String apptDescription;

//variable that sets the pin that controls the LED light to D4
int blueLED = D4;


//function prototypes for the functions used in the program - all function types are void as no values are returned
void addAppt();
void changeAppt();
void viewAppt();
void sort();
void menu();
void remind();

//function that sets the real time clock time.  The function call is commented out as I have already set the time
void setDateAndTime() {
  // false = 24hr clock mode
  rtc.setClockMode(false);
  //sets the year to the current year
  rtc.setYear(22);
  //sets the month to the current month
  rtc.setMonth(12);
  //sets the day to the current day
  rtc.setDate(12);
  //sets the hour to the current hour
  rtc.setHour(15);
  //sets the minute to the current minute
  rtc.setMinute(10);
  //sets the second to the current second
  rtc.setSecond(0);
}

//this code runs once when the program is started
void setup() {
  //initiates the wire library
  Wire.begin();
  //establishes serial communication and sets the data transfer speed to 1115200
  Serial.begin(115200);
  //function call to set the time on the real time clock - this is commented out as I have already done this
  //setDateAndTime();
  //delays the program for half a second/500 miliseconds
  delay(500);

  //sets the specified pin for the LED (D4) to output
  pinMode(blueLED, OUTPUT);

  // -- code for the buzzer --
  //Sets the pin D3, which is connected to the buzzer, to output
  pinMode(D3, OUTPUT);
  //turns the buzzer off (thank goodness)
  digitalWrite(D3, LOW);



  // -- Expansion board --
  //initalises the expansion board
  tm.displayBegin(); 
  //resets the expansion board
  tm.reset();        

  // -- OLED --
  //initialises the OLED screen
  display.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_I2C_ADDRESS);  //initialises the OLED screen
  //calls the method that changes the OLED's output to the new output
  display.display();
  //pauses for 2 seconds
  delay(2000);
  //clears the display
  display.clearDisplay();
  //sets the text size to 1
  display.setTextSize(1);
  //sets the text colour to white
  display.setTextColor(WHITE);
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);

  //array of hex codes for my image of a gargoyle to be displayed on the screen
  static const uint8_t gargoyleArray[1024] = { 0xff, 0xff, 0xff, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xfe, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xfe, 0x11, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xfc, 0x8d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xfd, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xfc, 0x43, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0x80, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0x83, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xc7, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0x8f, 0xe7, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xf8, 0x0f, 0x83, 0xff, 0xff, 0xfa, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xfe, 0x0f, 0x91, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0x9c, 0x89, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xfc, 0x07, 0xff, 0xff, 0xfd, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xfc, 0x03, 0xff, 0xff, 0xf9, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xfc, 0x03, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xfe, 0x03, 0xa3, 0x9f, 0xf8, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xe2, 0x07, 0x61, 0xf3, 0xf8, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xc0, 0x07, 0xd1, 0xf2, 0x7f, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xc7, 0xf3, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0x83, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xff, 0xfd, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0x4d, 0xff, 0x7f, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xfc, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xfc, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xcf, 0xcf, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xcf, 0xef, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xcf, 0xcf, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xcf, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcf, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe1, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0xf0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0xf8, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
                                               0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00 };
  //Displays a welcome message on the OLED screen
  display << "Welcome to your" << endl;
  display << "whimsical and" << endl;
  display << "frivolous" << endl;
  display << "reminder system*." << endl;
  //sets the cursor to the bottom left of the screen
  display.setCursor(0, 45);
  display << "*may contain" << endl;
  display << "gargoyles" << endl;
  //calls the method to display my text to the screen
  display.display();
  //pauses for 5 seconds
  delay(5000);
  //clears the display
  display.clearDisplay();

  // -- code for the gargoyle image on the OLED screen --
  //draws the image of the gargoyle using the array of hex codes that make up the image
  display.drawBitmap(0, 0, gargoyleArray, 128, 64, 1);
  //displays the image of the gargoyle
  display.display();
  //pauses for 3 seconds
  delay(3000);
  //clears the display
  display.clearDisplay();
}

//this code runs constantly after the setup code has run
void loop() {
  //calls the menu function
  menu();
}

void menu() {
  //sets the brightness on the expansion board to 4
  tm.brightness(4);
  //clears the display
  display.clearDisplay();
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);
  //turns the LED off
  digitalWrite(blueLED, LOW);
  //loops through my array of structs of dates and times looking for an element that isn't zero - this will be the first appointment
  for (int i = 0; i < 10; i++) {
    //if statement that executes if an element that is not zero is found
    if (dates[i].day != 0) {
      //declares the variable that holds the string used to get the last two characters of the year, to compare with the year value obtained by the real time clock
      String yearString;
      //sets the value to the year being accessed
      yearString = String(dates[i].year);
      //uses substrings and toInt() method to get the last two digits of the year, and convert them to an int ready for comparison
      int yearDigits = yearString.substring(yearString.length() - 2, yearString.length()).toInt();
      // -- reminder 24 hours before --
      //if statement that executes if the current day & time is 24 hours before the next appointment
      if (dates[i].day - 1 == rtc.getDate() && rtc.getMonth(century) == dates[i].month && rtc.getYear() == yearDigits && dates[i].hour == rtc.getHour(h12Flag, pmFlag) && dates[i].minute == rtc.getMinute()) {
        //calls the reminder function
        remind();
      }
      // -- reminder for one hour before --
      //if statement that executes if the current day & time is one hour before the next appointment
      else if (dates[i].day == rtc.getDate() && rtc.getMonth(century) == dates[i].month && rtc.getYear() == yearDigits && dates[i].hour - 1 == rtc.getHour(h12Flag, pmFlag) && dates[i].minute == rtc.getMinute()) {
        //calls the reminder function
        remind();
      }
      // -- reminder at the time of the appointment --
      //if statement that executes if the current day & time is the same as the next appointment
      else if (dates[i].day == rtc.getDate() && rtc.getMonth(century) == dates[i].month && rtc.getYear() == yearDigits && dates[i].hour == rtc.getHour(h12Flag, pmFlag) && dates[i].minute == rtc.getMinute()) {
        //calls the reminder function
        remind();
        //removes the appointment by setting every value at the current index to zero/empty
        dates[i].day = 0;
        dates[i].month = 0;
        dates[i].year = 0;
        dates[i].minute = 0;
        dates[i].hour = 0;
        dates[i].description = "";
      }
    }
  }
  //displays the menu screen
  display << "Would you like to add a" << endl;
  display << "new appointment (1)," << endl;
  display << "change(2) or cancel (3) an" << endl;
  display << "existing appointment" << endl;
  display << "or view your next" << endl;
  display << "appointment (4)?" << endl;
  display.display();
  //sets the button variable to 0
  button = 0;
  // reads which button has been pressed
  button = tm.readButtons();
  //input validation which executes if the button pressed is not one of the four options
  while (button > 8) {
    display.clearDisplay();
    display.setCursor(0, 0);
    //outputs error message
    display << "Invalid Response -" << endl;
    display << "Try Again" << endl;
    display.display();
    //pauses for 3 seconds
    delay(3000);
    //clears the screen
    display.clearDisplay();
    display.setCursor(0, 0);
    display.display();
    button = 0;
    menu();
  }
  //this loop will activate if the button pressed is button 1
  while (button == 1 && Serial.available() >= 0) {
    //calls the add appointment function
    addAppt();
  }
  //this loop will activate if the button pressed is button 2
  while (button == 2) {
    //clears the display
    display.clearDisplay();
    //calls the change appointment function
    changeAppt();
  }
  //this loop will activate if the button pressed is button 3
  while (button == 4) {
    //clears the display
    display.clearDisplay();
    //sets then cursor to the top left position of the screen
    display.setCursor(0,0);
    //calls the cancel appointment function
    cancelAppt();
  }
  //this loop will activate if the button pressed is button 4
  while (button == 8) {
    //clears the display
    display.clearDisplay();
    //sets the cursor to the top left position of the screen
    display.setCursor(0, 0);
    //calls the view appointment function
    viewAppt();
  }
}

//add appointment function
void addAppt() {
  //sets the count variable to 0
  int count = 0;
  //loops through the array of structs to see how many available appointments there are (elements that are not 0)
  for (int i = 0; i < 10; i++) {
    if (dates[i].day == 0) {
      count++;
    }
  }
  //if statement that executes if there are no empty elements
  if (count < 1) {
    //outputs appointments full message
    display << "Apppointments full" << endl;
    display.display();
    //pauses for 3 seconds
    delay(3000);
    //calls the menu function
    menu();
  }
  //clears the display
  display.clearDisplay();
  display.setCursor(0, 0);
  //asks the user to enter the day of their appointment
  display << "Add the day of your" << endl;
  display << "appointment" << endl;
  display.display();
  //sets the variable apptDay to 0
  apptDay = 0;
  //constantly reads from the serial monitor until the user inputs a value
  do {
    apptDay = Serial.parseInt();
  } while (apptDay == 0);
  //clears the display
  display.clearDisplay();
  display.setCursor(0, 0);
  //asks the user to enter the month of their appointment
  display << "Add the month of your" << endl;
  display << "appointment" << endl;
  display.display();
  //sets the variable apptMonth to 0
  apptMonth = 0;
  //constantly reads from the serial monitor until the user inputs a value
  do {
    apptMonth = Serial.parseInt();
  } while (apptMonth == 0);
  display.clearDisplay();
  display.setCursor(0, 0);
  //asks the user to enter the year of their appointment
  display << "Add the year of your" << endl;
  display << "appointment" << endl;
  display.display();
  //sets the variable apptMonth to 0
  apptYear = 0;
  //constantly reads from the serial monitor until the user inputs a value
  do {
    apptYear = Serial.parseInt();
  } while (apptYear == 0);
  display.clearDisplay();
  display.setCursor(0, 0);
  //asks the user to enter the hour of their appointment
  display << "Add the hour of" << endl;
  display << "your appointment" << endl;
  display.display();
  //sets the variable apptHour to 0
  apptHour = 0;
  //constantly reads from the serial monitor until the user enters a value
  do {
    apptHour = Serial.parseInt();
  } while (apptHour == 0);
  display.clearDisplay();
  display.setCursor(0, 0);
  //asks the user to enter the minute of their appointment
  display << "Add the minute of" << endl;
  display << "your appointment" << endl;
  display.display();
  //sets the variable apptMin to 0
  apptMin = 0;
  //constantly reads from the serial monitor until the user enters a value
  do {
    apptMin = Serial.parseInt();
  } while (apptMin == 0);
  display.clearDisplay();
  display.setCursor(0, 0);
  //asks the user to enter a description of their appointment
  display << "Enter a description" << endl;
  display << "of your appointment" << endl;
  display.display();
  //sets the variable apptDescription to an empty string
  apptDescription = "";
  //constantly reads from serial monitor until the user enters a value
  do {
    apptDescription = Serial.readString();
  } while (apptDescription == "");
  //if statement that executes if the first element of the array of structs is empty
  if (dates[0].day == 0) {
    //assigns all of the user inputs to the relevant element in the struct
    dates[0].day = apptDay;
    dates[0].month = apptMonth;
    dates[0].year = apptYear;
    dates[0].minute = apptMin;
    dates[0].hour = apptHour;
    dates[0].description = apptDescription;
  }
  //executes if the first element is not empty
  else {
    //loops through the code until it finds the next available element
    for (int i = 0; i < 10; i++) {
      //assigns all of the user inputs to the relevant element in the struct
      if (dates[i].day != 0 && dates[i + 1].day == 0) {
        dates[i + 1].day = apptDay;
        dates[i + 1].month = apptMonth;
        dates[i + 1].year = apptYear;
        dates[i + 1].minute = apptMin;
        dates[i + 1].hour = apptHour;
        dates[i + 1].description = apptDescription;
        //breaks the loop
        break;
      }
    }
  }
  //calls the sorting function
  sort();
  //clears the display
  display.clearDisplay();
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);
  //outputs confirmation message
  display << "Appointment added" << endl;
  display.display();
  //pauses for 3 sconds
  delay(3000);
  //calls the menu function
  menu();
}

//view appointment function
void viewAppt() {
  //calls the sort function to ensure that the correct date is displayed to the user
  sort();
  //sets the variable i to 0
  int i = 0;
  //loops through the code until it finds an element that is not empty
  for (int x = 0; x < 10; x++) {
    //if the current element is empty, i increases  by one
    if (dates[i].day == 0) {
      i++;
    }
  }
  //if the final element is empty
  if (dates[9].day == 0) {
    //clears the screen
    display.clearDisplay();
    //sets the cursor to the top left position of the screen
    display.setCursor(0, 0);
    //displays "no appointments"
    display << "No appointments" << endl;
    display.display();
    //pauses for 3 seconds
    delay(3000);
    //calls the menu function
    menu();
  }
  //if there is at least one empty element
  else {
    //clears the screen
    display.clearDisplay();
    //sets the cursor to the top left position of the screen
    display.setCursor(0, 0);
    //displays "Your next appointment is on"
    display << "Your next appointment" << endl;
    display << "is on: " << endl;
    //displays the dates and time of the next appointment
    display << dates[i].day << "/" << dates[i].month << "/" << dates[i].year << endl;
    display << "for " << dates[i].description << endl;
    display << "at:" << endl;
    //sets the minString variable to a string containing the minute of the next appointment
    String minString = String(dates[i].minute);
    //sets the hourString variable to a string containing the hour of the next appointment
    String hourString = String(dates[i].hour);
    //concatenates the two strings together
    String timeString = hourString + minString;
    //converts the timeString variable to an int and assigns it to the integer variable "time"
    int time = timeString.toInt();
    //displays the time on the 7 segment display
    tm.displayIntNum(time, false);
    display.display();
    //pauses for 5 seconds
    delay(5000);
    //resets the 7 segment display
    tm.reset();
    //calls the menu function
    menu();
  }
}

//sorting function
void sort() {
  //declares the temporary variables used to sort the dates, as I will be using a bubble sort
  int tempDay;
  int tempMonth;
  int tempYear;
  int tempMin;
  int tempHour;
  String tempDescription;
  //bubble sort
  //works by using a for loop and a nested for loop
  //works by checking if the current element in the array is greater than the next. If so, the current and next element are swapped and this repeats for every element
  for (int i = 0; i < 10; i++) {
    for (int x = 0; x < 9; x++) {
      if (dates[x].year > dates[x + 1].year || dates[x].year == dates[x + 1].year && dates[x].month > dates[x + 1].month || dates[x].year == dates[x + 1].year && dates[x].month == dates[x + 1].month && dates[x].day > dates[x + 1].day || dates[x].year == dates[x + 1].year && dates[x].month == dates[x + 1].month && dates[x].day == dates[x + 1].day && dates[x].hour > dates[x + 1].hour || dates[x].year == dates[x + 1].year && dates[x].month == dates[x + 1].month && dates[x].day == dates[x + 1].day && dates[x].hour == dates[x + 1].hour && dates[x].minute > dates[x + 1].minute) {
        tempDay = dates[x].day;
        tempMonth = dates[x].month;
        tempYear = dates[x].year;
        tempMin = dates[x].minute;
        tempHour = dates[x].hour;
        tempDescription = dates[x].description;
        dates[x].day = dates[x + 1].day;
        dates[x].month = dates[x + 1].month;
        dates[x].year = dates[x + 1].year;
        dates[x].minute = dates[x + 1].minute;
        dates[x].hour = dates[x + 1].hour;
        dates[x].description = dates[x + 1].description;
        dates[x + 1].day = tempDay;
        dates[x + 1].month = tempMonth;
        dates[x + 1].year = tempYear;
        dates[x + 1].minute = tempMin;
        dates[x + 1].hour = tempHour;
        dates[x + 1].description = tempDescription;
      }
    }
  }
}

//change appointment functoin
void changeAppt() {
  //declares the variables that store the old date and time
  int oldDay;
  int oldMonth;
  int oldYear;
  int oldMin;
  int oldHour;

  //clears the display
  display.clearDisplay();
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);
  //asks the user to enter the current day of their appointment
  display << "Add the current day" << endl;
  display << "of your" << endl;
  display << "appointment" << endl;
  display.display();
  //sets oldDay to 0
  oldDay = 0;
  //constantly reads from the serial monitor until the user enters a value
  do {
    oldDay = Serial.parseInt();
  } while (oldDay == 0);
  //clears the display
  display.clearDisplay();
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);
  //asks the user to enter the current month of their appointment
  display << "Add the current" << endl;
  display << "month of your" << endl;
  display << "appointment" << endl;
  display.display();
  //sets oldMonth to 0
  oldMonth = 0;
  //constantly reads from the serial monitor until the user enters a value
  do {
    oldMonth = Serial.parseInt();
  } while (oldMonth == 0);
  //clears the screen
  display.clearDisplay();
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);
  //asks the user to enter the current year of their appointment
  display << "Add the current" << endl;
  display << "year of your" << endl;
  display << "appointment" << endl;
  display.display();
  //sets oldYear to 0
  oldYear = 0;
  //constantly reads from the serial monitor until the user enters a value
  do {
    oldYear = Serial.parseInt();
  } while (oldYear == 0);
  //clears the screen
  display.clearDisplay();
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);
  //asks the user to enter the current hour of their appointment
  display << "Add the current hour" << endl;
  display << "of your appointment" << endl;
  display.display();
  //sets oldHour to 0
  oldHour = 0;
  //constantly reads from the serial monitor until the user enters a value
  do {
    oldHour = Serial.parseInt();
  } while (oldHour == 0);
  //clears the screen
  display.clearDisplay();
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);
  //asks the user to enter the current minute of their appointment
  display << "Add the current min" << endl;
  display << "of your appointment" << endl;
  display.display();
  //sets oldMin to 0
  oldMin = 0;
  //constantly reads from the serial monitor until the user enters a value
  do {
    oldMin = Serial.parseInt();
  } while (oldMin == 0);
  //clears the screen
  display.clearDisplay();
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);
  //sets the boolean apptExist to false
  bool apptExist = false;
  //loops through the code until it finds an element that matches the user input
  for (int i = 0; i < 10; i++) {
    if (dates[i].day == oldDay && dates[i].month == oldMonth && dates[i].year == oldYear && dates[i].minute == oldMin && dates[i].hour == oldHour) {
      //sets apptExist to true
      apptExist = true;
      //sets the values for the current appointment to 0 to effectiely 'delete' the appointment
      dates[i].day = 0;
      dates[i].month = 0;
      dates[i].year = 0;
      dates[i].minute = 0;
      dates[i].hour = 0;
      dates[i].description = "";
      //variables to the store the new inputs from the user
      int newDay;
      int newMonth;
      int newYear;
      int newMin;
      int newHour;
      String newDescription;
      //clears the screen
      display.clearDisplay();
      //sets the cursor to the top left position of the screen
      display.setCursor(0, 0);
      //asks the user to enter the new day of their appointment
      display << "Add the new day" << endl;
      display << "of your" << endl;
      display << "appointment" << endl;
      display.display();
      //sets newDay to 0
      newDay = 0;
      //constantly reads from the serial monitor until the user enters a value
      do {
        newDay = Serial.parseInt();
      } while (newDay == 0);
      //updates the current element to the new day
      dates[i].day = newDay;
      //clears the screen
      display.clearDisplay();
      //sets the cursor to the top left position of the screen
      display.setCursor(0, 0);
      //asks the user to enter the new month of their appointment
      display << "Add the new" << endl;
      display << "month of your" << endl;
      display << "appointment" << endl;
      display.display();
      //sets newMonth to 0;
      newMonth = 0;
      //constantly reads from the serial monitor until the user enters a value
      do {
        newMonth = Serial.parseInt();
      } while (newMonth == 0);
      //updates the current element to the new month
      dates[i].month = newMonth;
      //clears the screen
      display.clearDisplay();
      //sets the cursor to the top left position of the screen
      display.setCursor(0, 0);
      //asks the user to enter the new year of their appointment
      display << "Add the new" << endl;
      display << "year of your" << endl;
      display << "appointment" << endl;
      display.display();
      //sets newYear to 0
      newYear = 0;
      //constantly reads from the serial monitor until the user enters a value
      do {
        newYear = Serial.parseInt();
      } while (newYear == 0);
      //updates the current element to the new year
      dates[i].year = newYear;
      //clears the screen
      display.clearDisplay();
      //sets the cursor to the top left position of the screen
      display.setCursor(0, 0);
      //asks the user to enter the new hour of their appointment
      display << "Add the new" << endl;
      display << "hour of your" << endl;
      display << "appointment" << endl;
      display.display();
      //sets newHour to 0
      newHour = 0;
      //constantly reads from the serial monitor until the user enters a value
      do {
        newHour = Serial.parseInt();
      } while (newHour == 0);
      //updates the current element to the new hour
      dates[i].hour = newHour;
      //clears the screen
      display.clearDisplay();
      //sets the cursor to the top left position of the screen
      display.setCursor(0, 0);
      display << "Add the new" << endl;
      display << "min of your" << endl;
      display << "appointment" << endl;
      display.display();
      //sets newMin to 0
      newMin = 0;
      //constantly reads from the serial monitor until the user enters a value
      do {
        newMin = Serial.parseInt();
      } while (newMin == 0);
      //updates the current element to the new minute
      dates[i].minute = newMin;
      //clears the screen
      display.clearDisplay();
      //sets the cursor to the top left position of the screen
      display.setCursor(0, 0);
      //asks the user to enter the new description of their appointment
      display << "Add the new" << endl;
      display << "description" << endl;
      display << "of your" << endl;
      display << "appointment" << endl;
      display.display();
#
        //sets newDescription to an empty string
        newDescription = "";
      //constantly reads from the serial monitor until the user enters a value
      do {
        newDescription = Serial.readString();
      } while (newDescription == "");
      //updates the current element to the new description
      dates[i].description = newDescription;
      //clears the screen
      display.clearDisplay();
      //sets the cursor to the top left position of the screen
      display.setCursor(0, 0);
      //calls the sorting function to ensure the appointments are in the correct order
      sort();
      //outputs confirmation message
      display << "Appointment updated" << endl;
      display.display();
      //pauses for 3 seconds
      delay(3000);
      //calls the menu function
      menu();
    }
  }
  //if statement that executes if apptExist is false, meaning that the appointment could not be found
  if (apptExist == false) {
    //clears the screen
    display.clearDisplay();
    //sets the cursor to the top left position of the screen
    display.setCursor(0, 0);
    //outputs error message
    display << "Appointment not" << endl;
    display << "found" << endl;
    display.display();
    //pauses for 3 seconds
    delay(3000);
    //calls the menu function
    menu();
  }
}

//cancel appointment function
void cancelAppt() {
  //variables that store the old date and time of the appointment
  int oldDay;
  int oldMonth;
  int oldYear;
  int oldMin;
  int oldHour;

  //asks the user to enter the current day of their appointment
  display << "Add the current day" << endl;
  display << "of your" << endl;
  display << "appointment" << endl;
  display.display();
  //sets oldDay to 0
  oldDay = 0;
  //constantly reads from the serial monitor until the user enters a value
  do {
    oldDay = Serial.parseInt();
  } while (oldDay == 0);
  //clears the display
  display.clearDisplay();
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);
  //asks the user to enter the current month of their appointment
  display << "Add the current" << endl;
  display << "month of your" << endl;
  display << "appointment" << endl;
  display.display();
  //sets oldMonth to 0
  oldMonth = 0;
  //constantly reads from the serial monitor until the user enters a value
  do {
    oldMonth = Serial.parseInt();
  } while (oldMonth == 0);
  //clears the screen
  display.clearDisplay();
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);
  //asks the user to enter the current year of their appointment
  display << "Add the current" << endl;
  display << "year of your" << endl;
  display << "appointment" << endl;
  display.display();
  //sets oldYear to 0
  oldYear = 0;
  //constantly reads from the serial monitor until the user enters a value
  do {
    oldYear = Serial.parseInt();
  } while (oldYear == 0);
  //clears the screen
  display.clearDisplay();
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);
  //asks the user to enter the current hour of their appointment
  display << "Add the current hour" << endl;
  display << "of your appointment" << endl;
  display.display();
  //sets oldHour to 0
  oldHour = 0;
  //constantly reads from the serial monitor until the user enters a value
  do {
    oldHour = Serial.parseInt();
  } while (oldHour == 0);
  //clears the screen
  display.clearDisplay();
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);
  //asks the user to enter the current minute of their appointment
  display << "Add the current min" << endl;
  display << "of your appointment" << endl;
  display.display();
  //sets oldMin to 0
  oldMin = 0;
  //constantly reads from the serial monitor until the user enters a value
  do {
    oldMin = Serial.parseInt();
  } while (oldMin == 0);
  //clears the screen
  display.clearDisplay();
  //sets the cursor to the top left position of the screen
  display.setCursor(0, 0);
  bool apptExist = false;
  //loops through the array of structs until an appointment with matching details is found
  for (int i = 0; i < 10; i++) {
    if (dates[i].day == oldDay && dates[i].month == oldMonth && dates[i].year == oldYear && dates[i].minute == oldMin && dates[i].hour == oldHour) {
      //sets apptExist to true
      apptExist = true;
      //sets values in the current element to empty, effectively 'deleting' them
      dates[i].day = 0;
      dates[i].month = 0;
      dates[i].year = 0;
      dates[i].minute = 0;
      dates[i].hour = 0;
      dates[i].description = "";
      //outputs confirmation message
      display << "Appointment" << endl;
      display << "successfully" << endl;
      display << "deleted" << endl;
      display.display();
      //pauses for 3 seconds
      delay(3000);
      //calls the menu function
      menu();
    }
  }
  //if statement that executes if apptExist is false, meaning the appointment could not be found
  if (apptExist == false) {
    //output error message
    display << "Appointment" << endl;
    display << "not found" << endl;
    display.display();
    //pauses for 3 seconds
    delay(3000);
    //calls the menu function
    menu();
  }
  //failsafe
  else {
    //calls the menu function
    menu();
  }
}

//reminder function
void remind() {
  //sets i to 0
  int i = 0;
  //loops through the code until the next appointment is found
  for (int x = 0; x < 10; x++) {
    if (dates[i].day == 0) {
      i++;
    } else {
      //clears the screen
      display.clearDisplay();
      //sets the cursor to the top left position of the screen
      display.setCursor(0, 0);
      //displays when the next appointment is
      display << "Your next appointment" << endl;
      display << "is on: " << endl;
      display << dates[i].day << "/" << dates[i].month << "/" << dates[i].year << endl;
      display << "for " << dates[i].description << endl;
      display << "at:" << endl;
      //converts the minute of the appt to a string and assigns it to minString
      String minString = String(dates[i].minute);
      //converts the hour of the appt to a string and assigns it to hourString
      String hourString = String(dates[i].hour);
      //concatenates the two strings together
      String timeString = hourString + minString;
      //sets the brightness of the expansion board to 7
      tm.brightness(7);
      //converts the timeString to an int and assigns it to the integer variable "time"
      int time = timeString.toInt();
      //displays the time of the appt on the 7 segment led
      tm.displayIntNum(time, false);
      //lights up every LED on expansion board
      tm.setLEDs(0xFf00);
      //turns on the LED on the breadboard
      digitalWrite(blueLED, HIGH);  
      display.display();
      //turns the buzzer on
      digitalWrite(D3, HIGH);
      //pauses for 5 seconds
      delay(5000);
      //clears the screen
      display.clearDisplay();
      display.display();
      //turns the LED on the breadboard off
      digitalWrite(blueLED, LOW);
      //turns the buzzer off (thank goodness)
      digitalWrite(D3, LOW);
      //sets the brightness of the expansion board to 0
      tm.brightness(0);
      //pauses for 1 second
      delay(1000);
    }
  }
  //resets the expansion board
  tm.reset();
  //calls the menu function
  menu();
}