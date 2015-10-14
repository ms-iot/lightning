//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Public header file for Embedded Peripheral Usermode API
// Documentation Format: http://msdn.microsoft.com/en-us/library/ms177227.aspx
//

#ifndef _EMBEDDED_PERIPHERAL_USER_H_
#define _EMBEDDED_PERIPHERAL_USER_H_

#include <winbase.h>

#ifdef PWM_MAX_DUTYCYCLE
#undef PWM_MAX_DUTYCYCLE
#endif
#define PWM_MAX_DUTYCYCLE 0xffff

#ifdef PWM_RESOLUTION
#undef PWM_RESOLUTION
#endif
#define PWM_RESOLUTION 16

#define EMBAPI __cdecl

#ifdef __cplusplus
extern "C" {
#endif

//
// ADC - Analog to Digital Converters
//

typedef struct _ADC ADC;

HRESULT
EMBAPI
AdcCreateInstance (
    ULONG ConverterIndex,
    _Outptr_ ADC **AdcPPtr);

void
EMBAPI
AdcFree (
    _Post_ptr_invalid_ ADC *Inst);

LONG
EMBAPI
AdcSampleChannel (
    _Inout_ ADC *Inst,
    ULONG ChannelNum);

//
// GPIO
//

// single pin operations
HRESULT
EMBAPI
GpioRead (
    ULONG PinNumber,
    _Out_ LONG *Value);

// 0 = input, 1 = output
HRESULT
EMBAPI
GpioSetDir(
    ULONG PinNumber,
    ULONG Direction);

HRESULT
EMBAPI
GpioWrite (
    ULONG PinNumber,
    ULONG Value);

//
// I2C_CONTROLLER
// 

typedef struct _I2C I2C_CONTROLLER;

HRESULT
EMBAPI
I2CCreateInstance(
    ULONG ControllerIndex,
    ULONG SlaveAddress,
    ULONG ConnectionSpeed,
    _Outptr_ I2C_CONTROLLER **I2CPPtr);

void
EMBAPI
I2CFree(
    _Post_ptr_invalid_ I2C_CONTROLLER *Inst);

HRESULT
EMBAPI
I2CLockController(
    _Inout_ I2C_CONTROLLER *Inst);

HRESULT
EMBAPI
I2CRead(
    _Inout_ I2C_CONTROLLER *Inst,
    _Out_writes_bytes_to_opt_(ReceiveBufferSize, *BytesReturned) void *ReceiveBuffer,
    _In_ DWORD ReceiveBufferSize,
    _Out_opt_ DWORD *BytesReturned);

void
EMBAPI
I2CUnlockController(
    _Inout_ I2C_CONTROLLER *Inst);

HRESULT
EMBAPI
I2CWrite(
    _Inout_ I2C_CONTROLLER *Inst,
    _In_reads_bytes_opt_(SendBufferSize) const void *SendBuffer,
    DWORD SendBufferSize,
    _Out_opt_ DWORD *BytesWritten);

// Does a write, then a read without sending a stop bit between the write and read
HRESULT
EMBAPI
I2CWriteReadAtomic(
    _Inout_ I2C_CONTROLLER *Inst,
    _In_reads_bytes_opt_(SendBufferSize) void *SendBuffer,
    DWORD SendBufferSize,
    _Out_writes_bytes_to_opt_(ReceiveBufferSize, *BytesReturned) void *ReceiveBuffer,
    _In_ DWORD ReceiveBufferSize,
    _Out_opt_ DWORD *BytesReturned);

//
// PWM - Pulse Width Modulation
//

//
// Updates the duty cycle for a pin on which PWM
// has already been started.
//
// DutyCycle - 0 corresponds to 0% duty cycle
//             PWM_MAX_DUTYCYCLE corresponds to 100%
//             duty cycle
//
// Returns an error if PWM is not started on this pin
//
HRESULT
EMBAPI
PwmSetDutyCycle(
    ULONG PinNumber,
    ULONG DutyCycle);

//
// Start PWM on the specified pin. 
//
// The hardware will attempt to match the requested settings
// as closely as possible. Other GPIO operations cannot be
// performed while PWM is started on a pin
//
// FrequencyInHertz - the requested frequency of the PWM signal,
//                    must be greater than 0
//
// DutyCycle - the duty cycle as a proportion from 0 to
//             PWM_MAX_DUTYCYCLE
//
HRESULT
EMBAPI
PwmStart (
    ULONG PinNumber,
    ULONG FrequencyInHertz,
    ULONG DutyCycle);

//
// Stops PWM on the specified pin. The pin will be an OUTPUT
// held LOW after the call to PwmStop.
//
void
EMBAPI
PwmStop (ULONG PinNumber);

//
// SPI_CONTROLLER
//

typedef struct _SPI SPI_CONTROLLER;

typedef struct _SPI_CONTROLLER_CONFIG {
	ULONG ConnectionSpeed;              // clock speed in Hz
	USHORT DataBitLength;               // Data bit length. Supported values: 8, 16 
	USHORT SpiMode;                     // SPI_CONTROLLER Mode. Bit 1 : Clock Polarity (CPOL): 0 or 1
										//           Bit 0 : Clock phase (CPHA): 0 or 1
} SPI_CONTROLLER_CONFIG;

typedef enum _SPI_TRANSFER_FLAG {
	SPI_TRANSFER_FLAG_SEQUENTIAL = 0x1		// do write, then read
} SPI_TRANSFER_FLAG;


HRESULT
EMBAPI
SpiCreateInstance (
    ULONG ControllerIndex,
    _Outptr_ SPI_CONTROLLER **SpiPPtr);

void
EMBAPI
SpiFree (
    _Post_ptr_invalid_ SPI_CONTROLLER *Inst);

HRESULT
EMBAPI
SpiGetControllerConfig (
    _Inout_ SPI_CONTROLLER *Inst,
    _Out_ SPI_CONTROLLER_CONFIG *Config);

HRESULT
EMBAPI
SpiSetControllerConfig(
    _Inout_ SPI_CONTROLLER *Inst,
    const SPI_CONTROLLER_CONFIG *Config);

HRESULT
EMBAPI
SpiTransfer (
    _Inout_ SPI_CONTROLLER *Inst,
    DWORD Flags,
    _In_reads_bytes_opt_(SendBufferSize) void *SendBuffer,
    DWORD SendBufferSize,
    _Out_writes_bytes_opt_(ReceiveBufferSize) void *ReceiveBuffer,
    _In_ DWORD ReceiveBufferSize);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _EMBEDDED_PERIPHERAL_USER_H_
