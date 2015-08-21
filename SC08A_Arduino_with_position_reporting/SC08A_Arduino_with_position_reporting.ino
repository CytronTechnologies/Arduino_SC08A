const char tx = 1;  //assign tx with pin 1 (transmit)
const char rx = 0;  //assign rx with pin 0 (receive)
const char led_red = 13;  // assign led_red with pin 13 (there is an on-board red led)
const char button = 12;  //assign button with pin 12

//Setup
void setup()
{
  Serial.begin(9600); //initialize serial communication for UART with baudrate 9.6kbps
  pinMode(button,INPUT); // set button (or pin 12) as input
  pinMode(rx,INPUT);  //set rx (or pin 0) as input
  pinMode(tx, OUTPUT);  //set tx (or pin 1) as output
  pinMode(led_red, OUTPUT);  // set led_red (or pin 12) as output

  on_off_motor(0,1);  //activate all servo motor channels on SC08A
 
  while(digitalRead(button)); //waiting button to be pressed
  while(!digitalRead(button)); //waiting button to be released
}

//Main loop
void loop()  
{
  for(int count = 0; count<8; count++)  //set 1st position, speed for all servo motor channels (servo motors), assignable speed value (1-100)
  {                                     //In position reporting operation, setting speed = 0 will cause the servo motor oscillates due to limitation of software position feedback
    set_ch_pos_spd(count+1, 7300, 50);  //set position = 7000, speed = 50
  }
  digitalWrite(led_red,HIGH);  //led is on (indicaton) 
  while(rd_current_pos(1) < 7200);  //wait untill the (position-100) is reached
  
  for(int count = 0; count<8; count++)  //set 2nd position, speed for all servo motor channels (servo motors), assignable speed value (1-100)
  {                                    //In position reporting operation, setting speed = 0 will cause the servo motor oscillates due to limitation of software position feedback
    set_ch_pos_spd(count+1, 400, 50); //set position = 4000, speed = 50
  }
  
  digitalWrite(led_red,LOW);  //led is off (indicaton)
  while(rd_current_pos(1) > 500); //wait untill the (position + 100) is reached
}

void on_off_motor(unsigned char channel, unsigned char on)
{
 /*****Activate servo channel command*****
 - 2 bytes involved
 - 1st byte: Mode + Servo motor channel
   eg:
       Ob 1 1 0 x x x x x;
         |Mode |Servo channel
 -the 3 MSB bits (110) is mode to activate servo channels
 -the last xxxxx can be assigned with value 0-16, where
  0 ---> activate all channels
  1 ---> activate channel 1
  2 ---> activate channel 2
  ....
  ....
  16 ---> activate channel 16
 - 2nd byte: On/off
   eg:
       0b 0 0 0 0 0 0 0 x
   x = 1 --> on selected and activated channel/channels
   x = 0 --> off selected or activated channel/channels
 ****************************************/
 
   unsigned char first_byte = 0;
   first_byte = 0b11000000 | channel; //make up 1st byte
   Serial.write(first_byte); //send 1st byte use UART
   Serial.write(on); //send 2nd byte use UART
}

void set_ch_pos_spd(unsigned char channel, unsigned int position, unsigned char velocity)
{
 /*****Position and Speed Command*****
 - 4 bytes involved
 - 1st byte: Mode + Servo motor channel
   eg: 
     0b 1 1 1 x x x x x 
        Mode |Servo channel
 - 3 MSBs (111) is mode for position and speed command
 - the last xxxxx can be assigned with value 1-16, where
   1 ---> select channel 1
   2 ---> select channel 2
   ....
   ....
   16 ---> select channel 16
 - 2nd byte: Position (High byte) higher 7-bit  
   eg: 
     0b 0 x x x x x x x
 - the last xxxxxxx can be assigned with value 0 - 127
 - 3nd byte: Position (Low byte) lower 6-bit
   eg: 
     0b 0 0 x x x x x x
 - the last xxxxxx can be assigned with value 0 - 63
   **2nd byte and 3byte is the position value when combined together into 13 bits position
 - 4th byte: Speed (0-100)
   eg:
     0b 0 x x x x x x x
 - the last xxxxxx can be assigned with value 0 - 100
 ************************************/
   unsigned char first_byte = 0;
   unsigned char high_byte = 0;
   unsigned char low_byte = 0;
   first_byte = 0b11100000 | channel; //make up the 1st byte
   high_byte = (position >> 6) & 0b01111111; //obtain the high byte of 13 bits position value
   low_byte = position & 0b00111111; //obtain the low byte of 13 bits position value
   Serial.write(first_byte); //send the 1st byte
   Serial.write(high_byte); //send the 2nd byte
   Serial.write(low_byte); //send the 3rd byte
   Serial.write(velocity); // send the 4th byte
}

