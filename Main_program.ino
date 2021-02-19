/* ---include needed lbraries--- */
#include <LiquidCrystal_I2C.h> //Library used for LCD display
#include <Wire.h> //library used to enable I2C for the lcd
#include <OneWire.h> //Library used to interface with the temperature sensor
#include <DallasTemperature.h> //Library used to retrieve and interpret data from library
#include "sensorInterpretation.h" //custom area sensor library header file 

/* ---General Sensors marked with a suffix of "_g"--- */
const byte firePin_g[2] = {2,3}; //Fire sensors using interrupt pins 2,3 so it can halt the system
const byte rainPin_g = A0; //rain sensor uses first analog pin
#define ONE_WIRE_BUS 31 //Define Temperature sensor pin
OneWire oneWire(ONE_WIRE_BUS); //Setup one wire instance to communicate with the temperature sensor
DallasTemperature sensors(&oneWire); //Pass one wire reference to Dallas temperature library
const byte modSwitch_g[5] = {50,49,48,47,46}; //Temp 1 =  50 , Temp 2 = 49 , Rain = 48 ,Fire 1 = 47 ,Fire 2 = 46

/* ---Area 1 Sensors marked with suffix of "_a1"--- */
const byte moistPin_a1[2] = {A3,A4}; //Moisture pins for area 1
const byte lightPin_a1 = A11; //Light Dependant Resistor for area 1
const byte areaSwitch_a1 = 4; //Overall Area switch for area 1
const byte modSwitch_a1[3] = {53,52,51}; //Sensor enabling switches
const byte relay_a1 = 26;//Relay pin for Area 1

/* ---Area 2 Sensors marked with suffix of "_a2"--- */
const byte moistPin_a2[2] = {A5,A6};
const byte lightPin_a2 = A12;
const byte areaSwitch_a2 = 5; 
const byte modSwitch_a2[3] = {45,44,43};
const byte relay_a2 = 27;

/* ---Area 3 Sensors marked with suffix of "_a3"--- */
const byte moistPin_a3[2] = {A7,A8};
const byte lightPin_a3 = A13;
const byte areaSwitch_a3 = 6;
const byte modSwitch_a3[3] = {42,41,40};
const byte relay_a3 = 28;

/* ---Area 4 Sensors marked with suffix of "_a4"--- */
const byte moistPin_a4[2] = {A9,A10};
const byte lightPin_a4 = A14;
const byte areaSwitch_a4 = 7;
const byte modSwitch_a4[3] = {39,38,37};
const byte relay_a4 = 29;

const byte relays[4] = {26 ,27 ,28 ,29}; //an array to simply interface with al the relays

const byte rotary[3] = {19,18,17}; //rotary encoder pins: 19 = button , 18 = Clock, 17 = Data
byte screen = 1; //screen selector for the area updater
byte genScreen = 1; //Screen selector for the general info display
unsigned long timeS = 0; //This is used to determine when to update the menu screen
unsigned long timeOutVar = 0; //Lock out variable to ensure that menu isn't left on disabling the rest of the system
unsigned long genTimeUp = 0; //Time till the general gets changed to alternate information 
unsigned long genInfoTime = 0; //Time till the current info on the general screen gets updated
boolean clockS; //state of the clock pin e.g HIGH or LOW
boolean dataS; //state of the data pin e.g HIGH or LOW

/* ---Threshold Variables--- */
float tempThresh = 25; //Temperature should be less than this amount
int rainThresh = 300; //should be considered raining if it is less than this threshold

/* ---variables to keep track of system and check where it is irrigating--- */
boolean isWatering[4] = {false, false ,false ,false};

/* ---Initialize all the areas in garden as objects of the custom library for sensor interpretation--- */
//Per area the following info is given: The 1st moisture pin, its corresponding switch ,2nd moist ,corresponding switch ,light sensor, corresponding switch
sensorInterpretation area_1(moistPin_a1[0],modSwitch_a1[0] ,moistPin_a1[1] ,modSwitch_a1[1] , lightPin_a1 , modSwitch_a1[2]);
sensorInterpretation area_2(moistPin_a2[0],modSwitch_a2[0] ,moistPin_a2[1] ,modSwitch_a2[1] , lightPin_a2 , modSwitch_a2[2]);
sensorInterpretation area_3(moistPin_a3[0],modSwitch_a3[0] ,moistPin_a3[1] ,modSwitch_a3[1] , lightPin_a3 , modSwitch_a3[2]);
sensorInterpretation area_4(moistPin_a4[0],modSwitch_a4[0] ,moistPin_a4[1] ,modSwitch_a4[1] , lightPin_a4 , modSwitch_a4[2]);

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); //initalize instance of the lcd library
void setup(){
  Serial.begin(9600); //Start Serial Communication at a baud rate of 9600
  startupLCD(); //Start custom function that configures LCD
  sensors.begin();
  for(int i = 0; i < 4; i ++){
    if(i < 3){ //if for loop is num 0-2
      digitalWrite(rotary[i] , HIGH); //Activate pins on rotary encoder
    }
    pinMode(relays[i], OUTPUT); //Set relay pins to Output
  }
  attachInterrupt(digitalPinToInterrupt(rotary[1]), clockIn, RISING); //attach in interrupt to enable rotary encoder to function properly
  delay(2000); //Delay program so welcome screen is readable
  generalUpdate(genScreen); //Update lcd screen to first general update screen
  genTimeUp = millis(); //Reset screen change time to current program run time
  genInfoTime = millis(); //Reset screen update time to current program run time
}

