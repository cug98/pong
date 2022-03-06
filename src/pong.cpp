/*
* Implementation eines 1D-Pong Spieles im Rahmen der Übung zu Eingebettete Systene WT2022
* 
* Spielablauf: Zum Starten des Spieles müüsen nacheinander SW1 und SW2 gedrückt werden. Um den Ball zurückzuspielen,
* muss der entsprechende Switch getriggert werden, sobald der Ball sich auf den letzten beiden 2 LEDs befindet.
* Ein Spiel endet, sobald ein Spieler 2 Sätze gewonnen hat, wobei ein Satz aus 10 Punkten besteht
* 
* Abgabe von Übungsgruppe 1, Gruppe 9:
* Maximilian Filzer, Matrikelnummer: 1196756
* Christian Galda, Matrikelnummer: 1224466
*/

#include <Arduino.h>
#include <avr/io.h>
#include <ss_oled.h>

#define LED_NUM_MAX 30

//Display config
SSOLED ssoled;
#define SDA_PIN -1
#define SCL_PIN -1
// no reset pin needed
#define RESET_PIN -1
// let ss_oled find the address of our display
#define OLED_ADDR -1
#define FLIP180 0
#define INVERT 0
// Use the default Wire library
#define USE_HW_I2C 1

//Speichern des Punktestandes
long int score0 = 0, score1 = 0;
long int set0 = 0, set1 = 0;

//Variablen für Spielablauf
int position = LED_NUM_MAX / 2;
int direction = 1;
int start_delay = 400;
int current_delay = start_delay;

//Flags für Programmablauf
bool p0ready = false, p1ready = false;
bool updateDisplay = false;

//timestamps zum Prellen der Switches
unsigned int prellZeit = 500;
unsigned int interrupt0Zeit = 0;
unsigned int interrupt1Zeit = 0;

extern "C"
{
  // function prototypes
  void STRIPE_show(uint16_t index, char r, char g, char b, char bright);
  void STRIPE_com(uint8_t one_byte);
  void red_LED(void);
}

void initStripe(void);

void scoreUser(int);

void updateDisplayGame(void);
void updateDisplayWinner(void);
void updateDisplayStart(void);
void increase_speed(void);


void interrupt_user_0(void);
void interrupt_user_1(void);



void setup()
{
    //initialisierung des LED-Streifens
    initStripe();

    //Definieren der Interrupt-Funktionen
    attachInterrupt(digitalPinToInterrupt(2), interrupt_user_0, RISING);
    attachInterrupt(digitalPinToInterrupt(3), interrupt_user_1, RISING);
    
    //Display config
    int rc;
    rc = oledInit(&ssoled, OLED_128x64, OLED_ADDR, FLIP180, INVERT, USE_HW_I2C, SDA_PIN, SCL_PIN, RESET_PIN, 400000L);       // Standard HW I2C bus at 400Khz

    if (rc != OLED_NOT_FOUND)
    {
      updateDisplayStart();
    }
}

void loop()
{
  if(p0ready && p1ready)
  {  
    STRIPE_show(position, 0, 0, 0, 0);//schalte LED aus
    position += direction; //ändere Position der LED
    STRIPE_show(position, 0, 0, 10, 5); //schalte LED an
    
    //falls im Aus
    if(position <= 0)
    {
      //Buzzer
      digitalWrite(11, HIGH);
      delay(300);
      digitalWrite(11, LOW);
      
      scoreUser(0);
    }
    else if (position >= LED_NUM_MAX)
    {
      //Buzzer
      digitalWrite(11, HIGH);
      delay(300);
      digitalWrite(11, LOW);
      
      scoreUser(1);
    }
    
    delay(current_delay);
    
    //Flag, um unnötiges Updaten des Displays zu vermeiden
    if(updateDisplay)
    {
    if(set0 < 2 && set1 < 2)
      {
        updateDisplayGame();
      }
      else{
        updateDisplayWinner();
      }
      updateDisplay = false;
    }
  }
}




void interrupt_user_0()
{
  //falls Spiel läuft und der Switch nicht innerhalb der letzten 500ms aktiviert wurde
  if(p0ready && p1ready && millis() - interrupt0Zeit > prellZeit)
  {
    if(position == LED_NUM_MAX - 1 || position == LED_NUM_MAX - 2)
    {
      direction = direction * -1;
      increase_speed();
    }
    else
    {
      scoreUser(1);
    }
    interrupt0Zeit = millis();
  }
  p0ready = true;
  updateDisplay = true;
}

void interrupt_user_1()
{
  //falls Spiel läuft und der Switch nicht innerhalb der letzten 500ms aktiviert wurde
  if(p0ready && p1ready && millis() - interrupt1Zeit > prellZeit)
  {
    if(position == 0 || position == 1)
    {
      direction = direction * -1;
      increase_speed();
    }
    else
    {
      scoreUser(0);
    }
    interrupt1Zeit = millis();
  }
  p1ready = true;
  updateDisplay = true;
}

void increase_speed()
{
  if(current_delay >= 100)
  {
    current_delay -= 50;
  }
}

void scoreUser(int user)
{
  updateDisplay = true; // update Score auf Display
  
  //abhängig davon, welcher User gepunktet hat
  if(user == 0)
  {
    score0++;
    direction = 1; //ändere Richtung in Richtung des scoring players
    
    //falls Satz gewonnen
    if(score0 > 9){
      score0 = 0;
      score1 = 0;
      set0++;
    }
  }
  else
  {
    score1++;
    direction = -1; //change direction towards scoring player

    //falls Satz gewonnen
    if(score1 > 9){
      score0 = 0;
      score1 = 0;
      set1++;
    }
  }

  STRIPE_show(LED_NUM_MAX - 1, 0, 0, 0, 0); //schalte alle LEDs aus
  current_delay = start_delay; //reset Geschwindigkeit
  position = LED_NUM_MAX / 2; //LED startet in der Mitte
  return;
}

//Startbildschirm
void updateDisplayStart()
{
  oledFill(&ssoled, 0, 1);
  oledWriteString(&ssoled, 0, 0, 0, (char *)"please press", FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 1, (char *)"both buttons", FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 2, (char *)"to start", FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 3, (char *)"the game", FONT_NORMAL, 0, 1);
}

//Spielbildschirm mit aktuellem Punktestand
void updateDisplayGame()
{
  char set[] = "Sets: 0:0";
  set[6] = set0 + '0';
  set[8] = set1 + '0';
    
  char score[] = "0:0";
  score[0] = score0 + '0';
  score[2] = score1 + '0';
  oledFill(&ssoled, 0, 1);
  oledWriteString(&ssoled, 0, 16, 0, set, FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 16, 2,(char *)"Score:", FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 40, 4, score, FONT_STRETCHED, 0, 1);

  return;
}

//Endbildschirm mit Anzeige des Gewinners
void updateDisplayWinner()
{
  char * winner = (set0 >= 2) ? (char *) "Player 1" : (char *) "Player 2";
  oledFill(&ssoled, 0, 1);
  oledWriteString(&ssoled, 0, 16, 2,(char *)"Winner:", FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 4, winner, FONT_STRETCHED, 0, 1);
  p0ready = false;
  p1ready = false;
  set0 = 0;
  set1 = 0;
}
 
void initStripe()
{
  pinMode(51, OUTPUT); //  DDRB |= (1 << PB2);//OUTPUT MOSI
  pinMode(52, OUTPUT); //  DDRB |= (1 << PB1);//OUTPUT SCK
}