unsigned int rd_current_pos(unsigned char channel)
{
 /*****Servo position reporting Command*****
 - 3 bytes involved (send 1 byte, received 2 bytes)
 - 1st byte: Mode + Servo motor channel (Send)
   eg:
     0b 1 0 1 x x x x x 
        Mode |Servo channel
 - 3 MSBs (101) is mode for position reporting command
 - the last xxxxx can be assigned with value 1-16, where
   1 ---> select channel 1
   2 ---> select channel 2
   ....
   ....
   16 ---> select channel 16
 - 2nd byte: Position (High byte) higher 7-bit (1st received byte) 
   eg:
     0b 0 x x x x x x x 
 - Received value vary between 0 - 127
 - 3nd byte: Position (Low byte) lower 6-bit (2nd received byte)
   eg:
     0b 0 0 x x x x x x 
 - Received value vary between 0 - 63
 **2nd byte and 3byte is the position value when combined together into 13 bits
 ******************************************/
   unsigned char first_byte = 0;
   unsigned char high_byte = 0;
   unsigned char low_byte = 0;
   unsigned int reading_position = 0;
   unsigned char buffer[2];
 
   first_byte =  0b10100000 | channel; //make up the 1st byte
   Serial.write(first_byte); //send the 1st byte
   delay(200); //short delay for sending the data 
 
   while(!Serial.available()); //wait untill data received
   for(int count = 0; count<2; count++) //store two bytes received in an array
   {
     buffer[count] = Serial.read();
   }
 
 //combine received high byte & low byte into 13 bits position value
   high_byte = buffer[0];
   low_byte = buffer[1];
   reading_position = high_byte;
   reading_position = high_byte<<6;
   reading_position = reading_position | low_byte;
   return reading_position; //return position value to the main
}

void initial_position(unsigned char channel, unsigned int position) //optional, if used, the RX pin of Arduino Mainboard should be connected to TX pin of SC08A
{
/*****Servo starting position Command*****\
- 3 bytes involved
- 1st byte: Mode + Servo motor channel
  eg: 
    0b 1 0 0 x x x x x 
       Mode |Servo channel
- 3 MSBs (111) is mode for position and speed command
- the last xxxxx can be assigned with value 1-16, where
   1 ---> select channel 1
   2 ---> select channel 2
   ....
   ....
   16 ---> select channel 16
- 2nd byte: Position (High byte) higher 7-bit  
  eg: 
    0b 0 x x x x x x x
- the last xxxxxxx can be assigned with value 0 - 127
- 3nd byte: Position (Low byte) lower 6-bit
  eg: 
    0b 0 0 x x x x x x
- the last xxxxxx can be assigned with value 0 - 63
**2nd byte and 3byte is the position value when combined together into 13 bits
*****************************************/
  unsigned char first_byte = 0;
  unsigned char high_byte = 0;
  unsigned char low_byte = 0;
  first_byte = 0b10000000 | channel; //make up the 1st byte
  high_byte = (position >> 6) & 0b01111111; //make up the high byte
  low_byte = position & 0b00111111; //make up the low byte
  Serial.write(first_byte); //send the 1st byte
  Serial.write(high_byte); //send the 2nd byte
  Serial.write(low_byte); //send the 3rd byte
  delay(100); //short delay for sending the data
  while(!Serial.available()); //wait untill data received 
  while(Serial.read() != 0x04); //wait untill value 0x40 is received for indication
}


