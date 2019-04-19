#include <SoftwareSerial.h>
#include "RoboClaw.h"

SoftwareSerial serial(10,11);  
RoboClaw roboclaw(&serial,10000);

#define address 0x80

int display = 1;  

int distance = 23000; //initialize your distance variable here. and down in the motor section

const int clkPin= 6; //the clk attach to pin 2
const int dtPin= 7; //the dt pin attach to pin 3
int encoderVal = 4;
int menu_pos;
int last_menu;
int encoder_button = 5;
bool tracking_state = true;
int direction_state = 1;
int temp;


char x[1];
char y2[1];

uint16_t y_pos;
uint16_t x_pos;
    
uint16_t last_x;
uint16_t last_y;

int velocity = 300;
int objectX=50;
int objectY=50;
int currentX =0;
int angle;
float encoderMM = 33.8164;//32
int enc1;
double servopos;
int initial_servopos;



#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ 6, /* data=*/ 5, /* CS=*/ 4, /* reset=*/ 7);
//U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI 
//U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_FAST); // Dev 0, Fast I2C / TWI
//U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NO_ACK);

int speedMenu = 5;

void setup(void) {
  u8g2.begin();

  
pinMode(clkPin, INPUT);
pinMode(dtPin, INPUT);
pinMode(encoder_button, INPUT);
pinMode(8, INPUT);
roboclaw.begin(38400);
Serial.begin(9600); // initialize serial communications at 9600 bps

  roboclaw.SetM1VelocityPID(0x80,1.746,0.25225,0,10500);
  roboclaw.SetM1PositionPID(0x80, 2.5, 0, 200, 0, 0, -10000, 10000);

 // roboclaw.SetM1PositionPID(0x80, 20, 1, 300, 50, 0, -10000, 10000);
  roboclaw.SetM2VelocityPID(0x80,1.746,0.25225,0,6000);
  roboclaw.SetM2PositionPID(0x80, 30, 1.01, 305, 60,0, -100000, 100000); // p,i,d,max i, deadband,min,max

  u8g2.firstPage();   
      do 
      {  
      draw();  
      } 
      while( u8g2.nextPage() );  

      menu_pos = 0;
      last_menu = 0;
}



void loop(void) {
  int change = getEncoderTurn();//
  encoderVal = encoderVal + change;
  menu_pos = last_menu + change;
  if (digitalRead(8) == 1)
  {
    motor();
  }
  
  if (menu_pos != last_menu &&(menu_pos <4 && menu_pos>=0))
    {
      u8g2.firstPage();   
      do 
      {  
      draw();  
      } 
      while( u8g2.nextPage() );  
      last_menu = menu_pos;
    }

  if (digitalRead(encoder_button) == 0)
    {
      while (digitalRead(encoder_button) == 0)
      {}
      delay(100);
      if (menu_pos == 5)
      {
        motor();
      }
      else
        {
          config_menu();
        }
      while (digitalRead(encoder_button) == 0)
      {}
    }
}

int getEncoderTurn(void)
{
  static int oldA = HIGH; //set the oldA as HIGH
  static int oldB = HIGH; //set the oldB as HIGH
  int result = 0;
  int newA = digitalRead(clkPin);//read the value of clkPin to newA
  int newB = digitalRead(dtPin);//read the value of dtPin to newB
  if (newA != oldA || newB != oldB) //if the value of clkPin or the dtPin has changed
  {
  // something has changed
    if (oldA == HIGH && newA == LOW)
    {
      result = (oldB * 2 - 1);
    }
  }
  oldA = newA;
  oldB = newB;
  return result;
}

