/*
* your comment
*/

#include <Arduino.h>
#include <avr/io.h>
#include <ss_oled.h>

#define LED_NUM_MAX 30

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

long int score0 = 0, score1 = 0;
long int set0 = 0, set1 = 0;
int position = LED_NUM_MAX / 2;
int direction = 1;
int start_delay = 400;
int current_delay = start_delay;
bool p0ready = false, p1ready = false;
bool updateDisplay = false;
unsigned int prellZeit = 500;
unsigned int interrupt0Zeit = 0;
unsigned int interrupt1Zeit = 0;


const byte interruptPin0 = 2;
const byte interruptPin1 = 3;

extern "C"
{
  // function prototypes
  void STRIPE_show(uint16_t index, char r, char g, char b, char bright);
  void STRIPE_com(uint8_t one_byte);
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
    initStripe();
    attachInterrupt(digitalPinToInterrupt(2), interrupt_user_0, RISING);
    attachInterrupt(digitalPinToInterrupt(3), interrupt_user_1, RISING);
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
    STRIPE_show(position, 0, 0, 0, 0);//turn LED off
    position += direction; //change position
    STRIPE_show(position, 0, 0, 10, 5); // turn LED on
    
    //if out of bounds
    if(position <= 0)
    {
      scoreUser(0);
    }
    else if (position >= LED_NUM_MAX)
    {
      scoreUser(1);
    }
    
    delay(current_delay);
    
    //flag for updatig display to avoid unnecessary refreshes
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
  //if during game and switch was not activited during the last 500ms
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
  //if during game and switch was not activited during the last 500ms
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
  updateDisplay = true; // update score on display
  
  //trigger buzzer
  digitalWrite(11, HIGH);
  delay(300);
  digitalWrite(11, LOW);

  //handle which user scored
  if(user == 0)
  {
    score0++;
    if(score0 > 9){
      score0 = 0;
      score1 = 0;
      set0++;
    }
  }
  else
  {
    score1++;
    if(score1 > 9){
      score0 = 0;
      score1 = 0;
      set1++;
    }
  }

  STRIPE_show(LED_NUM_MAX - 1, 0, 0, 0, 0); //turn all LEDs off
  current_delay = start_delay; //reset speed
  direction = direction * -1; //change direction to scoring player
  position = LED_NUM_MAX / 2; //LED starts in the middle
  return;
}

//display intro screen
void updateDisplayStart()
{
  oledFill(&ssoled, 0, 1);
  oledWriteString(&ssoled, 0, 0, 0, (char *)"please press", FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 1, (char *)"both buttons", FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 2, (char *)"to start", FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 3, (char *)"the game", FONT_NORMAL, 0, 1);
}

//display game screen
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

//display winning screen
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