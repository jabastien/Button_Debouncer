//*********************************************************************************
// Platform Independent State Button Debouncer
// 
// Description: Debounces buttons on a single port being used by the application.
// This module takes what the signal on a GPIO port is doing and removes
// the oscillations caused by a bouncing button and tells the application if
// the button(s) are debounced. A benefit of this algorithm is that it can play
// nicely with button interrupts. Below is an example of how the button debouncer
// would work in practice in relation to a single button:
// 
// Real Signal:     00110000000000000001110000000000000000001111111000000000000000
// Bouncy Signal:   00101110000000000001101010000000000000001010101101000000000000
// Debounced Sig:   00111111111111111001111111111111111000001111111111111111111100
// 
// The debouncing algorithm used in this library is based partly on Jack
// Ganssle's state button debouncer example shown in, "A Guide to Debouncing" 
// Rev 3. http://www.ganssle.com/debouncing.htm
// 
// Copyright (C) 2014 Trent Cleghorn
// 
// Email: trentoncleghorn@gmail.com
// 
//                                  MIT License
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//*********************************************************************************

//*********************************************************************************
// Headers
//*********************************************************************************
#include "button_debounce.h"

//*********************************************************************************
// Functions
//*********************************************************************************
void 
ButtonDebounceInit(tDebouncer *port, uint8_t pulledUpButtons)
{
    uint8_t i;
    
    port->index = 0;
    port->debouncedState = 0xFF;
    port->changed = 0x00;
    port->pullType = pulledUpButtons;
    
    // Initialize the state array
    for(i = 0; i < MAX_BUTTON_CHECKS; i++)
    {
        port->state[i] = 0xFF;
    }
}

void
ButtonProcess(tDebouncer *port, uint8_t portStatus)
{
    uint8_t i;
    uint8_t lastDebouncedState = port->debouncedState;
    
    // If a button is high and is pulled down or
    // if a button is low and is pulled high, use a 0 bit
    // to denote the button has changed state. Else, a 1 bit
    // shows it is in a normal position.
    portStatus = ~(portStatus ^ port->pullType);
    
    // Save the port status info into the state array
    port->state[port->index] = portStatus;
    
    // Debounce the buttons
    for(i = 0, port->debouncedState = 0xFF; i < MAX_BUTTON_CHECKS; i++)
    {
        port->debouncedState = port->debouncedState & port->state[i];
    }
    
    // Check to make sure the index hasn't gone over the limit
    port->index++;
    if(port->index >= MAX_BUTTON_CHECKS)
    {
        port->index = 0;
    }
    
    // Calculate what changed.
    // If the switch was high and is now low, 1 and 0 xORed with
    // each other produces a 1. If the switch was low and is now
    // high, 0 and 1 xORed with each other produces a 1. Otherwise,
    // it is 0
    port->changed = port->debouncedState ^ lastDebouncedState;
}

uint8_t
ButtonPressed(tDebouncer *port, uint8_t GPIOButtonPins)
{
    // If the button changed and it changed to a 1, then the
    // user just pressed it.
    return (port->changed & (~port->debouncedState)) & GPIOButtonPins;
}

uint8_t
ButtonReleased(tDebouncer *port, uint8_t GPIOButtonPins)
{
    // If the button changed and it changed to a 0, then the
    // user just released the button.
    return (port->changed & port->debouncedState) & GPIOButtonPins;
}

uint8_t 
ButtonDebounceStateGet(tDebouncer *port)
{
    // Restore the debounced state to the normal sense of direction.
    // If a button is high and is pulled down, it is being pressed.
    // If a button is low and is pulled up, it is being pressed.
    // Else, they are not being pressed.
    return ~(port->debouncedState ^ port->pullType);
}
