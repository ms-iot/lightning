// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "arduino.h"

using namespace std;

//! Tracks interrupts assigned to a specific pin
// Used to track pin number, mode, state and callback function.
struct InterruptTracker
{
    int pin;
    int mode;
    int lastState;
    InterruptFunction fxn;

    InterruptTracker(int p, InterruptFunction f, int m)
    : pin(p)
    , fxn(f)
    , mode(m)
    {
        // snap the state of the pin at create time.
        lastState = digitalRead(pin);
    }

    ~InterruptTracker()
    {
    }

    void handle()
    {
        int currentState = digitalRead(pin);

        switch (mode)
        {
            // Callback is executed as long as the condition is low
            case LOW:
            if (currentState == LOW)
            {
                fxn();
            }
            break;

            // Callback is executed when the pin changes state
            case CHANGE:
            if (currentState != lastState)
            {
                fxn();
            }
            break;
            
            // Callback is executed when the pin changes from LOW to HIGH
            case RISING:
            if (lastState == LOW && currentState == HIGH)
            {
                fxn();
            }
            break;

            // Callback is executed when the pin changes from HIGH to LOW
            case FALLING:
            if (lastState == HIGH && currentState == LOW)
            {
                fxn();
            }
            break;
        }

        lastState = currentState;
    }
};

typedef std::map<int, shared_ptr<InterruptTracker> > InterruptPinMap;
static InterruptPinMap s_interruptMap;
static HANDLE s_sharedInterruptTimer = INVALID_HANDLE_VALUE;

//! At a fixed frequency (INTERRUPT_FREQUENCY), this callback will iterate through outstanding fake 'interrupts'
//! test the condition and call the callback.
static void CALLBACK InterruptTimerHandler(void* arg, DWORD, DWORD)
{
    UNREFERENCED_PARAMETER(arg);
    // During an interrupt handler, the caller can call detachInterrupt which modifies the list.
    // To handle this, we'll copy the list.
    std::vector<shared_ptr<InterruptTracker>> list;

    for (auto p : s_interruptMap)
    {
        list.push_back(p.second);
    }

    for (auto tracker : list)
    {
        tracker->handle();
    }
}

void attachInterrupt(uint8_t pin, InterruptFunction fxn, int mode)
{
    if (s_sharedInterruptTimer == INVALID_HANDLE_VALUE)
    {
        s_sharedInterruptTimer = CreateWaitableTimerEx(NULL, NULL, 0, TIMER_ALL_ACCESS);

        LARGE_INTEGER li = { 0 };
        if (SetWaitableTimer(s_sharedInterruptTimer, &li, INTERRUPT_FREQUENCY, InterruptTimerHandler, nullptr, FALSE) == 0)
        {
            DWORD err = GetLastError();
            ThrowError(HRESULT_FROM_WIN32(err) , "Error setting timer for interrupts: %d", err);
        }
    }

    auto it = s_interruptMap.find(pin);
    if (it != s_interruptMap.end())
    {
        // Changing mode or function?
        it->second->fxn = fxn;
        it->second->mode = mode;
    }
    else
    {
        s_interruptMap[pin] = make_shared<InterruptTracker>(pin, fxn, mode);
    }
}

void detachInterrupt(uint8_t pin)
{
    s_interruptMap.erase(pin);

    if (s_interruptMap.size() == 0)
    {
        if (s_sharedInterruptTimer != INVALID_HANDLE_VALUE)
        {
            CancelWaitableTimer(s_sharedInterruptTimer);
            CloseHandle(s_sharedInterruptTimer);
            s_sharedInterruptTimer = INVALID_HANDLE_VALUE;
        }
    }
}

