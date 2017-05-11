/*
    @Project Name: Watch'INT-Arduino Part
    @author: CHEN Muyao, FIS Victor
    @date: 13/03/2017
    @discription: Prototype d'une montre connectée

    This prototype is aimed at showing the time on the OLED screen by synchronizing the time via Bluetooth
    Time-sychronizing messages consists of the letter T followed by ten digit time (as seconds since Jan 1 1970)
    For example, we can send the text on the next line using Bluetooth Terminal to set the clock to noon Jan 1 2013: T1357041600
    This watch can also show the amount of different types of notifications and their contents. With the help of a button and a buzzer,
    we simplify the human-machine interaction.

    Wring:

    Bluetooth TX-10(RX on the Arduino)
    Bluetooth RX-11(TX on the Arduino)
    Screen SDA_PIN-A4
    Screen SCL_PIN-A5
    Button 7
    Buzzer 9
*/

#include <TimeLib.h>    // Library of Time

// Library of the screen
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#include <SoftwareSerial.h>    // Library of software serial communication

// Define the pins for RX, TX on the Arduino board respectively to connect with Bluetooth
SoftwareSerial mySerial(10, 11);

// Define the pins for the I²C Bus to communicate with OLED
#define SDA_PIN A4    //SDA pin of OLED connected to A4
#define SCL_PIN A5    //SCL pin of OLED connected to A5

// Define pin for buzzer
#define buzzer 9

// Define the objet "display" of OLED
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// Time message define
#define TIME_HEADER  "T"    // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message

// Info message define
#define INFO_HEADER "I"    // Header tag for notification message
int typeInfo = 0;    // Indicator for the type of infomation folowing INFO_HEADER
char content[20];    // Array to store the content of message or mail

const int BUTTON = 7;    // Button define

void setup()  {

  // Initialisation of buzzer
  pinMode(buzzer, OUTPUT); // Set buzzer - pin 9 as an output

  // Initialisation of button
  pinMode(BUTTON, INPUT);

  // Initialisation of Bluetooth communication module
  mySerial.begin(38400);    // Set the baud of port to 38400 in order to communicate with Bluetooth
  pinMode(13, OUTPUT);    // Define the indicator Led PIN 13 as output

  // Time synchronisation request
 // setSyncProvider(requestSync);    // Set function to call when sync required
//ne semble pas être nécéssaire

  // Initialisation of OLED Screen module
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize with the I2C addr 0x3C (for the 128x64)
  display.clearDisplay(); // Show splashscreen
  display.setTextSize(2);    // Set the text size
  display.setTextColor(WHITE);    // Set the text color as white (white seen as blue actually)
  display.setCursor(10, 20);   // Set the position of the 1st letter
  display.print("WATCH'INT");    // Show the welcome page in waiting for the sync

  tone(buzzer, 2093); // 0.1 sec of buzz indicating the watch boot
  delay(100);
  noTone(buzzer);

  display.display();    // Update all the changes to the screen

  // If the bluetooth serial communication is established, then process the synchronisation
  while (!mySerial.available()) {    // If sync signal is not received
    delay(10);    // Wait for the signal
  }
  processSyncMessage();    // Call a function to react to received messages
    
  tone(buzzer, 2093);    // Buzz indicating successful sync
  delay(100);
  tone(buzzer, 1568);
  delay(100);
  noTone(buzzer);
}

/*---------------------------------------------------RESET--------------------------------------------------------*/
void (*resetFunc)(void) = 0;    // Set the pointer to the beginning in order to reset
/*----------------------------------------------------------------------------------------------------------------*/

void loop() {
  // Detect the reset signal
  int held = 0;
  while ((digitalRead(BUTTON) == HIGH) && (held < 10)) {
    delay(100);
    held++;
  }

  if (held >= 10)
    resetFunc();    // If the button is held pushed for 1 sec, watch reset
  else if(held < 10 && held != 0)
      mySerial.println("C");    // If not, send the request to clear the notif

  checkInfo();    // Notification checking
  delay(800);
  timeshow();    // Time showing

}

