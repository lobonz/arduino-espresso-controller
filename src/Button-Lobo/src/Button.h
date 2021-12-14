 /**
  * @brief This library allows to manage a button.
  *  It can specify if the pressure is long or not. Morevoer, is possible to assign a time (in milliseconds) to consider the long press.
  * Copyright (c) 2020 Davide Palladino. 
  * All right reserved.
  * 
  * @author Davide Palladino
  * @contact me@davidepalladino.com
  * @website www.davidepalladino.com
  * @version 2.0.0
  * @date 1st June, 2021
  * 
  * This library is free software; you can redistribute it and/or
  *  modify it under the terms of the GNU General Public
  *  License as published by the Free Software Foundation; either
  *  version 3.0 of the License, or (at your option) any later version
  * 
  * This library is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  *  GNU Lesser General Public License for more details.
  * 
  */

#ifndef BUTTON_H
    #define BUTTON_H
    
    #ifndef ARDUINO_H
        #include <Arduino.h>
    #endif

    #define DEFAULT_LONG_PRESS 0                                                                // Default value in milliseconds for the long press.
    #define DEBOUNCE_DELAY 50
    typedef enum input : uint8_t {B_PULLUP, B_NOPULLUP} input_t;                                // Symbolic constants to indicate, respectively, if is "INPUT_PULLUP" or "INPUT".

    class Button {
        public:            
            /** 
             * @brief This constructor creates the object setting the pin button, the mode of the input and the time to define the long press. Moreover, it calls "pinMode".
             * @param pin Digital pin of the button.
             * @param mode Mode of the input, between "INPUT" (with "B_NOPULLUP" constant) and "INPUT_PULLUP" (with "B_PULLUP" constant).
             * @param timeLongPress Time in milliseconds for long press.
             */
            begin(uint8_t pin, input_t mode, uint32_t timeLongPress);

            /**
             * @brief This method sets the time for the long press.
             * @param timeLongPress Time in milliseconds for long press.
             */
            void setTimeLongPress(uint32_t timeLongPress);

            /**
             * @brief This method gets the time for the long press.
             * @return Time in milliseconds for long press.
             */          
            uint32_t getTimeLongPress();

            /**
             * @brief These methods get the state of the button if it has changed.
             * @return true or false.
             */
            bool wasPressed();
            bool wasLongPressed();
            bool wasReleased();

            /**
             * @brief This method gets the actual press, both for short and long. For short press there is the debouncing.
             * @return Value "pressed" when pressed, "released" when released, "longpress" when held longer than timeLongPress
             */
            String update();

        private:
            uint8_t pin;                                // Pin of the button to read the status.
            uint8_t mode;                               // Mode of the input, between "INPUT" (with "B_NOPULLUP" constant) and "INPUT_PULLUP" (with "B_PULLUP" constant).
            uint32_t timeLongPress;                     // Time in milliseconds for the long press.
            uint8_t valuePress;                         // This variable will contain the value where the button will be considered pressed. In example, "HIGH" if the "pinMode" is set to "INPUT"; "LOW" if the "pinMode" is set to "INPUT_PULLUP".
            uint32_t timeOut;                           // End of time, calculated with the actual time plus "timeLongPress".
            bool isPressed;                             // Flag to indicate if the button was already pressed.
            bool isLongPressed;                         // Flag to indicate if the last press was long or not.
            String buttonStatus;                         // "" when no change, "pressed" when pressed, "released" when released, "longpressed" when held longer than timeLongPress
            uint32_t lastDebounceTime;

            /**
             * @brief This method sets the input mode.
             * @param mode Mode of the input.
             */
            void setMode(input_t mode);

            /**
             * @brief This method sets the value where the button will be considered pressed.
             */
            void setValuePress();  
    };
#endif
