// Sonic wave control
#define ECHO 7 // Echo Pin
#define TRIG 18 // Trigger Pin

#define BUFFER_S 10

#define TOLERANCE 0.5 // A number that represents how much deviation we allow.

int i;
int baseline = 0;
int sonicSum;
int armed = false;
int alarm = false;
int sonicRead = 0;

/**
*  Setup Function
*/ 
void setup(){
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  Serial.begin(115200);      // initialize serial communication
  
  pinMode(GREEN_LED, OUTPUT);      // set the LED pin mode
  pinMode(YELLOW_LED, OUTPUT);      // set the LED pin mode
  pinMode(RED_LED, OUTPUT);      // set the LED pin mode
  
  pinMode(PUSH1, INPUT_PULLUP);
  pinMode(PUSH1, INPUT);
  
  pinMode(PUSH2, INPUT_PULLUP); 
  pinMode(PUSH2, INPUT);

  setLEDs("000");
}

/**
*  Main Loop
*/
void loop(){
  if(digitalRead(PUSH1) == HIGH){
    setArmed(true);  
  }
  if(digitalRead(PUSH2) == HIGH){
    setArmed(false);
  }
  
  if(armed && !alarm){
    sonicRead = getBaseline();
    if(sonicRead < baseline / (1 + TOLERANCE) || sonicRead > baseline * (1 + TOLERANCE)) {
      alarm = true;
      i = 0;
      Serial.println("Alarm has been triggered.");
    }
  }
  
  if(alarm) {
    if(i == 0){
      setLEDs("100");
      sleep(200);
      i = 1;
    }
    if(i == 1){
      setLEDs("010");
      sleep(200);
      i = 2;
    }
    if(i == 2){
      setLEDs("001");
      sleep(200);
      i = 0;
    }    
  }  
}

/**
*  Get a baseline reading from the sonic sensor
*/
int getBaseline(){
  sonicSum = 0;
  for(i = 0; i < BUFFER_S; i++){
    sonicSum += scan();
  }
  return sonicSum / BUFFER_S;
}

/**
*  Arm or Disarm the system
*/
void setArmed(int val){
  int sleepBeforeArming = 5000; // How long to pause before arming in ms
  if(val && !armed) {
    Serial.println("Arming countdown started...");
    
    for(i = 0; i < sleepBeforeArming / 400; i++){
      setLEDs("1--");
      sleep(200);
      setLEDs("0--");
      sleep(200);
    }
    
    baseline = getBaseline();
    armed = true;
    digitalWrite(GREEN_LED, HIGH);
    
    Serial.println("System Armed!");
  }
  else if(armed) {
    baseline = 0;
    armed = false;
    alarm = false;
    setLEDs("000");
    
    Serial.println("System Disarmed!");
  }
}

/**
*  Read a value from the sonic sensor
*/
int scan(){
  long duration, distance; // Duration used to calculate distance

  /* The following trigPin/echoPin cycle is used to determine the
  distance of the nearest object by bouncing soundwaves off of it. */
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  duration = pulseIn(ECHO, HIGH);
 //Calculate the distance (in cm) based on the speed of sound.
  distance = duration/58.2;
  return distance;
}

/**
*  Uses a mask to set all three LED's
*  The mask is three chars where the first is green second is yellow and third is red
*  1 -> Turn on
*  0 -> Turn off
*  - -> Don't change
*/
int setLEDs(char* flags){
  // Green
  if(flags[0] == '1'){
    digitalWrite(GREEN_LED, HIGH);
  }
  if(flags[0] == '0'){
    digitalWrite(GREEN_LED, LOW);
  }
  // Yellow
  if(flags[1] == '1'){
    digitalWrite(YELLOW_LED, HIGH);
  }
  if(flags[1] == '0'){
    digitalWrite(YELLOW_LED, LOW);
  }
  // Red
  if(flags[2] == '1'){
    digitalWrite(RED_LED, HIGH);
  }
  if(flags[2] == '0'){
    digitalWrite(RED_LED, LOW);
  }
}
