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
int current_delay = 400;
bool p0ready = false;
bool p1ready = false;


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


void interrupt_user_0(void);
void interrupt_user_1(void);



/*void setup()
{
  initStripe();
}*/
void setup()
{
    initStripe();
    attachInterrupt(digitalPinToInterrupt(2), interrupt_user_0, RISING);
    attachInterrupt(digitalPinToInterrupt(3), interrupt_user_1, RISING);
    int rc;
    rc = oledInit(&ssoled, OLED_128x64, OLED_ADDR, FLIP180, INVERT, USE_HW_I2C, SDA_PIN, SCL_PIN, RESET_PIN, 400000L);       // Standard HW I2C bus at 400Khz

    if (rc != OLED_NOT_FOUND)
    {
        char *msgs[] =
        {
          (char *)"SSD1306 @ 0x3C",
          (char *)"SSD1306 @ 0x3D",
          (char *)"SH1106 @ 0x3C",
          (char *)"SH1106 @ 0x3D"
        };

        oledFill(&ssoled, 0, 1);
        oledWriteString(&ssoled, 0, 0, 0, (char *)"OLED found:", FONT_NORMAL, 0, 1);
        oledWriteString(&ssoled, 0, 10, 2, msgs[rc], FONT_NORMAL, 0, 1);
        delay(3000);


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
    }
}
/*
void loop()
{
  //LED-ON
  //Start-Frame
  STRIPE_com(0);
  STRIPE_com(0);
  STRIPE_com(0);
  STRIPE_com(0);
  //LED-Frame
  STRIPE_com(0xE5); //Brigtness 5
  STRIPE_com(0x0A); //Blue
  STRIPE_com(0);    //Green
  STRIPE_com(0);    //Red
  //END-Frame
  STRIPE_com(0);
  delay(500);
  //LED-OFF
  //Start-Frame
  STRIPE_com(0);
  STRIPE_com(0);
  STRIPE_com(0);
  STRIPE_com(0);
  //LED-Frame
  STRIPE_com(0xE0); //Brigtness 0
  STRIPE_com(0);    //Blue
  STRIPE_com(0);    //Green
  STRIPE_com(0);    //Red
  //END-Frame
  STRIPE_com(0);
  delay(500);
}*/
/*
void loop()
{
  //LED - ON
  STRIPE_show(LED_NUM_MAX - 1, 0, 0, 10, 5);
  STRIPE_show(0, 0, 0, 10, 5);
  delay(500);
  //LED - OFF
  STRIPE_show(LED_NUM_MAX, 0, 0, 0, 0);
  delay(500);
}*/

void loop()
{
  if(p0ready && p1ready)
  {  
    STRIPE_show(position, 0, 0, 0, 0);//turn LED off
    position += direction; //change position
    STRIPE_show(position, 0, 0, 10, 5); // turn LED on
    //STRIPE_show(0, 0, 0, 10, 5);
    if(position < 0)
    {
      scoreUser(0);
    }
    else if (position >= LED_NUM_MAX)
    {
      scoreUser(1);
    }
    
    delay(current_delay);
    //LED - OFF
    //STRIPE_show(LED_NUM_MAX, 0, 0, 0, 0);
    //delay(500);
    //TODO: nicht mehr aus interrupt raus, wenn in Interrupt display gesetzt wird?
    //scoreUser(0);
    if(set0 < 2 && set1 < 2)
    {
      updateDisplayGame();
    }
    else{
      updateDisplayWinner();
    }
  }
  else 
  {
    updateDisplayStart();
  }
}

void interrupt_user_0()
{
  if(p0ready && p1ready)
  {
    if(position == LED_NUM_MAX - 1 || position == LED_NUM_MAX - 2)
    {
      direction = direction * -1;
      current_delay -= 50;
    }
    else
    {
      scoreUser(1);
    }
  }
  p0ready = true;
}

void interrupt_user_1()
{
  if(p0ready && p1ready)
  {
    if(position == 0 || position == 1)
    {
      direction = direction * -1;
      current_delay -= 50;
    }
    else
    {
      scoreUser(0);
    }
  }
  p1ready = true;
}

void scoreUser(int user)
{
  digitalWrite(11, HIGH);
  delay(300);
  digitalWrite(11, LOW);
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

void updateDisplayStart()
{
  //TODO: show status for each player (ready / not ready)
  oledFill(&ssoled, 0, 1);
  oledWriteString(&ssoled, 0, 0, 0, (char *)"please press", FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 1, (char *)"any button", FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 2, (char *)"to start", FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 3, (char *)"the game", FONT_NORMAL, 0, 1);
}

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

void updateDisplayWinner()
{
  char * winner = (set0 >= 2) ? (char *) "Player 1" : (char *) "Player 2";
  oledFill(&ssoled, 0, 1);
  oledWriteString(&ssoled, 0, 16, 2,(char *)"Winner:", FONT_NORMAL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 4, winner, FONT_STRETCHED, 0, 1);
  p0ready, p1ready = false;
}

void initStripe()
{
  pinMode(51, OUTPUT); //  DDRB |= (1 << PB2);//OUTPUT MOSI
  pinMode(52, OUTPUT); //  DDRB |= (1 << PB1);//OUTPUT SCK
}