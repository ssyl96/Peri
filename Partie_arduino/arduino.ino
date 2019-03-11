/*
  ReadAnalogVoltage
  Reads an analog input on pin 0, converts it to voltage, and prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
 
 This example code is in the public domain.
 */
int val = 1;
int new_val = 1;
// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  pinMode(13,OUTPUT);
  val = analogRead(A1); // initialize with the current light 
}

// the loop routine runs over and over again forever:
void loop() {
  new_val = analogRead(A1);  // read the analog in value on gpio A1
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  // print the results to the Serial Monitor:
  Serial.println(new_val);
  if (new_val <val){ 
    digitalWrite(13,LOW); // sets the digital pin 13 on
  }
  if (new_val >val)
  {
    digitalWrite(13,HIGH);
    Serial.println("reçoit un flash");
    
  }
  
  delay(1000);
}
