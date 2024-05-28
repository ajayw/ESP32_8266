#include    <Servo.h>

#define servo_PIN    9

Servo myservo;

int pos = 0;

void setup()
{
    myservo.attach(servo_PIN);
    myservo.write(90);
}

void loop()
{
    for (pos = 170; pos >= 10; pos -= 1) 
    { 
        myservo.write(pos);    
        delay(15);
    } 
    for (pos = 10; pos <= 170; pos += 1) 
    { 
        myservo.write(pos);            
        delay(15);                     
    }
}
