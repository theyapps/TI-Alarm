#include <WiFi.h>

// Sonic wave control
#define ECHO 7 // Echo Pin
#define TRIG 18 // Trigger Pin

#define BASELINE_S 20 // How many reading to use to measure our baseline

#define TOLERANCE 0.25 // A number that represents how much deviation we allow.

int ledCount;
int baseline = 0;
int armed = false;
int alarm = false;

// Network name
char ssid[] = "NSA-Probe23";
//char ssid[] = "S6";
// Network password
char password[] = "TG1672G5D1CE2";
//char password[] = "rcb120030";
// your network key Index number (needed only for WEP)
int keyIndex = 0;

WiFiServer server(80);

/**
 *  Setup Function
 */
void setup(){
  Serial.begin(115200);      // initialize serial communication
  
  setupServer();

  /* Initialize pins */
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(GREEN_LED, OUTPUT);      // set the LED pin mode
  pinMode(YELLOW_LED, OUTPUT);      // set the LED pin mode
  pinMode(RED_LED, OUTPUT);      // set the LED pin mode

  pinMode(PUSH1, INPUT_PULLUP);
  pinMode(PUSH1, INPUT);

  pinMode(PUSH2, INPUT_PULLUP); 
  pinMode(PUSH2, INPUT);

  /* Initialize LED states to OFF */
  setLEDs("000");
}

/**
 *  Main Loop
 */
void loop(){
  int sonicRead;
  if(digitalRead(PUSH1) == HIGH){
    setArmed(true);  
  }
  if(digitalRead(PUSH2) == HIGH){
    setArmed(false);
  }

  if(armed && !alarm){
    sonicRead = getMedianReadings(BASELINE_S);
    Serial.print("Median: ");
    Serial.println(sonicRead);
    if(sonicRead < baseline / (1 + TOLERANCE) || sonicRead > baseline * (1 + TOLERANCE)) {
      alarm = true;
      ledCount = 0;
      Serial.println("Alarm has been triggered.");
    }
  }

  if(alarm) {
    if(ledCount == 0){
      setLEDs("100");
      sleep(200);
      ledCount = 1;
    }
    if(ledCount == 1){
      setLEDs("010");
      sleep(200);
      ledCount = 2;
    }
    if(ledCount == 2){
      setLEDs("001");
      sleep(200);
      ledCount = 0;
    }    
  }  
  
  loopServer();
}

/**
*  Server Setup Code
*  TODO: Implement a connection timeout.
*/
void setupServer(){
  /* Initialize WIFI */
  // print the network name (SSID);
  Serial.print("Attempting to connect to Network named: ");
  Serial.println(ssid);
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");

  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  // you're connected now, so print out the status  
  printWifiStatus();

  Serial.println("Starting webserver on port 80");
  server.begin(); // start the web server on port 80
  Serial.println("Webserver started!");
}

/**
*  Server code to run in loop.
*/
void loopServer(){
  int i, j;
  
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    char buffer[150] = {0};                 // make a buffer to hold incoming data
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (strlen(buffer) == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:application/json");
            client.println();

            // the content of the HTTP response follows the header:
            client.println("{");
            
            client.print("\"armed\": ");
            if(armed){
              client.println("true,");
            }
            else {
              client.println("false,");
            }
            
            client.print("\"alarm\": ");
            if(alarm){
              client.println("true");
            }
            else {
              client.println("false");
            }
            
            client.println("}");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear the buffer:
            memset(buffer, 0, 150);
            j = 0;
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          buffer[j++] = c;      // add it to the end of the currentLine
        }

        if(endsWith(buffer, "POST /ARM")) {
          Serial.println("\nWeb client arming system...");
          setArmed(true);
        }
        if(endsWith(buffer, "POST /DISARM")) {
          Serial.println("\nWeb client disarming system...");
          setArmed(false);
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

/**
 *  Get a baseline reading from the sonic sensor
 */
int getAvgReadings(int sz){
  int sonicSum = 0;
  int i;
  for(i = 0; i < sz; i++){
    sonicSum += scan();
  }
  return sonicSum / sz;
}

/**
 *  Get a baseline reading from the sonic sensor
 */
int getMedianReadings(int sz){
  int temp;
  int *arr = new int[sz];
  int i, j;
  for(i = 0; i < sz; i++){
    arr[i] = scan();
  }
  // the following two loops sort the array x in ascending order
  for(i = 0; i < sz-1; i++) {
    for(j = i + 1; j < sz; j++) {
      if(arr[j] < arr[i]) {
        // swap elements
        temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
      }
    }
  }

  if(sz % 2 == 0) {
    // if there is an even number of elements, return mean of the two elements in the middle
    temp = (arr[sz/2] + arr[sz/2 - 1]) / 2;
    delete arr;
    return temp;
  } 
  else {
    // else return the element in the middle
    temp = arr[sz/2];
    delete arr;
    return temp;
  }
}

/**
 *  Arm or Disarm the system
 */
void setArmed(int val){
  int sleepBeforeArming = 5000; // How long to pause before arming in ms
  int i;
  if(val && !armed) {
    Serial.println("Arming countdown started...");

    for(i = 0; i < sleepBeforeArming / 400; i++){
      setLEDs("1--");
      sleep(200);
      setLEDs("0--");
      sleep(200);
    }

    baseline = getMedianReadings(BASELINE_S);
    Serial.print("Baseline set: ");
    Serial.println(baseline);
    armed = true;
    digitalWrite(GREEN_LED, HIGH);

    Serial.println("System Armed!");
  }
  else if(!val && armed) {
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
  else if(flags[0] == '0'){
    digitalWrite(GREEN_LED, LOW);
  }
  // Yellow
  if(flags[1] == '1'){
    digitalWrite(YELLOW_LED, HIGH);
  }
  else if(flags[1] == '0'){
    digitalWrite(YELLOW_LED, LOW);
  }
  // Red
  if(flags[2] == '1'){
    digitalWrite(RED_LED, HIGH);
  }
  else if(flags[2] == '0'){
    digitalWrite(RED_LED, LOW);
  }
}

/**
 *  a way to check if one array ends with another array
 */
boolean endsWith(char* inString, char* compString) {
  int compLength = strlen(compString);
  int strLength = strlen(inString);

  //compare the last "compLength" values of the inString
  int i;
  for (i = 0; i < compLength; i++) {
    char a = inString[(strLength - 1) - i];
    char b = compString[(compLength - 1) - i];
    if (a != b) {
      return false;
    }
  }
  return true;
}

/**
 *  Print details about the wifi connection.
 */
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("Network Name: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

