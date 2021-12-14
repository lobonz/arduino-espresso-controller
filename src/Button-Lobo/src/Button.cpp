#include "Button.h"

// Button::begin(uint8_t pin) : begin(pin, B_NOPULLUP, DEFAULT_LONG_PRESS) {}

// Button::begin(uint8_t pin, input_t mode) : begin(pin, mode, DEFAULT_LONG_PRESS) {}

// Button::begin(uint8_t pin, uint32_t timeLongPress) : begin(pin, B_NOPULLUP, timeLongPress) {}

Button::begin(uint8_t pin, input_t mode, uint32_t timeLongPress) {
    this->pin = pin;
    this->lastDebounceTime = millis();
    setMode(mode);
    setValuePress();

    pinMode(this->pin, this->mode);
    
    setTimeLongPress(timeLongPress);
}

void Button::setTimeLongPress(uint32_t timeLongPress) { this->timeLongPress = timeLongPress; }

uint32_t Button::getTimeLongPress() { return this->timeLongPress; }

bool Button::wasPressed(){
    if(this->buttonStatus == "pressed" && isPressed){
        return true;
    }
    else
    {
        return false;
    }
}
bool Button::wasLongPressed(){
    if(this->buttonStatus == "longpressed" && isLongPressed){
        return true;
    }
    else
    {
        return false;
    }
}
bool Button::wasReleased(){
    if(this->buttonStatus == "released" && !isPressed && !isLongPressed){
        return true;
    }
    else
    {
        return false;
    }
}

String Button::update() {  
    /* Read and save the value for next analysis. */
    uint8_t valueRead = digitalRead(this->pin);
    this->buttonStatus = "";

    if ((millis() - this->lastDebounceTime) > DEBOUNCE_DELAY)
    {
        this->lastDebounceTime = millis();
        /* Checking if the button is pressed in this moment. */
        if (valueRead == this->valuePress) {
            /* Checking if is the first press or not. */
            if (!this->isPressed) { 
                this->buttonStatus = "pressed";
                this->isPressed = true;
                /* Checking if is set the time of long press. If there is a value, will be set the timeout. */
                if (getTimeLongPress() > DEFAULT_LONG_PRESS) {
                    timeOut = millis() + getTimeLongPress();
                }
            } else {
                /* Checking if this is a long press. */
                if ((millis() >= timeOut) && (getTimeLongPress() > DEFAULT_LONG_PRESS) && !this->isLongPressed) {
                    //Serial.println("Is long press on first time.");
                    this->buttonStatus = "longpressed";
                    this->isLongPressed = true;
                } else if (this->isLongPressed) {
                    this->buttonStatus = "";//No Change
                } else {
                    this->buttonStatus = "";//No Change
                }
            }
        
        /* Else, the button has not been just pressed. */
        } else {
            /* Checking if is the short press, verifying if is set the "timeLongPress" or not. */      
            //if (this->isPressed && (((this->buttonStatus != "longpress") && (millis() < timeOut) && !this->isLongPressed) || (getTimeLongPress() == DEFAULT_LONG_PRESS))) {
            if (this->isPressed ) {
                //Serial.println("Is short press.");
                this->isPressed = false;
                this->isLongPressed = false;
                
                this->buttonStatus = "released";

            /* Else, is not a press. */
            } else {
                //Serial.println("Is not pressed.");
                this->isPressed = false;
                this->isLongPressed = false;
                this->buttonStatus = "";
            }
        }
    }

    return this->buttonStatus;
}

void Button::setMode(input_t mode) {
    /* Translation of "mode" parameter "B_NOPULLUP"/"B_PULLUP" to the rispective "INPUT"/"INPUT_PULLUP". */
    if (mode == B_NOPULLUP) {
        this->mode = INPUT;
    } else if (mode == B_PULLUP) {
        this->mode = INPUT_PULLUP;
    }
}

void Button::setValuePress() {
    /* Checking what is the value of press, "HIGH" if the mode is INPUT; "LOW" if is INPUT_PULLUP. */
    if (mode == INPUT) {
        this->valuePress = HIGH;
    } else if (mode == INPUT_PULLUP) {
        this->valuePress = LOW;
    }
}