#include <BleGamepad.h>
#include <BleConnectionStatus.h>
#include "Button.h"
#include "Rotary.h"
#include "RotaryKnob.h"
#include <driver/adc.h>
#include <driver/gpio.h>
 
//pins, see https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
//GPIO34-39 can only be set as input mode and do not have software pullup or pulldown functions.
   #define ONBOARD_LED GPIO_NUM_2 //this one is used by rotaryencoder2button as well
   #define BATTERY_LED GPIO_NUM_4
//Buttons
#define BUTTON1 GPIO_NUM_26
#define BUTTON2 GPIO_NUM_18
#define BUTTON3 GPIO_NUM_27
#define BUTTON4 GPIO_NUM_25
#define BUTTON5 GPIO_NUM_19
#define BUTTON6 GPIO_NUM_23
#define BUTTON7 GPIO_NUM_5    //Encoder Button
#define BUTTON8 GPIO_NUM_32   //Encoder Button
#define BUTTON9 GPIO_NUM_12
#define BUTTON10 GPIO_NUM_4
#define BUTTON11 GPIO_NUM_13
#define BUTTON12 GPIO_NUM_0

//Rotarys
#define ROTARYENCODER1A GPIO_NUM_14
#define ROTARYENCODER1B GPIO_NUM_33
#define ROTARYENCODER2A GPIO_NUM_16
#define ROTARYENCODER2B GPIO_NUM_17




//times

#define ROTARYENCODER_PRESSTIME 20


RTC_DATA_ATTR int bootCount = 0; //stays in memory after standby
volatile int countTimerInterrupt;
int standbyCounter = 0;
float percentCharged = 100;
int read_raw;
float voltage_value;
bool pushedSent = false;


//the init for the timerInterrupt
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

//inititialization knobs
BleGamepad bleGamepad("Volante R9", "NAME", 100);
//Buttons
Button button1(BUTTON1);
Button button2(BUTTON2);
Button button3(BUTTON3);
Button button4(BUTTON4);
Button button5(BUTTON5);
Button button6(BUTTON6);
Button button7(BUTTON7);
Button button8(BUTTON8);
Button button9(BUTTON9);
Button button10(BUTTON10);
Button button11(BUTTON11);
Button button12(BUTTON12);






//Button button24(ROTARYENCODER4BUTTON);
Button buttons[] = {button1, button2, button3, button4, button5, button6, button7, button8, button9, button10, button11, button12};
int bleButtons[] = {BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4, BUTTON_5, BUTTON_6, BUTTON_7, BUTTON_8, BUTTON_9, BUTTON_10, BUTTON_11, BUTTON_12};
int numberOfButtons = *(&buttons + 1) - buttons;

RotaryKnob rotaryKnob1 = RotaryKnob(ROTARYENCODER1A, ROTARYENCODER1B, ROTARYENCODER_PRESSTIME);
RotaryKnob rotaryKnob2 = RotaryKnob(ROTARYENCODER2A, ROTARYENCODER2B, ROTARYENCODER_PRESSTIME);


RotaryKnob rotaryKnobs[] = {rotaryKnob1, rotaryKnob2};
int bleRotaryKnobPins[] = {BUTTON_13, BUTTON_14, BUTTON_15, BUTTON_16};
int numberofRotaryKnobs = *(&rotaryKnobs + 1) - rotaryKnobs;



/************************** S E T  U P *****************************************************/
void setup() {
  setCpuFrequencyMhz(80); //20-30% batterylife saver
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initializing...");
  pinMode(ONBOARD_LED,OUTPUT);
  //pinMode(GPIO_NUM_4,OUTPUT);
  
  
  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  
  
  

  attachInterrupt(buttons[0].getButtonId(), buttonsISR1, CHANGE);
  attachInterrupt(buttons[1].getButtonId(), buttonsISR1, CHANGE);
  attachInterrupt(buttons[2].getButtonId(), buttonsISR2, CHANGE);
  attachInterrupt(buttons[3].getButtonId(), buttonsISR3, CHANGE);
  attachInterrupt(buttons[4].getButtonId(), buttonsISR4, CHANGE);
  attachInterrupt(buttons[5].getButtonId(), buttonsISR5, CHANGE);
  attachInterrupt(buttons[6].getButtonId(), buttonsISR6, CHANGE);
  attachInterrupt(buttons[7].getButtonId(), buttonsISR7, CHANGE);
  attachInterrupt(buttons[8].getButtonId(), buttonsISR8, CHANGE);
  attachInterrupt(buttons[9].getButtonId(), buttonsISR9, CHANGE);
  attachInterrupt(buttons[10].getButtonId(), buttonsISR10, CHANGE);
  attachInterrupt(buttons[11].getButtonId(), buttonsISR11, CHANGE);
  attachInterrupt(buttons[12].getButtonId(), buttonsISR12, CHANGE);
  attachInterrupt(buttons[13].getButtonId(), buttonsISR13, CHANGE);
  attachInterrupt(buttons[14].getButtonId(), buttonsISR14, CHANGE);
  attachInterrupt(buttons[15].getButtonId(), buttonsISR15, CHANGE);
  attachInterrupt(buttons[16].getButtonId(), buttonsISR16, CHANGE);

  //the ISRs for the rotaryencoders
  #ifdef ROTARYENCODER1A
  attachInterrupt(ROTARYENCODER1A, rotaryKnobISR1, CHANGE);
  attachInterrupt(ROTARYENCODER1B, rotaryKnobISR1, CHANGE);
  attachInterrupt(ROTARYENCODER2A, rotaryKnobISR2, CHANGE);
  attachInterrupt(ROTARYENCODER2B, rotaryKnobISR2, CHANGE);
  #endif
  
  //the battery dac pin for measuring the voltage (percent battery)
 // adc2_config_channel_atten( ADC2_CHANNEL_7, ADC_ATTEN_DB_11 );

  //start bluetooth
  BleGamepadConfiguration bleGamepadConfig;
bleGamepadConfig.setButtonCount(16);
bleGamepadConfig.setIncludeRxAxis(0);
bleGamepadConfig.setIncludeRyAxis(0);
bleGamepadConfig.setIncludeRzAxis(0);
bleGamepadConfig.setIncludeSlider1(0);
bleGamepadConfig.setIncludeSlider2(0);
bleGamepadConfig.setIncludeXAxis(0);
bleGamepadConfig.setIncludeYAxis(0);
bleGamepadConfig.setIncludeZAxis(0);
bleGamepadConfig.setHatSwitchCount(0);
bleGamepad.begin(&bleGamepadConfig);
}

