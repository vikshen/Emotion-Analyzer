const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

int inputPin = A0;

void setup() {
  // initialize serial communication with computer:
  Serial.begin(9600);
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}

void loop() {
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(inputPin);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  //Serial.println(average);
  //delay(3000);
  // send it to the computer as ASCII digits
  //Serial.println(average);
  if((average > 470)&(average < 500))
  {
    Serial.println("Approx 72 to 76 bpm- Normal and Calm");
  delay(2000);
  }
if((average > 500) &(average <509))
{
  Serial.println("Approx 76 to 80 bmp - Normal and Happy");
  delay(2000);
}
if((average > 510) & (average < 525))
{
  Serial.println("Approx 80 to 85 bmp - Normal and excited");
  delay(2000);
}
if((average>550)&(average<635))
{
  Serial.println("Approx 100 bmp - Excess heart rate exhausted");
  delay(2000);
}
//Serial.println(average);
//delay in between reads for stability
if(average>635)
{
  Serial.println("Please attach sensor properly");
  delay(4000);
  Serial.println("Caliberating");
  delay(2000);
}
if(average<469)
{
  Serial.println("Approx 60 to 70 bpm -Calm and Resting ");
  delay(2000);
}
}