void Speed(void) {
  u8g2.setFont(u8g2_font_6x10_tf);

  u8g2.setColorIndex(1);

  u8g2.drawBox(0,0,128,17); // speed
  //u8g2.drawFrame(0,0,128,20); // speed
  u8g2.drawFrame(0,22,40,26); //track config x
  u8g2.drawFrame(44,22,40,26); // track config y
  u8g2.drawFrame(88,22,40,26); // track config on/off
  u8g2.drawFrame(0,50,128,14); // stats
  
  
  u8g2.drawStr(6, 31, "Trk X");
  u8g2.drawStr(49, 31, "Trk Y");
  u8g2.drawStr(93, 31, "Track");
  u8g2.drawStr(86, 60, "Dir");

  int width = map(speedMenu,0,10,0,128);
  u8g2.drawBox(0,17,width,3);

  u8g2.setColorIndex(0);
  u8g2.drawStr(25, 10, "Speed:");
  char bus[1];
  sprintf (bus, "%d", (speedMenu*velocity));
  u8g2.drawStr(65, 10, bus);
  u8g2.setColorIndex(1);
  
  u8g2.setFont(u8g2_font_4x6_tf);  

  if (tracking_state == true)
  {u8g2.drawStr(103, 39, "On");}
  else
  {u8g2.drawStr(104, 39, "Off");} 
  
  if (direction_state == 1)
  {u8g2.drawStr(110, 60, "->");}
  else
  {u8g2.drawStr(110, 60, "<-");}
  
  char buf[1];
  sprintf (buf, "%d", objectX);
  u8g2.drawStr(17, 39, buf);

  char but[1];
  sprintf (but, "%d", objectY);
  u8g2.drawStr(61, 39, but);
}

void trackX() {
u8g2.setFont(u8g2_font_6x10_tf);

  u8g2.setColorIndex(1);

  //u8g2.drawBox(0,0,128,17); // speed
  u8g2.drawFrame(0,0,128,20); // speed
  u8g2.drawBox(0,22,40,26); //track config x
  u8g2.drawFrame(44,22,40,26); // track config y
  u8g2.drawFrame(88,22,40,26); // track config on/off
  u8g2.drawFrame(0,50,128,14); // stats
  
  u8g2.setColorIndex(1);
  
  
  u8g2.drawStr(49, 31, "Trk Y");
  u8g2.drawStr(93, 31, "Track");
  u8g2.drawStr(86, 60, "Dir");
  u8g2.drawStr(25, 10, "Speed:");
  char bus[1];
  sprintf (bus, "%d", (speedMenu*velocity));
  u8g2.drawStr(65, 10, bus);

  int width = map(speedMenu,0,10,0,128);
  u8g2.drawBox(0,17,width,3);

  u8g2.setColorIndex(0);
  u8g2.drawStr(6, 31, "Trk X");
  char buf[1];
  sprintf (buf, "%d", objectX);
  u8g2.drawStr(17, 39, buf);
  u8g2.setColorIndex(1);
  
  u8g2.setFont(u8g2_font_4x6_tf);  

  if (tracking_state == true)
  {u8g2.drawStr(103, 39, "On");}
  else
  {u8g2.drawStr(104, 39, "Off");} 
  
  if (direction_state == 1)
  {u8g2.drawStr(110, 60, "->");}
  else
  {u8g2.drawStr(110, 60, "<-");}
  
  char but[1];
  sprintf (but, "%d", objectY);
  u8g2.drawStr(61, 39, but);

}
void trackY() {
  u8g2.setFont(u8g2_font_6x10_tf);

  u8g2.setColorIndex(1);

  //u8g2.drawBox(0,0,128,17); // speed
  u8g2.drawFrame(0,0,128,20); // speed
  u8g2.drawFrame(0,22,40,26); //track config x
  u8g2.drawBox(44,22,40,26); // track config y
  u8g2.drawFrame(88,22,40,26); // track config on/off
  u8g2.drawFrame(0,50,128,14); // stats
  
  u8g2.setColorIndex(1);
  
  
  u8g2.drawStr(6, 31, "Trk X");
  u8g2.drawStr(93, 31, "Track");
  u8g2.drawStr(86, 60, "Dir");
  u8g2.drawStr(25, 10, "Speed:");
  char bus[1];
  sprintf (bus, "%d", (speedMenu*velocity));
  u8g2.drawStr(65, 10, bus);

  int width = map(speedMenu,0,10,0,128);
  u8g2.drawBox(0,17,width,3);

  u8g2.setColorIndex(0);
  u8g2.drawStr(49, 31, "Trk Y");
  char but[1];
  sprintf (but, "%d", objectY);
  u8g2.drawStr(61, 39, but);
  u8g2.setColorIndex(1);
  
  u8g2.setFont(u8g2_font_4x6_tf);  

  if (tracking_state == true)
  {u8g2.drawStr(103, 39, "On");}
  else
  {u8g2.drawStr(104, 39, "Off");} 
  
  if (direction_state == 1)
  {u8g2.drawStr(110, 60, "->");}
  else
  {u8g2.drawStr(110, 60, "<-");}
  
  char buf[1];
  sprintf (buf, "%d", objectX);
  u8g2.drawStr(17, 39, buf);
}