/*-------------------------------------AFFICHAGE DU TEMPS ET DE L'HEURE-------------------------------------------*/
void timeshow() {
  // If time status is set
  if (timeStatus() != timeNotSet) {
    display.clearDisplay();
    digitalClockDisplay();    // Clock is displayed
  }

  // If the time is set
  if (timeStatus() == timeSet) {
    digitalWrite(13, HIGH); // LED on if synced
  } else {
    digitalWrite(13, LOW);  // LED off if needs refresh
  }
}

void digitalClockDisplay() {
  // Digital clock display of the time
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15, 20);
  display.print(hour());    // Display time
  printDigits(minute());
  printDigits(second());

  // Change the line to display the date
  display.setCursor(15, 40);
  display.print(day());    //Display date
  display.print("/");
  display.print(month());
  display.print("/");
  display.print(year());
  display.display();
}

void printDigits(int digits) {
  // Utility function for digital clock display: prints preceding colon and leading 0
  display.print(":");
  if (digits < 10)   // If the number is between 0 and 9
    display.print('0');    // Print a 0 before
  display.print(digits);    // Display minutes or seconds
}


void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  if (mySerial.find(TIME_HEADER)) {
    pctime = mySerial.parseInt();
    if ( pctime >= DEFAULT_TIME) { // Check the integer is a valid time (greater than Jan 1 2013)
      setTime(pctime); // Sync Arduino clock to the time received on the serial port
    }
  }
}

time_t requestSync() {
  mySerial.write(TIME_REQUEST);
  return 0; // The time will be sent later in response to serial mesg
}
/*----------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------CHECK INFO---------------------------------------------------*/
void checkInfo() {
  // Check for new notification
  int typeInfo_before = typeInfo;    // Store the previous information type

  // Get the notification type
  if (mySerial.available()) {
    if (mySerial.find(INFO_HEADER))    // Find the header of notification "I"
    {
      typeInfo = mySerial.parseInt();    // Extract type of information as integer
    }
  }

  // Divide the integer to get seperately the amount of new notif for each type
  int typeInfo_local = typeInfo;    // Define a local variable to process this calculate
  int Nb_S;    // Define the indicator of SMS
  int Nb_M;    // Define the indicator of Mail
  int Nb_R;    // Define the indicator of Remind

  Nb_R = typeInfo_local % 10;
  typeInfo_local = typeInfo_local / 10;
  Nb_M = typeInfo_local % 10;
  typeInfo_local = typeInfo_local / 10;
  Nb_S = typeInfo_local % 10;

  showNotif(Nb_S, Nb_M, Nb_R);    // Call the function to show the amount of notif

  // Update the content of the array if there is any change
  if (typeInfo_before != typeInfo)
  {
    if ((Nb_S != 0) || (Nb_M != 0) || (Nb_R != 0))    // If there is any new message, get the content
    {
      for (int i = 0; i < 20 ; ++i)    // Buffer of 20 characters
      {
        content[i] = mySerial.read();
        delay(10);
      }
    }
    else    // If there is not any message, show nothing
    {
      for (int i = 0; i < 20 ; ++i)
      {
        content[i] = '\0';
      }
    }
  }

  showContent(content);    // Call this function to show the content


}

// Function showing the Notif on the screen
void showNotif(int Nb_S, int Nb_M, int Nb_R) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1, 1);

  display.print("S : ");
  display.print(Nb_S);
  display.print("  ");
  display.print("M : ");
  display.print(Nb_M);
  display.print("  ");
  display.print("R : ");
  display.print(Nb_R);
  display.display();
}

// Function showing the content of message on the screen
void showContent(char content[]) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);

  for (int i = 0; i < 20; ++i)
  {
    display.print(content[i]);
  }

  display.display();
}
/*----------------------------------------------------------------------------------------------------------------*/
