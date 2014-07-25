
/*************************************************************/
/********************** IMPORTANT NOTE ***********************/
/*************************************************************/
/* Because pin states are altered during the course of these */
/* test, they are expected to be executed from a fresh boot. */
/*************************************************************/

#include "stdafx.h"

#include <iostream>

#include "arduino.h"
#include "spi.h"

unsigned int test_count = 0;
unsigned int success_count = 0;

int _tmain(int argc, _TCHAR* argv[])
{
	return RunArduinoSketch();
}

bool IsSoCPinDataEqual(PIN_DATA &a, PIN_DATA &b) {
    bool equal(true);
    equal &= (a.currentMux == b.currentMux);
    equal &= (a.muxSet == b.muxSet);
    equal &= (a.pinInitialized == b.pinInitialized);
    equal &= (a.pwmDutyCycle == b.pwmDutyCycle);
    equal &= (a.pwmIsEnabled == b.pwmIsEnabled);
    return equal;
}
bool IsCypressPinDataEqual(PIN_DATA &a, PIN_DATA &b) {
    bool equal(true);
    equal &= (a.currentMode == b.currentMode);
    equal &= (a.modeSet == b.modeSet);
    equal &= (a.pinInUseI2c == b.pinInUseI2c);
    equal &= (a.pinInUseSpi == b.pinInUseSpi);
    equal &= (a.pinIsLocked == b.pinIsLocked);
    //equal &= (a.stateIsKnown == b.stateIsKnown);
    equal &= (a.state == b.state);
    equal &= IsSoCPinDataEqual(a, b);
    return equal;
}