void Track_enabled() {

  u8g2.setFont(u8g2_font_6x10_tf);

  u8g2.setColorIndex(1);

  //u8g2.drawBox(0,0,128,17); // speed
  u8g2.drawFrame(0,0,128,20); // speed
  u8g2.drawFrame(0,22,40,26); //track config x
  u8g2.drawFrame(44,22,40,26); // track config y
  u8g2.drawBox(88,22,40,26); // track config on/off
  u8g2.drawFrame(0,50,128,14); // stats
  
  u8g2.setColorIndex(1);
  
  u8g2.drawStr(6, 31, "Trk X");
  u8g2.drawStr(49, 31, "Trk Y");
  u8g2.drawStr(86, 60, "Dir");
  u8g2.drawStr(25, 10, "Speed:");
  char bus[1];
  sprintf (bus, "%d", (speedMenu*velocity));
  u8g2.drawStr(65, 10, bus);

  int width = map(speedMenu,0,10,0,128);
  u8g2.drawBox(0,17,width,3);

  u8g2.setColorIndex(0);
  u8g2.drawStr(93, 31, "Track");
  if (tracking_state == true)
  {u8g2.drawStr(103, 39, "On");}
  else
  {u8g2.drawStr(104, 39, "Off");} 
  u8g2.setColorIndex(1);
  
  u8g2.setFont(u8g2_font_4x6_tf);  

  if (direction_state == 1)
  {u8g2.drawStr(110, 60, "->");}
  else
  {u8g2.drawStr(110, 60, "<-");}
  
  char buf[1];
  sprintf (buf, "%d", objectX);
  u8g2.drawStr(17, 39, buf);
  
  char but[1];
  sprintf (but, "%d", objectY);
  u8g2.drawStr(61, 39, but);

}

void draw()
{
  switch(menu_pos) //Carrega a tela correspondente  
  {
   case 0:
    Speed();  
    break;  
   case 1:  
    trackX();  
    break;  
   case 2:  
    trackY();  
    break;  
   case 3:  
    Track_enabled();  
    break;  
   }  
}

void config_menu()
{
  switch(menu_pos) //Carrega a tela correspondente  
  {
   case 0:  
      while(digitalRead(encoder_button) == 1)
      {
        int turn = getEncoderTurn();
        speedMenu = speedMenu + turn;
        if (turn != 0)
        {
          u8g2.firstPage();   
          do 
          {Speed();} 
          while( u8g2.nextPage() );
        }
      }  
      delay(400);
   break;  
   
   case 1:
      while(digitalRead(encoder_button) == 1)
      {
        int turn = getEncoderTurn();
        objectX = objectX + turn;
        if (turn != 0)
        {
          u8g2.firstPage();   
          do 
          {trackX();} 
          while( u8g2.nextPage() );
        }
      }
      delay(400);
   break;  
   
   case 2:  
      while(digitalRead(encoder_button) == 1)
      {
        int turn = getEncoderTurn();
        turn = turn * 3;
        objectY = objectY + turn;
        if (turn != 0)
        {
          u8g2.firstPage();   
          do 
          {trackY();} 
          while( u8g2.nextPage() );
        }
      }
      delay(400);
   break;
   
   case 3:  
      if (tracking_state == true)
      {
        tracking_state = false;
      }
      else
      {
        tracking_state = true;
      }
      u8g2.firstPage();   
      do 
      {Track_enabled();} 
      while( u8g2.nextPage() ); 
    break;  
  } 
}

