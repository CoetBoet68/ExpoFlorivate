#include "Arduino.h" //insert library with all the main arduino functions
#include "sensorInterpretation.h" //insert header file of the library

//main function of the library where all the library variables get passed values
sensorInterpretation::sensorInterpretation(byte moist1, byte switch1 , byte moist2 , byte switch2 , byte light , byte switch3){
  /* ---The library assigns varaibles the values passed to it from the main program--- */
   _moist1 = moist1;
   _switch1 = switch1;
   _moist2 = moist2;
   _switch2 = switch2;
   _light = light;
   _switch3 = switch3;
}
/* ---Function that helps to troubleshoot system through Serial Monitor when something goes wrong--- */
sensorInterpretation::sensorTester(){
  if(digitalRead(_switch1)){ //checks if the sensor is active
    Serial.print("Moist 1 : "); //prints the name of the sensor to serial monitor
    Serial.println(analogRead(_moist1)); //prints the value of the sensor
  }else{
    Serial.println("Moist 1 is deactivated"); //if the sensor is deactivated it prints this to the serial monitor
  }
  //the previous code is repeated for all the sensors in the class
  if(digitalRead(_switch2)){
    Serial.print("Moist 2 : ");
    Serial.println(analogRead(_moist2));
  }else{
    Serial.println("Moist 2 is deactivated");
  }
  if(digitalRead(_switch3)){
    Serial.print("Light : ");
    Serial.println(analogRead(_light));
  }else{
    Serial.println("Light is deactivated");
  }
}
/* ---Function that returns the average of the moisture sensors--- */
int sensorInterpretation::moistAverage(){
  double totalMoist = 0.0; //Variable to store the total amount of moisture
  byte count = 0; //variable that increments with the amount of active sensors
  if(digitalRead(_switch1)){ //checks if first moisture sensor is active
    totalMoist += float(map(analogRead(_moist1),1023 ,0,0 ,1023)); //it reverses the value and adds it to the total moisture
    count ++; //increments the amount of active sensors
   }
   if(digitalRead(_switch2)){
     totalMoist +=float(map(analogRead(_moist2),1023 ,0,0 ,1023));
     count++;
    }
  return (totalMoist/(1023*count))*100; //returns the moisture percentage (total moisture divided by the number of active sensors
}
/* ---Function that checks what the time of day is--- */
String sensorInterpretation::timeOfDay(){
  if(digitalRead(_switch3)){ //checks if the light sensor is active
    int sensVal = analogRead(_light); //the amount the sensor outputs is stored in a variable
    if(sensVal > 550){ //if the value is more than 550 than it is day
      return "Day"; //return the string day
    }else{
      return "Night"; //if it is less than return the night
    }
  }else{ //if the sensor is inactive return Not Available
    return "N/A";
  }
}
/* ---Function that checks if the area needs to be irrigated or not--- */
boolean sensorInterpretation::waterReady(){
  if(moistReady(1) && moistReady(2) && lightReady()){ //if all the sensors meet the conditions:
    return true; //return true that this area should be irrigated
  }else{
    return false; //return false ,this area shouldn't be irrigated
  }
}
/* ---Function that checks if the moisture sensors determined that the area should be irrigated--- */
//function takes the argument of num which determines which sensor is tested
boolean sensorInterpretation::moistReady(byte num){
  switch(num){
    //sensor 1 is tested
    case 1:
      if(digitalRead(_switch1)){ //checks if sensor is active
        if(analogRead(_moist1) > 450){ 
          return true; //if sensor outputs more than 450 return true
        }else{
          return false; //if sensor outputs less than 450 than return false
        }
      }else{
        return true; //if sensor is inactive than return false
      }
    break;
    //test sensor 2
    case 2:
      if(digitalRead(_switch2)){
        if(analogRead(_moist2) > 450){
          return true;
        }else{
          return false;
        }
      }else{
        return true;
      }
    break;
  }
}
/* ---Function that checks if it is the appropriate time of day to irrigate--- */
boolean sensorInterpretation::lightReady(){
  if(digitalRead(_switch3)){ //checks if light sensor is active
    if(analogRead(_light) < 550){
      return true;  //if sensor outputs less than 550 then return true
    }else{
      return false; //if sensor outputs more than 550 then return false
    }
  }else{
    return true; //if sensor is inactive then return true
  }
}