/************************** E N D  S E T  U P *****************************************************/

/************************** L O O P *****************************************************/
void loop() {
  
  if(bleGamepad.isConnected()) {
    digitalWrite(ONBOARD_LED,HIGH);
    scanButtons();
    scanRotaryEncoders();
  }     
  else {
    digitalWrite(ONBOARD_LED,LOW);
  }
  
}
/************************** E N D  L O O P ***********************************************/



void scanButtons() {
  for(int i=0; i<numberOfButtons; i++) {
    buttons[i].button_ISR();
    if (buttons[i].pushed()) {
      standbyCounter = 0;                           //reset the standbyCounter
      bleGamepad.press(bleButtons[i]);
      buttons[i].clearChangeFlag();
    }
    else {
      if (buttons[i].released()) {
        bleGamepad.release(bleButtons[i]);
        buttons[i].clearChangeFlag();
      }
    }
  }   
}

void scanRotaryEncoders() {
    for(int i=0; i<numberofRotaryKnobs; i++) {
      if (rotaryKnobs[i].isSendingPressButton()) {       
         if(rotaryKnobs[i].isReleaseNeeded()) {
            bleGamepad.release(bleRotaryKnobPins[i*2]);
            bleGamepad.release(bleRotaryKnobPins[i*2+1]);
            rotaryKnobs[i].setSendingPressButton(false);
            rotaryKnobs[i].releaseDone();
              delay(50);
         }
      }
      else if (rotaryKnobs[i].changed()) {
          if (rotaryKnobs[i].isUp()) {
            rotaryKnobs[i].counter--;
            bleGamepad.press(bleRotaryKnobPins[i*2]);
            delay(50);
            //Serial.println("up");
          } 
          else {
            rotaryKnobs[i].counter++;
            bleGamepad.press(bleRotaryKnobPins[i*2+1]);
            delay(50);
            //Serial.println("down");
          }
          rotaryKnobs[i].setSendingPressButton(true);

      }
    }
}

void buttonsISR0() {
   buttons[0].button_ISR();
}
void buttonsISR1() {
   buttons[1].button_ISR();
}
void buttonsISR2() {
   buttons[2].button_ISR();
}
void buttonsISR3() {
   buttons[3].button_ISR();
}
void buttonsISR4() {
   buttons[4].button_ISR();
}
void buttonsISR5() {
   buttons[5].button_ISR();
}
void buttonsISR6() {
   buttons[6].button_ISR();
}
void buttonsISR7() {
   buttons[7].button_ISR();
}
void buttonsISR8() {
   buttons[8].button_ISR();
}
void buttonsISR9() {
   buttons[9].button_ISR();
}
void buttonsISR10() {
   buttons[10].button_ISR();
}
void buttonsISR11() {
   buttons[11].button_ISR();
}
void buttonsISR12() {
   buttons[12].button_ISR();
}
void buttonsISR13() {
   buttons[13].button_ISR();
}
void buttonsISR14() {
   buttons[14].button_ISR();
}
void buttonsISR15() {
   buttons[15].button_ISR();
}
void buttonsISR16() {
   buttons[16].button_ISR();
}

// rotate is called anytime the rotary inputs change state.
void rotaryKnobISR1() {
    rotaryKnobs[0].ISR();
}
void rotaryKnobISR2() {
    rotaryKnobs[1].ISR();
}
void rotaryKnobISR3() {
    rotaryKnobs[2].ISR();
}
void rotaryKnobISR4() {
    rotaryKnobs[3].ISR();
}