void motor()
{
  uint8_t status1;
  bool valid1;
  uint8_t depth1,depth2;
  
//--------------
    if (direction_state == 1)
    {
      distance = 23000;// make sure to change distance here too
      temp = (objectX*10*encoderMM)-enc1;
    }
    else if(direction_state == -1)
    {
      distance = 0;
      temp = (objectX*10*encoderMM)-enc1;//5000 is temp midpoint. replace this with objectX
    }
    
    Serial.print("distance");
    Serial.println(distance);

    enc1 = roboclaw.ReadEncM2(address, &status1, &valid1);
    //Serial.println(temp);
    double angle1 = atan2 ((objectY*10*encoderMM), temp); 
    angle1 =  (angle1 * 180)/3.14159265;
    initial_servopos = map(angle1, 180, 0, 0, 6000);
    Serial.print("initial servopos ");
    Serial.println(initial_servopos);

    
    roboclaw.SpeedAccelDeccelPositionM1(address,10000,2000,10000,initial_servopos,1);
    
    delay (3000);


    int runVelocity = speedMenu*velocity;
    
    if (direction_state == 1)
    {
        roboclaw.SpeedAccelDistanceM2(address,130000,runVelocity,23000,1);
    }
    else if(direction_state == -1)
    {
        runVelocity = runVelocity*-1;
        roboclaw.SpeedDistanceM2(address,runVelocity,23000,1);
    }

      enc1 = roboclaw.ReadEncM2(address, &status1, &valid1);
      if (direction_state == 1 && tracking_state == true)
      {
          while(enc1<distance)
          {
              enc1 = roboclaw.ReadEncM2(address, &status1, &valid1);
              if (enc1<22900)
              {
                    temp = (objectX*10*encoderMM)-roboclaw.ReadEncM2(address, &status1, &valid1);//5000 is temp midpoint. replace this with objectX
                    angle1 = atan2 ((objectY*10*encoderMM), temp);  
                    angle1 =  (angle1 * 180)/3.14159265;
                    servopos = map(angle1, 180, 0, 0, 6000);
                  
                    //if (servopos!=initial_servopos)
                    //{
                      roboclaw.SpeedAccelDeccelPositionM1(address,10000,10000,10000,servopos,1);
                      Serial.println(servopos);
                    //}
              }
          }
      }

      else if (direction_state == -1 && tracking_state == true)
      {
          while(enc1>100)
          {
              enc1 = roboclaw.ReadEncM2(address, &status1, &valid1);
              if (enc1>distance)
              {
                    temp = (objectX*10*encoderMM)-roboclaw.ReadEncM2(address, &status1, &valid1);//5000 is temp midpoint. replace this with objectX
                    angle1 = atan2 ((objectY*10*encoderMM), temp);
                    angle1 =  (angle1 * 180)/3.14159265;
                    servopos = map(angle1, 180, 0, 0, 6000);
                  
                    //if (servopos!=initial_servopos)
                    //{
                      roboclaw.SpeedAccelDeccelPositionM1(address,10000,10000,10000,servopos,1);
                      Serial.println(servopos);
                    //}
              }
          }
      }
      else if (tracking_state == false)
      {       
        roboclaw.SpeedAccelDeccelPositionM1(address,10000,5000,10000,3000,1);
      }



  direction_state = direction_state*-1;
  delay(2000);
  roboclaw.SpeedAccelDeccelPositionM1(address,10000,4000,10000,0,1);
  delay(2000);
}