void loop() {
  if((millis()-genInfoTime)/1000>1){   //updates screen and checks irrigation every second
    irrigate(); //start custom function to check what areas need to be irrigated
    FIRE(); //start custom function to check if there is a fire and deal with it accordingly
    if((millis()-genTimeUp)/1000>20){ //changes displayed screen every 20 seconds
      if(genScreen>1){
        genScreen = 1; //if the screen is more than one (2) change to 1
      }else{
        genScreen = 2; //otherwise if screen 1 than change ot screen 2
      }
      genTimeUp = millis(); //rest the variable
    }
    generalUpdate(genScreen); //update the screen after potential adjustments have been made
    genInfoTime = millis();  //reset the variable
  }
  menu(); //start custom function to check if user wants to scroll throough the area menu
  
}

/* ---Function to configure LCD upon startup--- */
void startupLCD(){
  lcd.begin(16,2); //begin LCD 16 characters in width and 2 rows in height
  lcd.backlight(); //turn on the LCD backlight
  lcd.clear(); //clear screen if it was previously populated
  lcd.setCursor(3,0); //set cursor to the third character on the first row
  lcd.print("Welcome to"); //display string on LCD
  lcd.setCursor(4,1);
  lcd.print("iGarden");
}
/* ---Function to calculate the average moisture across all sensors--- */
int moistTotal(){
  int total = 0; //variable to store the total moisture
  byte count = 0; //variable to count the amount of sensors checked
  if(digitalRead(areaSwitch_a1)){ //only execute if the area switch is turned on
    total+= area_1.moistAverage(); //read the amount of moisture and add it to total
    count ++; //increment count variable
  }
  if(digitalRead(areaSwitch_a2)){
    total+= area_2.moistAverage();
    count ++;
  }
  if(digitalRead(areaSwitch_a3)){
    total+= area_3.moistAverage();
    count ++;
  }
  if(digitalRead(areaSwitch_a4)){
    total+= area_4.moistAverage();
    count ++;
  }
  return total/count; //get the average by dividing the total moisture with the amount of active sensors
}
/* ---Function that updates the LCD screen with info of the Area specified by the passed num variable--- */
void areaUpdate(byte num){
  int moistValue = 0; //variable to store the moisture percentage of the area
  switch(num){ //check which area's info is desired
    case 1: //info about area 1 is desired
      if(digitalRead(areaSwitch_a1)){ //check if Area 1's switch is on
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Area 1 :");
        lcd.setCursor(0,1);
        lcd.print("M:");
        lcd.setCursor(3, 1);
        moistValue = area_1.moistAverage(); //populate the variable with custom moisture function
        lcd.print(moistValue); //print the moisture percentage for the area
        if(moistValue <10){
          lcd.setCursor(4,1); //if moist percentage is only 1 digit display % symbol at character 4
        }else if(moistValue< 100 && moistValue > 9){
          lcd.setCursor(5,1); //if moist average is 2 digits display % symbol at character 5
        }else{
          lcd.setCursor(6,1); //if moist average is 3 digits display % symbol at character 6
        }
        lcd.print("%");
        lcd.setCursor(8 ,1);
        lcd.print("L:");
        lcd.setCursor(10,1);
        lcd.print(area_1.timeOfDay()); //print what the time of day is for specific area with custom function
      }else{ //if area switch is OFF print that the area is offline
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Area 1 :");
        lcd.setCursor(3,1);
        lcd.print("Offline");
      }
    break;
    case 2: //info for area 2 is desired
      if(digitalRead(areaSwitch_a2)){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Area 2 :");
        lcd.setCursor(0,1);
        lcd.print("M:");
        lcd.setCursor(3, 1);
        moistValue = area_2.moistAverage();
        lcd.print(moistValue);
        if(moistValue <10){
          lcd.setCursor(4,1);
        }else if(moistValue< 100 && moistValue > 9){
          lcd.setCursor(5,1);
        }else{
          lcd.setCursor(6,1);
        }
        lcd.print("%");
        lcd.setCursor(8 ,1);
        lcd.print("L:");
        lcd.setCursor(10,1);
        lcd.print(area_2.timeOfDay());
      }else{
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Area 2 :");
        lcd.setCursor(3,1);
        lcd.print("Offline");
      }
    break;
    case 3: //info for area 3 is desired
      if(digitalRead(areaSwitch_a3)){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Area 3 :");
        lcd.setCursor(0,1);
        lcd.print("M:");
        lcd.setCursor(3, 1);
        moistValue = area_3.moistAverage();
        lcd.print(moistValue);
        if(moistValue <10){
          lcd.setCursor(4,1);
        }else if(moistValue< 100 && moistValue > 9){
          lcd.setCursor(5,1);
        }else{
          lcd.setCursor(6,1);
        }
        lcd.print("%");
        lcd.setCursor(8 ,1);
        lcd.print("L:");
        lcd.setCursor(10,1);
        lcd.print(area_3.timeOfDay());
      }else{
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Area 3 :");
        lcd.setCursor(3,1);
        lcd.print("Offline");
      }
    break;
    case 4: //info for area 4 is desired
      if(digitalRead(areaSwitch_a4)){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Area 4 :");
        lcd.setCursor(0,1);
        lcd.print("M:");
        lcd.setCursor(3, 1);
        moistValue = area_4.moistAverage();
        lcd.print(moistValue);
        if(moistValue <10){
          lcd.setCursor(4,1);
        }else if(moistValue< 100 && moistValue > 9){
          lcd.setCursor(5,1);
        }else{
          lcd.setCursor(6,1);
        }
        lcd.print("%");
        lcd.setCursor(8 ,1);
        lcd.print("L:");
        lcd.setCursor(10,1);
        lcd.print(area_4.timeOfDay());
      }else{
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Area 4 :");
        lcd.setCursor(3,1);
        lcd.print("Offline");
      }
    break;
  }
}
/* ---Function that updates the LCD screen with general info according to the num area--- */
void generalUpdate(byte num){
  int moistValue = 0; //initialize a variable to store moist percentage
  String temperature = getTemp(); //check temperature and populate temperature variable already so there isn't any lag
  switch(num){ //checks what general screen is desired
    case 2: //screen 2 is desired and displayed on LCD
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("General:");
      lcd.setCursor(0,1);
      lcd.print("M:");
      lcd.setCursor(3, 1);
      if(moistTotal() == 0){ //If there is 0% moisture populate moistValue variable with "OFF"
        moistValue = "OFF";
      }else{ //otherwise make the variable equal to the moist percentage
        moistValue = moistTotal();
      }
      lcd.print(moistValue); //display the contents of the moist value variable
      if(moistValue <10){
        lcd.setCursor(4,1);
      }else if(moistValue< 100 && moistValue > 9){
        lcd.setCursor(5,1);
      }else{
        lcd.setCursor(6,1);
      }
      lcd.print("%");
      lcd.setCursor(8 ,1);
      lcd.print("L:");
      lcd.setCursor(10,1);
      lcd.print(dayORnight()); //display if it is night or day overall according to all the LDR's
    break;
    case 1: //general screen 1 is desired and displayed on LCD
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("General:");
      lcd.setCursor(0,1);
      lcd.print("R:");
      lcd.setCursor(3, 1);
      if(isRaining()){ //check if it is raining with this custom function
        lcd.print("!"); //if it is then print ! symbol
      }else{
        lcd.print("X"); //if it isn't then print X symbol
      }
      lcd.setCursor(6 ,1);
      lcd.print("T:");
      lcd.setCursor(9,1);
      lcd.print(temperature); //display the contents of temperature variable
    break;
  }
}
/* ---Function to check if it is day or night across all variable--- */
String dayORnight(){
  int days = 0; //Counter for the amount of sensors that output that is day time
  int count = 0; //Counter for the amount of active sensors
  if(digitalRead(areaSwitch_a1)){ //check if the specific area is active
    count++; //if it is active increment count variable
    if(area_1.timeOfDay().equals("Day")){ //if the time of day for specific area is day than increment day counter
      days ++;
    }
  }
  if(digitalRead(areaSwitch_a2)){
    count++;
    if(area_2.timeOfDay().equals("Day")){
      days ++;
    }
  }
  if(digitalRead(areaSwitch_a3)){
    count++;
    if(area_3.timeOfDay().equals("Day")){
      days ++;
    }
  }
  if(digitalRead(areaSwitch_a4)){
    count++;
    if(area_4.timeOfDay().equals("Day")){
      days ++;
    }
  }
  if(days > 0 || count == 0){ //if there are no sensors or at least one outputs day
    return "Day"; //then return that it is day
  }else{
    return "Night"; //otherwise return that is night time
  }
}
/* ---Function that controls the navigation of the Area Info Menu--- */
void menu(){
  boolean butt = digitalRead(rotary[0]); //check the state of rotary button (Pressed or Released)
  if(butt == false){ //If it is pressed enter the menu and initially inform user to release the button
    lcd.clear();
    lcd.setCursor(0 ,0);
    lcd.print("Getting Info,");
    lcd.setCursor(0 ,1);
    lcd.print("Release Button");
    screen = 1; //reset screen to the 1st area  
    int temp = 5; //Temporary variable that will be used to see if there is change in screen
    delay(2000); //Delay gives users the chance to release button
    butt = true; //Turn the button state OFF
    timeS = millis(); //reset area updating timer to current program runtime
    timeOutVar = millis(); //reset the kickout variable to the current program runtime
    while(butt == true && (millis()-timeOutVar)/1000<60){ // checks if button is still unpressed and that 60 seconds has not passed
      if(screen != temp){
        timeOutVar = millis(); //when screen changes variable resets so that users are not locked out while still navigating
      }
      if(screen!=temp || (millis()- timeS)/1000> 1){ //check if the selected screen is not already displayed and if at least one second has passed.
        areaUpdate(screen); //then update screen
        timeS = millis(); //and reset to area updating timer to current program runtime
      }
      temp = screen; //change temporary variable to current screen
      butt = digitalRead(rotary[0]); //check if the button is being pressed
    }
    //display message stating that menu has been exited and that button should be released
    lcd.clear();
    lcd.setCursor(0 ,0);
    lcd.print("Going back,");
    lcd.setCursor(0 ,1);
    lcd.print("Release Button");
    delay(1000); //delay gives user chance to release button
  }
}
/* ---Interrupt function that increments the screen variable according to the rotary encoder--- */
void clockIn(){
  noInterrupts();// stop any more signals from interfering
  dataS = digitalRead(rotary[2]); //check the state of the data pin
  clockS = digitalRead(rotary[1]); //check the state of the clock pin
  if(dataS == clockS){ //if data is equal to clock than decrement screen variable
    if(screen <= 1){ //if the screen is already at 1st screen than change to 4th screen 
      screen=4;
    }else{
      screen -= 1; //otherwise decrement as normal
    }
  }else{ //if data is not equal to clock than increment screen variable
    if(screen >= 4){ //if the screen is already at 4 go to 1st screen
      screen = 1;
    }else{
      screen += 1; //otherwise increment as normal
    }
  }
  delay(100);//smoothen the code out
  interrupts();// enable interrupts again
}
/* ---Function that gets the average temperature and converts it to a string--- */
String getTemp(){ 
  float tempTotal = 0; //stores the total addition of temperatures
  byte count = 0; //counts the total amount of active sensors
  if(digitalRead(modSwitch_g[0])){ 
    sensors.requestTemperatures(); //request the temperatures
    tempTotal= sensors.getTempCByIndex(0); //add temperature outputted to the tempTotal variable
    count++; //increment the count variable
  }
  if(count == 0){ 
    return "N/A"; //if there aren't any sensors active return that information is "N/A"
  }
  return String(tempTotal); //otherwise return the average temperature in string format
  
}
/* ---Function that checks if temperature is low enough for irrigation to take place--- */
boolean tempReady(){
  String temp = getTemp(); //get the average temperature
  if(temp.equals("N/A")){
    return true; //if there aren't any sensors present return that irrigation may take place
  }else{
    if(temp.toFloat() < tempThresh){
      return true; //if the temperature is lower than the maximum return that irrigation may take place
    }else{
      return false; //if the temperature is too high return that irrigation may not take place
    }
  }
}
/* ---Function that checks if it is raning--- */
boolean isRaining(){
  if(digitalRead(modSwitch_g[2])){ //check if the rain sensor is active
    if(analogRead(rainPin_g) < rainThresh){ 
      return true; //if the rain pin returns a lower value than the threshold return that it is raining
    }else{
      return false; //it is higher return that it isn't raining
    }
  }else{
    return false; //if the sensor isn't active than return that it isn't raining
  }
}
/* ---Function that checks all the general sensors for if irrigation is possible--- */
boolean generalReady(){
  if(tempReady() && !(isRaining())){ 
    return true; //if the temperature is low enough and it isn't raining return that irrigation may take place
  }else{
    return false; //otherwise return that irrigation shouldn't take place
  }
}
/* ---Function that controls all the relays seperate according to their corresponding environment/area--- */
void irrigate(){
  if(generalReady()){ //first check if all the general sensors allow irrigation to take place
    //Check Area 1
    if(digitalRead(areaSwitch_a1)){ //check if area is active
      if(area_1.waterReady()){ //check if the area needs to be irrigated
        if(isWatering[0] == false){ 
          digitalWrite(relay_a1, HIGH); //if it isn't currently being irrigated then turn the relay on
          isWatering[0] = true; //set corresponding variable to it is busy irrigating
        }
      }else{ 
        if(isWatering[0]){ //if the area doesn't need to be irrigated than first check if it is currently irrigating
          digitalWrite(relay_a1, LOW); //then turn off the corresponding relay
          isWatering[0] = false; //and set the isWatering variable to it isn't busy irrigating
        }
      }
     }else{
      if(isWatering[0]){ //if the area is set to inactive check if it was busy irrigating
        digitalWrite(relay_a1, LOW); //then turn the corresponding relay off
        isWatering[0] = false; //and set isWatering variable to not irrigating
      }
    }
    //Check Area 2
    if(digitalRead(areaSwitch_a2)){
      if(area_2.waterReady()){
        if(isWatering[1] == false){
          digitalWrite(relay_a2, HIGH);
          isWatering[1] = true;
        }
      }else{
        if(isWatering[1]){
          digitalWrite(relay_a2, LOW);
          isWatering[1] = false;
        }
      }
     }else{
      if(isWatering[1]){
        digitalWrite(relay_a2, LOW);
        isWatering[1] = false;
      }
    }
    //Check Area 3
    if(digitalRead(areaSwitch_a3)){
      if(area_3.waterReady()){
        if(isWatering[2] == false){
          digitalWrite(relay_a3, HIGH);
          isWatering[2] = true;
        }
      }else{
        if(isWatering[2]){
          digitalWrite(relay_a3, LOW);
          isWatering[2] = false;
        }
      }
    }else{
      if(isWatering[2]){
        digitalWrite(relay_a3, LOW);
        isWatering[2] = false;
      }
    }
    //Check Area 4
    if(digitalRead(areaSwitch_a4)){
      if(area_4.waterReady()){
        if(isWatering[3] == false){
          digitalWrite(relay_a4, HIGH);
          isWatering[3] = true;
        }
      }else{
        if(isWatering[3]){
          digitalWrite(relay_a4, LOW);
          isWatering[3] = false;
        }
      }
    }else{
      if(isWatering[3]){
        digitalWrite(relay_a4, LOW);
        isWatering[3] = false;
      }
    }
  }else{ //if the general sensors don't allow the garden to irrigate
    for(int i = 0 ; i < 4; i ++){ //use for loop to individually check if an area was busy irrigating and then turn off corresponding relay
      if(isWatering[i]){ 
        digitalWrite(relays[i], LOW); 
        isWatering[i] = false; //then set corresponding variable to not irrigating
      }
      
    }
  }
}
/* ---Function that checks if there is an fire and then acts accordingly--- */
void FIRE(void){
  boolean switch1 =digitalRead(modSwitch_g[3]); //capture the state of the first fire switch 
  boolean switch2 = digitalRead(modSwitch_g[4]); //capture the state of the second
  if(switch1 || switch2){ //if either of the fire switches are active execute following code
    /*The while loop requires the following conditions:
     * 1*--- that the first fire sensor shows that there is a fire and for the fire sensor's corresponding switch to be active
     * 2*--- or that the same apllies to the scond fire sensor
     */
    while((switch1 && digitalRead(firePin_g[0])== false) || (switch2 && digitalRead(firePin_g[1])== false)){ //this will repeat till the conditions arent met(when there isn't a fire)
      //print fire warning
      lcd.clear(); 
      lcd.setCursor(3 ,0);
      lcd.print("FIRE!!!!!");
      lcd.setCursor(3 ,1);
      lcd.print("Get Help!");
      for(int i = 0 ; i < 4; i ++){
        if(isWatering[i] == false){//checks if the sprayer was already irrigating
          digitalWrite(relays[i], HIGH); //if it wasn't then the relay is turned on
          isWatering[i] = true; //and the corresponding isWatering variable shows it is irrigating
        }
        
      }
      delay(700);//make warning text readable
    }
  }
}