void IO0_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = {0};
    _ReadPinConfiguration(0, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO0_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(0, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO0_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(0, INPUT);
    _ReadPinConfiguration(0, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO0_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(0, OUTPUT);
    _ReadPinConfiguration(0, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO0_AnalogWriteFail(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(0, &before);
    _ReadPinMuxConfig(0, &before);
    try {
        analogWrite(0, 128);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(0, &after);
    _ReadPinMuxConfig(0, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO0_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogRead(0);
    _ReadPinMuxConfig(14, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO0_DigitalRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    digitalRead(0);
    _ReadPinMuxConfig(0, &data);
    ::success_count += (success = (data.currentMux == ALTERNATE_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO1_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = {0};
    _ReadPinConfiguration(1, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO1_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(1, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO1_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(1, INPUT);
    _ReadPinConfiguration(1, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO1_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(1, OUTPUT);
    _ReadPinConfiguration(1, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO1_AnalogWriteFail(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(1, &before);
    _ReadPinMuxConfig(1, &before);
    try {
        analogWrite(1, 128);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(1, &after);
    _ReadPinMuxConfig(1, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO1_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogRead(1);
    _ReadPinMuxConfig(15, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}
/*
void IO2_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(2, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}
*/
void IO2_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(2, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO2_SetPinModeToInput(void) {
    //::test_count++;
    //bool success(false);
    //PIN_DATA data = { 0 };
    pinMode(2, INPUT);
    //_ReadPinConfiguration(2, &data);
    //::success_count += (success = (data.currentMode == INPUT));
    //Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO2_SetPinModeToOutput(void) {
    //::test_count++;
    //bool success(false);
    //PIN_DATA data = { 0 };
    pinMode(2, OUTPUT);
    //_ReadPinConfiguration(2, &data);
    //::success_count += (success = (data.currentMode == OUTPUT));
    //Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO2_AnalogWriteFail(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(2, &before);
    _ReadPinMuxConfig(2, &before);
    try {
        analogWrite(2, 128);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(2, &after);
    _ReadPinMuxConfig(2, &after);
    ::success_count += (success = IsSoCPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO2_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogRead(2);
    _ReadPinMuxConfig(16, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}
/*
void IO3_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(3, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}
*/
void IO3_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(3, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO3_SetPinModeToInput(void) {
    //::test_count++;
    //bool success(false);
    //PIN_DATA data = { 0 };
    pinMode(3, INPUT);
    //_ReadPinConfiguration(3, &data);
    //::success_count += (success = (data.currentMode == INPUT));
    //Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO3_SetPinModeToOutput(void) {
    //::test_count++;
    //bool success(false);
    //PIN_DATA data = { 0 };
    pinMode(3, OUTPUT);
    //_ReadPinConfiguration(3, &data);
    //::success_count += (success = (data.currentMode == OUTPUT));
    //Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO3_AnalogWriteHalf(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(3, 128);
    _ReadPinConfiguration(3, &data);
    ::success_count += (success = (data.pwmDutyCycle == 128));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 128, data.pwmDutyCycle); }
}

void IO3_AnalogWriteFull(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(3, 255);
    _ReadPinConfiguration(3, &data);
    ::success_count += (success = (data.pwmDutyCycle == 255));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 255, data.pwmDutyCycle); }
}

void IO3_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogRead(3);
    _ReadPinMuxConfig(17, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO4_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(4, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO4_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(4, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO4_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(4, INPUT);
    _ReadPinConfiguration(4, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO4_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(4, OUTPUT);
    _ReadPinConfiguration(4, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO4_AnalogWriteFail(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(4, &before);
    _ReadPinMuxConfig(4, &before);
    try {
        analogWrite(4, 128);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(4, &after);
    _ReadPinMuxConfig(4, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO4_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogRead(4);
    _ReadPinMuxConfig(18, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO5_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(5, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO5_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(5, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO5_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(5, INPUT);
    _ReadPinConfiguration(5, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO5_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(5, OUTPUT);
    _ReadPinConfiguration(5, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO5_AnalogWriteHalf(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(5, 128);
    _ReadPinConfiguration(5, &data);
    ::success_count += (success = (data.pwmDutyCycle == 128));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 128, data.pwmDutyCycle); }
}

void IO5_AnalogWriteFull(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(5, 255);
    _ReadPinConfiguration(5, &data);
    ::success_count += (success = (data.pwmDutyCycle == 255));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 255, data.pwmDutyCycle); }
}

void IO5_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogRead(5);
    _ReadPinMuxConfig(19, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO6_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(6, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO6_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(6, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO6_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(6, INPUT);
    _ReadPinConfiguration(6, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO6_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(6, OUTPUT);
    _ReadPinConfiguration(6, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO6_AnalogWriteHalf(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(6, 128);
    _ReadPinConfiguration(6, &data);
    ::success_count += (success = (data.pwmDutyCycle == 128));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 128, data.pwmDutyCycle); }
}

void IO6_AnalogWriteFull(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(6, 255);
    _ReadPinConfiguration(6, &data);
    ::success_count += (success = (data.pwmDutyCycle == 255));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 255, data.pwmDutyCycle); }
}

void IO6_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(6, &before);
    _ReadPinMuxConfig(6, &before);
    try {
        analogRead(6);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(6, &after);
    _ReadPinMuxConfig(6, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO7_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(7, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO7_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(7, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO7_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(7, INPUT);
    _ReadPinConfiguration(7, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO7_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(7, OUTPUT);
    _ReadPinConfiguration(7, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO7_AnalogWriteHalf(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(7, 128);
    _ReadPinConfiguration(7, &data);
    ::success_count += (success = (data.pwmDutyCycle == 128));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 128, data.pwmDutyCycle); }
}

void IO7_AnalogWriteFull(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(7, 255);
    _ReadPinConfiguration(7, &data);
    ::success_count += (success = (data.pwmDutyCycle == 255));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 255, data.pwmDutyCycle); }
}

void IO7_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(7, &before);
    _ReadPinMuxConfig(7, &before);
    try {
        analogRead(7);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(7, &after);
    _ReadPinMuxConfig(7, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO8_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(8, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO8_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(8, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO8_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(8, INPUT);
    _ReadPinConfiguration(8, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO8_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(8, OUTPUT);
    _ReadPinConfiguration(8, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO8_AnalogWriteHalf(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(8, 128);
    _ReadPinConfiguration(8, &data);
    ::success_count += (success = (data.pwmDutyCycle == 128));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 128, data.pwmDutyCycle); }
}

void IO8_AnalogWriteFull(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(8, 255);
    _ReadPinConfiguration(8, &data);
    ::success_count += (success = (data.pwmDutyCycle == 255));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 255, data.pwmDutyCycle); }
}

void IO8_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(8, &before);
    _ReadPinMuxConfig(8, &before);
    try {
        analogRead(8);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(8, &after);
    _ReadPinMuxConfig(8, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO9_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(9, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO9_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(9, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO9_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(9, INPUT);
    _ReadPinConfiguration(9, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO9_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(9, OUTPUT);
    _ReadPinConfiguration(9, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO9_AnalogWriteHalf(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(9, 128);
    _ReadPinConfiguration(9, &data);
    ::success_count += (success = (data.pwmDutyCycle == 128));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 128, data.pwmDutyCycle); }
}

void IO9_AnalogWriteFull(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(9, 255);
    _ReadPinConfiguration(9, &data);
    ::success_count += (success = (data.pwmDutyCycle == 255));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 255, data.pwmDutyCycle); }
}

void IO9_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(9, &before);
    _ReadPinMuxConfig(9, &before);
    try {
        analogRead(9);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(9, &after);
    _ReadPinMuxConfig(9, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}
/*
void IO10_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(10, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}
*/
void IO10_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(10, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO10_SetPinModeToInput(void) {
    //::test_count++;
    //bool success(false);
    //PIN_DATA data = { 0 };
    pinMode(10, INPUT);
    //_ReadPinConfiguration(10, &data);
    //::success_count += (success = (data.currentMode == INPUT));
    //Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO10_SetPinModeToOutput(void) {
    //::test_count++;
    //bool success(false);
    //PIN_DATA data = { 0 };
    pinMode(10, OUTPUT);
    //_ReadPinConfiguration(10, &data);
    //::success_count += (success = (data.currentMode == OUTPUT));
    //Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO10_AnalogWriteHalf(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(10, 128);
    _ReadPinConfiguration(10, &data);
    ::success_count += (success = (data.pwmDutyCycle == 128));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 128, data.pwmDutyCycle); }
}

void IO10_AnalogWriteFull(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(10, 255);
    _ReadPinConfiguration(10, &data);
    ::success_count += (success = (data.pwmDutyCycle == 255));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 255, data.pwmDutyCycle); }
}

void IO10_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(10, &before);
    _ReadPinMuxConfig(10, &before);
    try {
        analogRead(10);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(10, &after);
    _ReadPinMuxConfig(10, &after);
    ::success_count += (success = IsSoCPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO11_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(11, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO11_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(11, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO11_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(11, INPUT);
    _ReadPinConfiguration(11, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO11_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(11, OUTPUT);
    _ReadPinConfiguration(11, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO11_AnalogWriteHalf(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(11, 128);
    _ReadPinConfiguration(11, &data);
    ::success_count += (success = (data.pwmDutyCycle == 128));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 128, data.pwmDutyCycle); }
}

void IO11_AnalogWriteFull(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    analogWrite(11, 255);
    _ReadPinConfiguration(11, &data);
    ::success_count += (success = (data.pwmDutyCycle == 255));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (!success) { Log(L"\tExpected:\t%u\n\tActual:\t%u\n", 255, data.pwmDutyCycle); }
}

void IO11_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(11, &before);
    _ReadPinMuxConfig(11, &before);
    try {
        analogRead(11);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(11, &after);
    _ReadPinMuxConfig(11, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO12_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(12, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO12_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(12, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO12_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(12, INPUT);
    _ReadPinConfiguration(12, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO12_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(12, OUTPUT);
    _ReadPinConfiguration(12, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO12_AnalogWriteFail(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(12, &before);
    _ReadPinMuxConfig(12, &before);
    try {
        analogWrite(12, 128);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(12, &after);
    _ReadPinMuxConfig(12, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO12_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(12, &before);
    _ReadPinMuxConfig(12, &before);
    try {
        analogRead(12);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(12, &after);
    _ReadPinMuxConfig(12, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO13_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(13, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO13_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(13, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO13_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(13, INPUT);
    _ReadPinConfiguration(13, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO13_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(13, OUTPUT);
    _ReadPinConfiguration(13, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO13_AnalogWriteFail(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(13, &before);
    _ReadPinMuxConfig(13, &before);
    try {
        analogWrite(13, 128);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(13, &after);
    _ReadPinMuxConfig(13, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO13_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(13, &before);
    _ReadPinMuxConfig(13, &before);
    try {
        analogRead(13);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(13, &after);
    _ReadPinMuxConfig(13, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO14_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(14, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO14_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(14, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO14_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(14, INPUT);
    _ReadPinConfiguration(14, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO14_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(14, OUTPUT);
    _ReadPinConfiguration(14, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO14_AnalogWriteFail(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(14, &before);
    _ReadPinMuxConfig(14, &before);
    try {
        analogWrite(14, 128);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(14, &after);
    _ReadPinMuxConfig(14, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO14_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(14, &before);
    _ReadPinMuxConfig(14, &before);
    try {
        analogRead(14);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(14, &after);
    _ReadPinMuxConfig(14, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO15_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(15, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO15_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(15, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO15_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(15, INPUT);
    _ReadPinConfiguration(15, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO15_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(15, OUTPUT);
    _ReadPinConfiguration(15, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO15_AnalogWriteFail(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(15, &before);
    _ReadPinMuxConfig(15, &before);
    try {
        analogWrite(15, 128);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(15, &after);
    _ReadPinMuxConfig(15, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO15_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(15, &before);
    _ReadPinMuxConfig(15, &before);
    try {
        analogRead(15);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(15, &after);
    _ReadPinMuxConfig(15, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO16_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(16, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO16_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(16, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO16_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(16, INPUT);
    _ReadPinConfiguration(16, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO16_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(16, OUTPUT);
    _ReadPinConfiguration(16, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO16_AnalogWriteFail(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(16, &before);
    _ReadPinMuxConfig(16, &before);
    try {
        analogWrite(16, 128);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(16, &after);
    _ReadPinMuxConfig(16, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO16_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(16, &before);
    _ReadPinMuxConfig(16, &before);
    try {
        analogRead(16);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(16, &after);
    _ReadPinMuxConfig(16, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO17_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(17, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO17_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(17, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO17_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(17, INPUT);
    _ReadPinConfiguration(17, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO17_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(17, OUTPUT);
    _ReadPinConfiguration(17, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO17_AnalogWriteFail(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(17, &before);
    _ReadPinMuxConfig(17, &before);
    try {
        analogWrite(17, 128);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(17, &after);
    _ReadPinMuxConfig(17, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO17_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(17, &before);
    _ReadPinMuxConfig(17, &before);
    try {
        analogRead(17);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(17, &after);
    _ReadPinMuxConfig(17, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO18_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(18, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO18_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(18, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO18_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(18, INPUT);
    _ReadPinConfiguration(18, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO18_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(18, OUTPUT);
    _ReadPinConfiguration(18, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO18_AnalogWriteFail(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(18, &before);
    _ReadPinMuxConfig(18, &before);
    try {
        analogWrite(18, 128);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(18, &after);
    _ReadPinMuxConfig(18, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO18_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(18, &before);
    _ReadPinMuxConfig(18, &before);
    try {
        analogRead(18);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(18, &after);
    _ReadPinMuxConfig(18, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO19_CypressInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinConfiguration(19, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO19_MuxInitializationCheck(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    _ReadPinMuxConfig(19, &data);
    ::success_count += (success = (data.currentMux == DEFAULT_MUX));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO19_SetPinModeToInput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(19, INPUT);
    _ReadPinConfiguration(19, &data);
    ::success_count += (success = (data.currentMode == INPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO19_SetPinModeToOutput(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA data = { 0 };
    pinMode(19, OUTPUT);
    _ReadPinConfiguration(19, &data);
    ::success_count += (success = (data.currentMode == OUTPUT));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO19_AnalogWriteFail(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(19, &before);
    _ReadPinMuxConfig(19, &before);
    try {
        analogWrite(19, 128);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(19, &after);
    _ReadPinMuxConfig(19, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void IO19_AnalogRead(void) {
    ::test_count++;
    bool success(false);
    PIN_DATA before = { 0 }, after = { 0 };
    _ReadPinConfiguration(19, &before);
    _ReadPinMuxConfig(19, &before);
    try {
        analogRead(19);
    }
    catch (_arduino_fatal_error e) {}
    _ReadPinConfiguration(19, &after);
    _ReadPinMuxConfig(19, &after);
    ::success_count += (success = IsCypressPinDataEqual(before, after));
    Log(L"%s | %s\n", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
}

void SPI_BeginToEnd(void) {
    ::test_count++;
    bool success(true);
    size_t failure_line(0);
    PIN_DATA before[3] = { 0 }, after[3] = { 0 };

    _ReadPinConfiguration(11, &before[0]);
    _ReadPinMuxConfig(11, &before[0]);
    _ReadPinConfiguration(12, &before[1]);
    _ReadPinMuxConfig(12, &before[1]);
    _ReadPinConfiguration(13, &before[2]);
    _ReadPinMuxConfig(13, &before[2]);

    success &= (before[0].currentMux == DEFAULT_MUX);
    if (!failure_line && !success) { failure_line = __LINE__; }
    success &= (before[1].currentMux == DEFAULT_MUX);
    if (!failure_line && !success) { failure_line = __LINE__; }
    success &= (before[2].currentMux == DEFAULT_MUX);
    if (!failure_line && !success) { failure_line = __LINE__; }

    SPI.begin();

    _ReadPinConfiguration(11, &after[0]);
    _ReadPinMuxConfig(11, &after[0]);
    _ReadPinConfiguration(12, &after[1]);
    _ReadPinMuxConfig(12, &after[1]);
    _ReadPinConfiguration(13, &after[2]);
    _ReadPinMuxConfig(13, &after[2]);

    success &= (after[0].currentMux == ALTERNATE_MUX);
    if (!failure_line && !success) { failure_line = __LINE__; }
    success &= (after[1].currentMux == ALTERNATE_MUX);
    if (!failure_line && !success) { failure_line = __LINE__; }
    success &= (after[2].currentMux == ALTERNATE_MUX);
    if (!failure_line && !success) { failure_line = __LINE__; }

    SPI.end();

    _ReadPinConfiguration(11, &after[0]);
    _ReadPinMuxConfig(11, &after[0]);
    _ReadPinConfiguration(12, &after[1]);
    _ReadPinMuxConfig(12, &after[1]);
    _ReadPinConfiguration(13, &after[2]);
    _ReadPinMuxConfig(13, &after[2]);

    success &= IsSoCPinDataEqual(before[0], after[0]);
    if (!failure_line && !success) { failure_line = __LINE__; }
    success &= IsSoCPinDataEqual(before[1], after[1]);
    if (!failure_line && !success) { failure_line = __LINE__; }
    success &= IsSoCPinDataEqual(before[2], after[2]);
    if (!failure_line && !success) { failure_line = __LINE__; }

    success &= (after[0].currentMux == DEFAULT_MUX);
    if (!failure_line && !success) { failure_line = __LINE__; }
    success &= (after[1].currentMux == DEFAULT_MUX);
    if (!failure_line && !success) { failure_line = __LINE__; }
    success &= (after[2].currentMux == DEFAULT_MUX);
    if (!failure_line && !success) { failure_line = __LINE__; }

    ::success_count += success;
    Log(L"%s | %s", (success ? L"passed" : L"FAILED"), __FUNCTIONW__);
    if (failure_line) { Log(L" (first error on line %lu)\n", failure_line); }
    else { Log(L"\n"); }
}

void setup(void) {
    IO0_CypressInitializationCheck();
    IO1_CypressInitializationCheck();
    //IO2_CypressInitializationCheck();  // This tests is based on the unreadable state of the related SoC pin
    //IO3_CypressInitializationCheck();  // This tests is based on the unreadable state of the related SoC pin
    IO4_CypressInitializationCheck();
    IO5_CypressInitializationCheck();
    IO6_CypressInitializationCheck();
    IO7_CypressInitializationCheck();
    IO8_CypressInitializationCheck();
    IO9_CypressInitializationCheck();
    //IO10_CypressInitializationCheck();  // This tests is based on the unreadable state of the related SoC pin
    IO11_CypressInitializationCheck();
    IO12_CypressInitializationCheck();
    IO13_CypressInitializationCheck();
    IO14_CypressInitializationCheck();
    IO15_CypressInitializationCheck();
    IO16_CypressInitializationCheck();
    IO17_CypressInitializationCheck();
    IO18_CypressInitializationCheck();
    IO19_CypressInitializationCheck();

    IO0_MuxInitializationCheck();
    IO1_MuxInitializationCheck();
    IO2_MuxInitializationCheck();
    IO3_MuxInitializationCheck();
    IO4_MuxInitializationCheck();
    IO5_MuxInitializationCheck();
    IO6_MuxInitializationCheck();
    IO7_MuxInitializationCheck();
    IO8_MuxInitializationCheck();
    IO9_MuxInitializationCheck();
    IO10_MuxInitializationCheck();
    IO11_MuxInitializationCheck();
    IO12_MuxInitializationCheck();
    IO13_MuxInitializationCheck();
    IO14_MuxInitializationCheck();
    IO15_MuxInitializationCheck();
    IO16_MuxInitializationCheck();
    IO17_MuxInitializationCheck();
    IO18_MuxInitializationCheck();
    IO19_MuxInitializationCheck();
    
    IO0_SetPinModeToOutput();
    IO1_SetPinModeToOutput();
    IO2_SetPinModeToOutput();
    IO3_SetPinModeToOutput();
    IO4_SetPinModeToOutput();
    IO5_SetPinModeToOutput();
    IO6_SetPinModeToOutput();
    IO7_SetPinModeToOutput();
    IO8_SetPinModeToOutput();
    IO9_SetPinModeToOutput();
    IO10_SetPinModeToOutput();
    IO11_SetPinModeToOutput();
    IO12_SetPinModeToOutput();
    IO13_SetPinModeToOutput();
    IO14_SetPinModeToOutput();
    IO15_SetPinModeToOutput();
    IO16_SetPinModeToOutput();
    IO17_SetPinModeToOutput();
    IO18_SetPinModeToOutput();
    IO19_SetPinModeToOutput();

    IO0_AnalogWriteFail();
    IO1_AnalogWriteFail();
    IO2_AnalogWriteFail();
    IO3_AnalogWriteHalf();
    IO3_AnalogWriteFull();
    IO4_AnalogWriteFail();
    IO5_AnalogWriteHalf();
    IO5_AnalogWriteFull();
    IO6_AnalogWriteHalf();
    IO6_AnalogWriteFull();
    IO7_AnalogWriteHalf();
    IO7_AnalogWriteFull();
    IO8_AnalogWriteHalf();
    IO8_AnalogWriteFull();
    IO9_AnalogWriteHalf();
    IO9_AnalogWriteFull();
    IO10_AnalogWriteHalf();
    IO10_AnalogWriteFull();
    IO11_AnalogWriteHalf();
    IO11_AnalogWriteFull();
    IO12_AnalogWriteFail();
    IO13_AnalogWriteFail();
    IO14_AnalogWriteFail();
    IO15_AnalogWriteFail();
    IO16_AnalogWriteFail();
    IO17_AnalogWriteFail();
    IO18_AnalogWriteFail();
    IO19_AnalogWriteFail();

    IO0_SetPinModeToInput();
    IO1_SetPinModeToInput();
    IO2_SetPinModeToInput();
    IO3_SetPinModeToInput();
    IO4_SetPinModeToInput();
    IO5_SetPinModeToInput();
    IO6_SetPinModeToInput();
    IO7_SetPinModeToInput();
    IO8_SetPinModeToInput();
    IO9_SetPinModeToInput();
    IO10_SetPinModeToInput();
    IO11_SetPinModeToInput();
    IO12_SetPinModeToInput();
    IO13_SetPinModeToInput();
    IO14_SetPinModeToInput();
    IO15_SetPinModeToInput();
    IO16_SetPinModeToInput();
    IO17_SetPinModeToInput();
    IO18_SetPinModeToInput();
    IO19_SetPinModeToInput();
    
    //IO0_AnalogRead();  // Calling AnalogRead() only temporarily modifies the mux, and cannot be tested automatically
    //IO1_AnalogRead();
    //IO2_AnalogRead();
    //IO3_AnalogRead();
    //IO4_AnalogRead();
    //IO5_AnalogRead();
    IO6_AnalogRead();
    IO7_AnalogRead();
    IO8_AnalogRead();
    IO9_AnalogRead();
    IO10_AnalogRead();
    IO11_AnalogRead();
    IO12_AnalogRead();
    IO13_AnalogRead();
    IO14_AnalogRead();
    IO15_AnalogRead();
    IO16_AnalogRead();
    IO17_AnalogRead();
    IO18_AnalogRead();
    IO19_AnalogRead();

    SPI_BeginToEnd();

    Log(L"\n%u/%u TEST PASSED\n", ::success_count, ::test_count);
}

void loop(void) {
    _exit_arduino_loop();
}
