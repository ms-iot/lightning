// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _I2C_CONTROLLER_H_
#define _I2C_CONTROLLER_H_

#include <Windows.h>

#include "PinSupport.h"
#include "DmapSupport.h"

#define CMD_WRITE 0
#define CMD_READ 1

//
// Class that is used to interact with the I2C Controller hardware.
//
class I2cControllerClass
{
public:
    I2cControllerClass()
    {
        m_hController = INVALID_HANDLE_VALUE;
        m_controller = nullptr;
    }

    virtual ~I2cControllerClass()
    {
        this->endExternal();
    }

    // This method maps the I2C controller if needed.
    inline BOOL mapIfNeeded()
    {
        if (m_hController == INVALID_HANDLE_VALUE)
        {
            return _mapController();
        }
        else
        {
            return TRUE;
        }
    }

    // This method is used to prepare to use the I2C bus to communicate with devices
    // that are external to the board.
    BOOL beginExternal()
    {
        BOOL status = TRUE;
        ULONG error = ERROR_SUCCESS;


        // Set the MUXes for external I2C use.
        status = _setPinI2c(PIN_I2C_CLK);
        if (!status) { status = GetLastError(); }

        if (status)
        {
            status = _setPinI2c(PIN_I2C_DAT);
            if (!status) { status = GetLastError(); }
        }

        if (!status)
        {
            SetLastError(error);
        }
        return status;
    }

    void endExternal()
    {
        // Set the pns used for I2C back to Digital inputs, on a best effort basis.
        _setPinFunction(PIN_I2C_DAT, FUNC_DIO);
        _setPinMode(PIN_I2C_DAT, INPUT, false);
        _setPinFunction(PIN_I2C_CLK, FUNC_DIO);
        _setPinMode(PIN_I2C_CLK, INPUT, false);
    }

    //
    // I2C Controller accessor methods.  These methods assume the I2C Controller
    // has already been mapped using mapIfNeeded().
    //

    inline BOOL masterModeIsSet()
    {
        return (m_controller->IC_CON.MASTER_MODE == 1);
    }

    inline void setMasterMode()         // Only valid to set it, slave mode not supported.
    {
        m_controller->IC_CON.MASTER_MODE = 1;
    }

    inline BOOL controllerIsEnabled()
    {
        return (m_controller->IC_ENABLE.ENABLE == 1);
    }

    inline void enableController()
    {
        m_controller->IC_ENABLE.ENABLE = 1;
    }

    inline void disableController()
    {
        m_controller->IC_ENABLE.ENABLE = 0;
    }

    inline BOOL isActive()
    {
        return (m_controller->IC_ENABLE_STATUS.IC_EN == 1);
    }

    inline void setStandardSpeed()
    {
        m_controller->IC_CON.SPEED = 1;
    }

    inline void setFastSpeed()
    {
        m_controller->IC_CON.SPEED = 2;
    }

    inline void enableRestartSupport()
    {
        m_controller->IC_CON.IC_RESTART_EN = 1;
    }

    inline void disableRestartSupport()
    {
        m_controller->IC_CON.IC_RESTART_EN = 0;
    }

    inline void set7bitAddressing()
    {
        m_controller->IC_CON.IC_10BITADDR_MASTER = 0;
    }

    inline void setAddress(ULONG adr)       // Supports 7 or 10 bit addressing
    {
        m_controller->IC_TAR.IC_TAR = (adr & 0x3FF);
    }

    inline BOOL txFifoNotFull()
    {
        return (m_controller->IC_STATUS.TFNF == 1);
    }

    inline BOOL txFifoFull()
    {
        return (m_controller->IC_STATUS.TFNF == 0);
    }

    inline BOOL txFifoEmpty()
    {
        return (m_controller->IC_STATUS.TFE == 1);
    }

    inline BOOL rxFifoNotEmtpy()
    {
        return (m_controller->IC_STATUS.RFNE == 1);
    }

    inline BOOL rxFifoEmpty()
    {
        return (m_controller->IC_STATUS.RFNE == 0);
    }

    inline void issueCmd(ULONG cmd)
    {
        m_controller->IC_DATA_CMD.ALL_BITS = cmd;
    }

    inline UCHAR readByte()
    {
        return (m_controller->IC_DATA_CMD.DAT);
    }

    inline void maskAllInterrupts()
    {
        m_controller->IC_INTR_MASK.ALL_BITS = 0;
    }

    inline void clearAllInterrupts()
    {
        ULONG dummy = m_controller->IC_CLR_INTR.ALL_BITS;
    }

    //
    // Structures used to map and access the I2C Controller registers.
    // These must agree with the actual hardware!
    //

    #pragma warning(push)
    #pragma warning(disable : 4201) // Ignore nameless struct/union warnings

    // I2C Control Register.
    typedef union {
        struct {
            ULONG MASTER_MODE : 1;          // 0: master disabled, 1: master enabled
            ULONG SPEED : 2;                // 01: standard (100 kbit/s), 10: fast (400 kbit/s)
            ULONG _rsv1 : 1;                // Reserved
            ULONG IC_10BITADDR_MASTER : 1;  // 0: start transfers with 7-bit addressing, 1: use 10-bit
            ULONG IC_RESTART_EN : 1;        // 0: disable restars, 1: allow restarts
            ULONG _rsv2 : 26;               // Reserved
        };
        ULONG ALL_BITS;
    } _IC_CON;

    // I2C Master Target Address Register.
    typedef union {
        struct {
            ULONG IC_TAR : 10;              // Target slave address for an I2C transaction
            ULONG GC_OR_START : 1;          // Reserved - see Quark datasheet 9.5.1.27
            ULONG SPECIAL : 1;              // Reserved - see Quark datasheet 9.5.1.27
            ULONG _rsv : 20;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_TAR;

    // I2C Data Buffer and Command Register.
    typedef union {
        struct {
            ULONG DAT : 8;                  // Data to be transmitted or received on the I2C bus
            ULONG CMD : 1;                  // 0: transmit the data, 1: receive a byte of data
            ULONG STOP : 1;                 // 0: do not issue STOP, 1: issue STOP after byte transfer
            ULONG RESTART : 1;              // 0: RESTART only if direction change, 1: RESTART before xfr
            ULONG _rsv : 21;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_DATA_CMD;

    // I2C Standard Speed SCL High Count Register.
    typedef union {
        struct {
            ULONG IC_SS_SCL_HCNT : 16;      // 6-65525, I2C clock high period count
            ULONG _rsv : 16;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_SS_SCL_HCNT;

    // I2C Standard Speed SCL Low Count Register.
    typedef union {
        struct {
            ULONG IC_SS_SCL_LCNT : 16;      // 8-65535, I2C clock low period count
            ULONG _rsv : 16;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_SS_SCL_LCNT;

    // I2C Fast Speed SCL High Count Register.
    typedef union {
        struct {
            ULONG IC_FS_SCL_HCNT : 16;      // 6-65535, I2C clock high period count
            ULONG _rsv : 16;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_FS_SCL_HCNT;

    // I2C Fast Speed SCL Low Count Register.
    typedef union {
        struct {
            ULONG IC_FS_SCL_LCNT : 16;      // 8-65535, I2C clock low period count.
            ULONG _rsv : 16;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_FS_SCL_LCNT;

    // I2C Interrupt Status Register.
    typedef union {
        struct {
            ULONG R_RX_UNDER : 1;           // Set if read buffer is read when it is empty
            ULONG R_RX_OVER : 1;            // Set if byte received when RX FIFO is full
            ULONG R_RX_FULL : 1;            // Set if RX FIFO reaches IC_RX_TL threshold
            ULONG R_TX_OVER : 1;            // Set if byte written when TX FIFO is full
            ULONG R_TX_EMPTY : 1;           // Set with TX FIFO is at or below IC_TX_TL threshold
            ULONG R_RD_REQ : 1;             // Slave function, unused here
            ULONG R_TX_ABRT : 1;            // Set if TX fails and can't complete
            ULONG _rsv1 : 1;                // Reserved
            ULONG R_ACTIVITY : 1;           // Set after there has been I2C activity
            ULONG R_STOP_DET : 1;           // Set when a STOP condition has occurred
            ULONG R_START_DET : 1;          // Set when a START or RESTART condition has occurred
            ULONG _rsv2 : 21;               // Reserved
        };
        ULONG ALL_BITS;
    } _IC_INTR_STAT;

    // I2C Interrupt Mask Register.
    typedef union {
        struct {
            ULONG M_RX_UNDER : 1;           // 0: Disable RX Underflow status and interrupt
            ULONG M_RX_OVER : 1;            // 0: Disable RX Overflow status and interrupt
            ULONG M_RX_FULL : 1;            // 0: Disable RX Full status and interrupt
            ULONG M_TX_OVER : 1;            // 0: Disable TX Overflow status and interrupt
            ULONG M_TX_EMPTY : 1;           // 0: Disable TX Empty status and interrupt
            ULONG _rsv1 : 1;                // Reserved
            ULONG M_TX_ABRT : 1;            // 0: Disable TX Abort status and interrupt
            ULONG _rsv2 : 1;                // Reserved
            ULONG M_ACTIVITY : 1;           // 0: Disable I2C Activity status and interrupt
            ULONG M_STOP_DET : 1;           // 0: Disable STOP Condition status and interrupt
            ULONG M_START_DET : 1;          // 0: Disable START/RESTART Conditon status and interrupt
            ULONG _rsv3 : 21;               // Reserved
        };
        ULONG ALL_BITS;
    } _IC_INTR_MASK;

    // I2C Raw Interrupt Status Register.
    typedef union {
        struct {
            ULONG RX_UNDER : 1;             // 0: Unmasked RX Underflow status
            ULONG RX_OVER : 1;              // 0: Unmasked RX Overflow status
            ULONG RX_FULL : 1;              // 0: Unmasked RX Full status
            ULONG TX_OVER : 1;              // 0: Unmasked TX Overflow status
            ULONG TX_EMPTY : 1;             // 0: Unmasked TX Empty status
            ULONG _rsv1 : 1;                // Reserved
            ULONG TX_ABRT : 1;              // 0: Unmasked TX Abort status
            ULONG _rsv2 : 1;                // Reserved
            ULONG ACTIVITY : 1;             // 0: Unmasked I2C Activity status
            ULONG STOP_DET : 1;             // 0: Unmasked STOP Condition status
            ULONG START_DET : 1;            // 0: Unmasked START/RESTART Conditon status
            ULONG _rsv3 : 21;               // Reserved
        };
        ULONG ALL_BITS;
    } _IC_RAW_INTR_STAT;

    // I2C Receive FIFO Threshold Level Register.
    typedef union {
        struct {
            ULONG IC_RX_TL : 8;             // 0-FIFO_LEN, threshold is IC_RX_TL + 1
            ULONG _rsv : 24;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_RX_TL;

    // I2C Transmit FIFO Threshold Level Register.
    typedef union {
        struct {
            ULONG IC_TX_TL : 8;             // 0-FIFO_LEN, threshold is IC_TX_TL value
            ULONG _rsv : 24;                // Rserved
        };
        ULONG ALL_BITS;
    } _IC_TX_TL;

    // I2C Clear Combined and Individual Interrupt Register.
    typedef union {
        struct {
            ULONG CLR_INTR : 1;             // Read to clear all software clearable interrupts
            ULONG _rsv : 31;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_CLR_INTR;

    // I2C Clear RX_UNDER Interrupt Register.
    typedef union {
        struct {
            ULONG CLR_RX_UNDER : 1;         // Read to clear RX_UNDER interrupt status
            ULONG _rsv : 31;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_CLR_RX_UNDER;

    // I2C Clear RX_OVER Interrupt Register.
    typedef union {
        struct {
            ULONG CLR_RX_OVER : 1;          // Read to clear RX_OVER interrupt status
            ULONG _rsv : 31;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_CLR_RX_OVER;

    // I2C Clear TX_OVER Interrupt Register.
    typedef union {
        struct {
            ULONG CLR_TX_OVER : 1;          // Read to clear TX_OVER interrupt status
            ULONG _rsv : 31;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_CLR_TX_OVER;

    // I2C Clear RD_REQ Interrupt Register.
    typedef union {
        struct {
            ULONG CLR_RD_REQ : 1;           // Read to clear RD_REQ interrupt status
            ULONG _rsv : 31;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_CLR_RD_REQ;

    // I2C Clear TX_ABRT Interrupt Register.
    typedef union {
        struct {
            ULONG CLR_TX_ABRT : 1;          // Read to clear TX_ABRT interrupt status
            ULONG _rsv : 31;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_CLR_TX_ABRT;

    // I2C Clear ACTIVITY Interrupt Register.
    typedef union {
        struct {
            ULONG CLR_ACTIVITY : 1;         // Read to clear ACTIVITY interrupt status
            ULONG _rsv : 31;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_CLR_ACTIVITY;

    // I2C Clear STOP_DET Interrupt Register.
    typedef union {
        struct {
            ULONG CLR_STOP_DET : 1;         // Read to clear STOP_DET interrupt status
            ULONG _rsv : 31;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_CLR_STOP_DET;

    // I2C Clear START_DET Interrupt Register.
    typedef union {
        struct {
            ULONG CLR_START_DET : 1;        // Read to clear START_DET interrupt status
            ULONG _rsv : 31;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_CLR_START_DET;

    // I2C Controller Enable Register.
    typedef union {
        struct {
            ULONG ENABLE : 1;               // 0: I2C Controller Disabled, 1: Enabled
            ULONG _rsv : 31;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_ENABLE;

    // I2C Controller Status Register.
    typedef union {
        struct {
            ULONG ACTIVITY : 1;             // 0: Not active, 1: Active
            ULONG TFNF : 1;                 // 0: TX FIFO is full, 1: TX FIFO is not full
            ULONG TFE : 1;                  // 0: TX FIFO is not empty, 1: TX FIFO is empty
            ULONG RFNE : 1;                 // 0: RX FIFO is empty, 1: RX FIFO is not empty
            ULONG RFF : 1;                  // 0: RX FIFO is not full, 1: RX FIFO is full
            ULONG MST_ACTIVITY : 1;         // 0: Master state machine is idle, 1: Active
            ULONG _rsv : 26;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_STATUS;

    // I2C Transmit FIFO Level Register.
    typedef union {
        struct {
            ULONG TXFLR : 5;                // Count of valid data entries in TX FIFO
            ULONG _rsv : 27;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_TXFLR;

    // I2C Receive FIFO Level Register.
    typedef union {
        struct {
            ULONG RXFLR : 5;                // Count of valid data entries in RX FIFO
            ULONG _rsv : 27;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_RXFLR;

    // I2C SDA Hold Time Register.
    typedef union {
        struct {
            ULONG IC_SDA_HOLD : 16;         // 1-65535, I2C clock periods to hold data after SCL drops
            ULONG _rsv : 16;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_SDA_HOLD;

    // I2C Transmit Abort Source Register.
    typedef union {
        struct {
            ULONG ABRT_7B_ADDR_NOACK : 1;   // Set if in 7-bit addr mode and addr not acked
            ULONG ABRT_10ADDR1_NOACK : 1;   // Set if 10-bit addr mode, and 1st addr byte not acked
            ULONG ABRT_10ADDR2_NOACK : 1;   // Set if 10-bit addr mode, and 2nd addr byte not acked
            ULONG ABRT_TXDATA_NOACK : 1;    // Set if addr was acked, but data was not acked
            ULONG _rsv1 : 3;                // Reserved
            ULONG ABRT_SBYTE_ACKDET : 1;    // Set if a start byte was sent, and a slave acked it
            ULONG _rsv2 : 1;                // Reserved
            ULONG ABRT_SBYTE_NORSTRT : 1;   // Set if start byte send with restart disabled
            ULONG ABRT_10B_RD_NORSTRT : 1;  // Set if 10-bit address read sent with restart disabled
            ULONG ABRT_MASTER_DIS : 1;      // Set if Master operation attempted with Master mode disabled
            ULONG ARB_LOST : 1;             // Set if master has lost arbitration
            ULONG _rsv3 : 19;               // Reserved
        };
        ULONG ALL_BITS;
    } _IC_TX_ABRT_SOURCE;

    // I2C Enable Status Register.
    typedef union {
        struct {
            ULONG IC_EN : 1;                // 0: Controller is inactive, 1: Controller active
            ULONG _rsv : 31;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_ENABLE_STATUS;

    // Standerd Speed and Full Speed Spike Suppression Limit Register.
    typedef union {
        struct {
            ULONG IC_FS_SPKLENRX_TL : 8;    // 2-255, max I2C clock periods to reject spike
            ULONG _rsv : 24;                // Reserved
        };
        ULONG ALL_BITS;
    } _IC_FS_SPKLEN;

    #pragma warning( pop )

    // Laoyout of the Quark I2C Controller registers in memory.
    typedef struct _I2C_CONTROLLER {
        volatile _IC_CON            IC_CON;             // 0x00 - Control Register
        volatile _IC_TAR            IC_TAR;             // 0x04 - Master Target Address
        ULONG                       _reserved1[2];      // 0x08 - 0x0F
        volatile _IC_DATA_CMD       IC_DATA_CMD;        // 0x10 - Data Buffer and Command
        volatile _IC_SS_SCL_HCNT    IC_SS_SCL_HCNT;     // 0x14 - Standard Speed Clock SCL High Count
        volatile _IC_SS_SCL_LCNT    IC_SS_SCL_LCNT;     // 0x18 - Standard Speed Clock SCL Low Count
        volatile _IC_FS_SCL_HCNT    IC_FS_SCL_HCNT;     // 0x1C - Fast Speed Clock Scl High Count
        volatile _IC_FS_SCL_LCNT    IC_FS_SCL_LCNT;     // 0x20 - Fast Speed Clock Scl Low Count
        ULONG                       _reserved2[2];      // 0x24 - 0x2B
        volatile _IC_INTR_STAT      IC_INTR_STAT;       // 0x2C - Interrupt Status
        volatile _IC_INTR_MASK      IC_INTR_MASK;       // 0x30 - Interrupt Mask
        volatile _IC_RAW_INTR_STAT  IC_RAW_INTR_STAT;   // 0x34 - Raw Interrupt Status
        volatile _IC_RX_TL          IC_RX_TL;           // 0x38 - Receive FIFO Threshold Level
        volatile _IC_TX_TL          IC_TX_TL;           // 0x3C - Transmit FIFO Threshold Level
        volatile _IC_CLR_INTR       IC_CLR_INTR;        // 0x40 - Clear Combined and Individual Interrupt
        volatile _IC_CLR_RX_UNDER   IC_CLR_RX_UNDER;    // 0x44 - Clear RX_UNDER Interrupt
        volatile _IC_CLR_RX_OVER    IC_CLR_RX_OVER;     // 0x48 - Clear RX_OVER Interrupt
        volatile _IC_CLR_TX_OVER    IC_CLR_TX_OVER;     // 0x4C - Clear TX_OVER Interrupt
        volatile _IC_CLR_RD_REQ     IC_CLR_RD_REQ;      // 0x50 - Clear RD_REQ Interrupt
        volatile _IC_CLR_TX_ABRT    IC_CLR_TX_ABRT;     // 0x54 - Clear TX_ABRT Interrupt
        ULONG                       _reserved3;         // 0x58 - 0x5B
        volatile _IC_CLR_ACTIVITY   IC_CLR_ACTIVITY;    // 0x5C - Clear ACTIVITY Interrupt
        volatile _IC_CLR_STOP_DET   IC_CLR_STOP_DET;    // 0x60 - Clear STOP_DET Interrupt
        volatile _IC_CLR_START_DET  IC_CLR_START_DET;   // 0x64 - Clear START_DET Interrupt
        ULONG                       _reserved4;         // 0x68 - 0x6B
        volatile _IC_ENABLE         IC_ENABLE;          // 0x6C - Enable
        volatile _IC_STATUS         IC_STATUS;          // 0x70 - Status
        volatile _IC_TXFLR          IC_TXFLR;           // 0x74 - Transmit FIFO Level
        volatile _IC_RXFLR          IC_RXFLR;           // 0x78 - Receive FIFO Level
        volatile _IC_SDA_HOLD       IC_SDA_HOLD;        // 0x7C - SDA Hold
        volatile _IC_TX_ABRT_SOURCE IC_TX_ABRT_SOURCE;  // 0x80 - Transmit Abort Source
        ULONG                       _reserved5[6];      // 0x84 - 0x9B
        volatile _IC_ENABLE_STATUS  IC_ENABLE_STATUS;   // 0x9C - Enable Status
        volatile _IC_FS_SPKLEN      IC_FS_SPKLEN;       // 0xA0 - SS and FS Spike Suppression Limit
    } I2C_CONTROLLER, *PI2C_CONTROLLER;

private:

    // Method to map the I2C controller into this process' virtual address space.
    BOOL _mapController()
    {
        BOOL status = TRUE;
        BOOL error = ERROR_SUCCESS;
        PVOID baseAddress = nullptr;

        status = GetControllerBaseAddress(  dmapI2cDeviceName,
                                            m_hController,
                                            baseAddress,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE );
        if (!status)
        {
            error = GetLastError();
        }
        else
        {
            m_controller = (PI2C_CONTROLLER)baseAddress;
        }

        if (!status) { SetLastError(error); }
        return status;
    }

    // Handle to the device that can be used to map I2C controller registers
    // into user-mode address space.
    HANDLE m_hController;

    // Pointer to the object used to address the I2C Controller registers after
    // they are mapped into this process' address space.
    PI2C_CONTROLLER m_controller;
};

__declspec (selectany) I2cControllerClass g_i2c;

//
// Here, "transfer" is used to mean reading or writing one or more bytes within a
// transaction, in a single direction.
// Transfers are of type WRITE unless they are explicitely set to be a read transfer.
//
class I2cTransferClass
{
public:
    I2cTransferClass()
    {
        clear();
    }

    virtual ~I2cTransferClass()
    {
    }

    // Prepare to use or re-use this object.
    inline void clear()
    {
        m_pNextXfr = nullptr;
        m_pBuffer = nullptr;
        m_bufBytes = 0;
        m_isRead = FALSE;
        m_preRestart = FALSE;
        resetCmd();
        resetRead();
    }
    
    // Prepare to use this object to step through buffer for command processing.
    inline void resetCmd()
    {
        m_nextCmd = 0;
        m_lastCmdFetched = FALSE;
    }

    // Prepare to use this object to step through buffer for read processing.
    inline void resetRead()
    {
        m_nextRead = 0;
    }

    inline void markReadTransfer()
    {
        m_isRead = TRUE;
    }

    inline void markPreRestart()
    {
        m_preRestart = TRUE;
    }

    inline void setBuffer(PUCHAR buffer, ULONG bufBytes)
    {
        m_pBuffer = buffer;
        m_bufBytes = bufBytes;
    }

    inline PUCHAR getBuffer()
    {
        return m_pBuffer;
    }

    inline ULONG getBufferSize()
    {
        return m_bufBytes;
    }

    inline BOOL transferIsRead()
    {
        return m_isRead;
    }

    inline BOOL preResart()
    {
        return m_preRestart;
    }

    inline void chainNextTransfer(I2cTransferClass* pNext)
    {
        m_pNextXfr = pNext;
    }

    // Returns nullptr when there is no "next transfer".
    inline I2cTransferClass* getNextTransfer()
    {
        return m_pNextXfr;
    }

    // Gets the next command/write byte.  Returns FALSE if there is none.
    inline BOOL getNextCmd(UCHAR & next)
    {
        if (m_nextCmd >= m_bufBytes)
        {
            return FALSE;
        }
        else
        {
            next = m_pBuffer[m_nextCmd];
            m_nextCmd++;
            if (m_nextCmd == m_bufBytes)
            {
                m_lastCmdFetched = TRUE;
            }
            return TRUE;
        }
    }

    // Returns TRUE is the last command byte has been fetched from buffer.
    inline BOOL lastCmdFetched()
    {
        return m_lastCmdFetched;
    }

    // Return the next available location in the read buffer, or nullptr
    // if the read buffer is full.
    inline PUCHAR getNextReadLocation()
    {
        if (!m_isRead || (m_nextRead >= m_bufBytes))
        {
            return nullptr;
        }
        else
        {
            PUCHAR nextRead = &(m_pBuffer[m_nextRead]);
            m_nextRead++;
            return nextRead;
        }
    }

private:

    // Pointer to the next transfer in the queue (if any)
    I2cTransferClass* m_pNextXfr;

    // Pointer to buffer associated with this transfer.
    PUCHAR m_pBuffer;

    // Size of the buffer in bytes.
    ULONG m_bufBytes;

    // TRUE if this is a read transfer.
    BOOL m_isRead;

    // Index of next command/write location in buffer.
    ULONG m_nextCmd;

    // Index of next read location in buffer (unused for writes).
    ULONG m_nextRead;

    // TRUE when the last command byte has been fetched.
    BOOL m_lastCmdFetched;

    // TRUE to start transfer with a RESTART.
    BOOL m_preRestart;
};

//
// Here, "transaction" is used to mean a set of I2C transfers that occurs 
// to/from a single I2C slave address.
// A transaction begins with a START and ends with a STOP.  The I2C bus is
// claimed for exclusive use by a transaction during the execution phase.
//
class I2cTransactionClass
{
public:
    I2cTransactionClass()
    {
        m_slaveAddress = 0;
        m_pFirstXfr = nullptr;
        m_pXfrQueueTail = nullptr;
        m_cmdsOutstanding = 0;
        m_readsOutstanding = 0;
        m_hI2cLock = CreateMutex(NULL, FALSE, L"Global\\I2c_Controller_Mutex");
        if (m_hI2cLock == NULL)
        {
            m_hI2cLock = INVALID_HANDLE_VALUE;
        }
    }

    virtual ~I2cTransactionClass()
    {
        reset();
        if (m_hI2cLock != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_hI2cLock);
            m_hI2cLock = INVALID_HANDLE_VALUE;
        }
    }

    // Prepare this transaction for re-use.
    // Any previously set slave address is not affected by this method.
    void reset()
    {
        I2cTransferClass* pCurrent = m_pFirstXfr;
        I2cTransferClass* pNext = nullptr;

        // Delete each transfer entry in the transfer queue.
        while (pCurrent != nullptr)
        {
            pNext = pCurrent->getNextTransfer();
            delete pCurrent;
            pCurrent = pNext;
        }
        m_pFirstXfr = nullptr;
        m_pXfrQueueTail = nullptr;
        m_cmdsOutstanding = 0;
        m_readsOutstanding = 0;

        m_readWaitCount = 0;
    }

    // Sets the 7-bit address of the slave for this tranaction.
    BOOL setAddress(UCHAR slaveAdr)
    {
        BOOL status = TRUE;
        BOOL error = ERROR_SUCCESS;


        // Verify we successfully create the I2C Controller Lock.
        if (m_hI2cLock == INVALID_HANDLE_VALUE)
        {
            status = FALSE;
            error = ERROR_CREATE_FAILED;
        }

        if (status && ((slaveAdr < 0x08) || (slaveAdr >= 0x77)))
        {
            status = FALSE;
            error = ERROR_INVALID_ADDRESS;
        }

        if (status)
        {
            m_slaveAddress = slaveAdr;
        }

        if (!status) { SetLastError(error); }
        return status;
    }

    // Add a write transfer to the transaction.
    BOOL write(PUCHAR buffer, ULONG bufferBytes)
    {
        return write(buffer, bufferBytes, FALSE);
    }

    BOOL write(PUCHAR buffer, ULONG bufferBytes, BOOL preRestart)
    {
        BOOL status = TRUE;
        BOOL error = ERROR_SUCCESS;
        I2cTransferClass* pXfr = nullptr;


        // Sanity check the buffer and size parameters.
        if ((buffer == nullptr) || (bufferBytes == 0))
        {
            status = FALSE;
            error = ERROR_INVALID_PARAMETER;
        }

        if (status)
        {
            // Allocate a transfer object.
            pXfr = new I2cTransferClass;

            if (pXfr == 0)
            {
                status = FALSE;
                error = ERROR_OUTOFMEMORY;
            }
        }

        if (status)
        {
            // Mark transfer to start with a restart if that was requested.
            if (preRestart)
            {
                pXfr->markPreRestart();
            }

            // Fill in the transfer object (default is Write Transfer).
            pXfr->setBuffer(buffer, bufferBytes);

            // Queue the transfer as part of this transaction.
            _queueTransfer(pXfr);

            // Add the write byte count to the transaction outstanding command count.
            m_cmdsOutstanding = m_cmdsOutstanding + bufferBytes;
        }

        if (!status) { SetLastError(error); }
        return status;
    }

    // Add a read transfer to the transaction.
    BOOL read(PUCHAR buffer, ULONG bufferBytes)
    {
        return read(buffer, bufferBytes, FALSE);
    }

    BOOL read(PUCHAR buffer, ULONG bufferBytes, BOOL preRestart)
    {
        BOOL status = TRUE;
        BOOL error = ERROR_SUCCESS;
        I2cTransferClass* pXfr = nullptr;


        // Sanity check the buffer and size parameters.
        if ((buffer == nullptr) || (bufferBytes == 0))
        {
            status = FALSE;
            error = ERROR_INVALID_PARAMETER;
        }

        if (status)
        {
            // Allocate a transfer object.
            pXfr = new I2cTransferClass;

            if (pXfr == 0)
            {
                status = FALSE;
                error = ERROR_OUTOFMEMORY;
            }
        }

        if (status)
        {
            // Mark transfer to start with a restart if that was requested.
            if (preRestart)
            {
                pXfr->markPreRestart();
            }

            // Fill in the transfer object.
            pXfr->setBuffer(buffer, bufferBytes);
            pXfr->markReadTransfer();

            // Queue the transfer as part of this transaction.
            _queueTransfer(pXfr);

            // Add the read byte count to the transaction outstanding command and read counts.
            m_cmdsOutstanding = m_cmdsOutstanding + bufferBytes;
            m_readsOutstanding = m_readsOutstanding + bufferBytes;
        }

        if (!status) { SetLastError(error); }
        return status;
    }

    // Method to perform the transfers associated with this transaction.
    BOOL execute()
    {
        BOOL status = TRUE;
        BOOL error = ERROR_SUCCESS;
        I2cTransferClass* pXfr = nullptr;
        I2cTransferClass* pReadXfr = nullptr;
        DWORD lockResult = 0;
        BOOL haveLock = FALSE;


        // TODO: Verify transaction parameters are set up properly (?)

        // Verify we successfully created the I2C Controller Lock.
        if (m_hI2cLock == INVALID_HANDLE_VALUE)
        {
            status = FALSE;
            error = ERROR_CREATE_FAILED;
        }

        // Claim the I2C controller.
        lockResult = WaitForSingleObject(m_hI2cLock, 5000);
        if ((lockResult == WAIT_OBJECT_0) || (lockResult == WAIT_ABANDONED))
        {
            haveLock = TRUE;
        }
        else if (lockResult == WAIT_TIMEOUT)
        {
            status = FALSE;
            error = ERROR_TIMEOUT;
        }
        else
        {
            status = FALSE;
            error = GetLastError();
        }

        // Initialize the controller.
        status = _initializeI2cForTransaction();
        if (!status) { error = GetLastError(); }

        // Process each transfer on the queue.
        if (status)
        {
            status = _processTransfers();
            if (!status) { error = GetLastError(); }

            // Shut down the controller.
            _shutDownI2cAfterTransaction();
        }

        // Release this transaction's claim on the controller.
        if (haveLock)
        {
            status = ReleaseMutex(m_hI2cLock);
            if (!status) { error = GetLastError(); }
            haveLock = FALSE;
        }

        if (!status) { SetLastError(error); }
        return status;
    }

    // Method to get the number of loops done to purge outstanding reads.
    inline void getReadWaitCount(ULONG & waits)
    {
        waits = m_readWaitCount;
    }

private:
    // The address of the I2C slave for this transaction.
    // Currently, only 7-bit addresses are supported.
    UCHAR m_slaveAddress;

    // Queue of transfers for this transaction.
    I2cTransferClass* m_pFirstXfr;

    // Address of transfer queue tail.
    I2cTransferClass* m_pXfrQueueTail;

    // Number of I2C command operations outstanding.
    LONG m_cmdsOutstanding;

    // Number of I2C reads outstanding.
    LONG m_readsOutstanding;

    // The wait time for outstanding reads.
    ULONG m_readWaitCount;

    // The I2C Mutex handle.
    HANDLE m_hI2cLock;

    // Method to queue a transfer as part of this transaction.
    void _queueTransfer(I2cTransferClass* pXfr)
    {
        // If the transfer queue is empty:
        if (m_pXfrQueueTail == nullptr)
        {
            // Add this transfer as the first entry in the queue.
            m_pFirstXfr = pXfr;
            m_pXfrQueueTail = pXfr;
        }

        // If there is at least one other transfer in the queue:
        else
        {
            // Add this entry to the tail of the queue.
            m_pXfrQueueTail->chainNextTransfer(pXfr);
            m_pXfrQueueTail = pXfr;
        }
    }

    // Method to initialize the I2C Controller at the start of a transaction.
    BOOL _initializeI2cForTransaction()
    {
        BOOL status = TRUE;
        BOOL error = ERROR_SUCCESS;


        // Get the I2C Controller mapped if it is not mapped yet.
        status = g_i2c.mapIfNeeded();
        if (!status) { error = GetLastError(); }

        if (status)
        {
            // Make sure the I2C controller is disabled.
            g_i2c.disableController();

            // Set the desired I2C Clock speed.
            g_i2c.setStandardSpeed();

            // Allow bus restarts.
            g_i2c.enableRestartSupport();

            // Set the address of the slave this tranaction affects.
            g_i2c.set7bitAddressing();
            g_i2c.setAddress(m_slaveAddress);

            // Mask all interrupts.
            g_i2c.maskAllInterrupts();

            // Clear any outstanding interrupts.
            g_i2c.clearAllInterrupts();

            // We know that when the I2C Controller is disabled the RX and TX
            // FIFOs are cleared, so we don't need to flush them here.

            // Enable the controller.
            g_i2c.enableController();
        }

        if (!status) { SetLastError(error); }
        return status;
    }

    // Method to process each transfer in this transaction.
    BOOL _processTransfers()
    {
        BOOL status = TRUE;
        BOOL error = ERROR_SUCCESS;
        I2cTransferClass* cmdXfr = nullptr;
        I2cTransferClass* readXfr = nullptr;
        PUCHAR readPtr = nullptr;
        BOOL restart = FALSE;
        ULONG cmdDat;
        ULONG readWaitCount;
        UCHAR writeByte;
        UCHAR readByte;


        // For each transfer in the queue:
        cmdXfr = m_pFirstXfr;
        if ((cmdXfr != nullptr) && (cmdXfr->preResart()))
        {
            restart = TRUE;
        }
        while ((m_cmdsOutstanding > 0) && (cmdXfr != nullptr))
        {
            // If this is the first read transfer:
            if ((readXfr == nullptr) && cmdXfr->transferIsRead())
            {
                // Indicate this is the transfer to read into.
                readXfr = cmdXfr;
                readXfr->resetRead();
                readPtr = readXfr->getNextReadLocation();
            }

            // Prepare to access the cmd buffer.
            cmdXfr->resetCmd();

            // For each byte in the transfer:
            while (cmdXfr->getNextCmd(writeByte))
            {
                // Wait for at least one empty space in the TX FIFO.
                while (g_i2c.txFifoFull());
                
                // Issue the command.
                if (cmdXfr->transferIsRead())
                {
                    cmdDat = 0x100;             // Build read command (data is ignored)
                }
                else
                {
                    cmdDat = writeByte;         // Build write command with data byte
                }

                // If restart has been requested, signal a pre-RESTART.
                if (restart)
                {
                    cmdDat = cmdDat | (1 << 10);
                    restart = FALSE;            // Only one RESTART allowed per transfer
                }

                // If this is the last command in the last buffer, signal a STOP.
                if (cmdXfr->lastCmdFetched() && (cmdXfr->getNextTransfer() == nullptr))
                {
                    cmdDat = cmdDat | (1 << 9);
                }

                g_i2c.issueCmd(cmdDat);
                m_cmdsOutstanding--;

                // Pull any available bytes out of the receive FIFO.
                while (g_i2c.rxFifoNotEmtpy())
                {
                    // Read a byte from the I2C Controller.
                    readByte = g_i2c.readByte();
                    m_readsOutstanding--;

                    // Store the byte if we have a place for it.
                    if (readPtr != nullptr)
                    {
                        *readPtr = readByte;

                        // Figure out where the next byte should go.
                        readPtr = readXfr->getNextReadLocation();
                        while ((readPtr == nullptr) && (readXfr->getNextTransfer() != nullptr))
                        {
                            readXfr = readXfr->getNextTransfer();
                            readXfr->resetRead();
                            readPtr = readXfr->getNextReadLocation();
                        }
                    }
                }
            }
            cmdXfr = cmdXfr->getNextTransfer();
        }

        // Complete any outstanding reads.
        readWaitCount = 0;
        while (status && (m_readsOutstanding > 0))
        {
            // Pull any available bytes out of the receive FIFO.
            while (g_i2c.rxFifoNotEmtpy())
            {
                // Read a byte from the I2C Controller.
                readByte = g_i2c.readByte();
                m_readsOutstanding--;

                // Store the byte if we have a place for it.
                if (readPtr != nullptr)
                {
                    *readPtr = readByte;

                    // Figure out where the next byte should go.
                    readPtr = readXfr->getNextReadLocation();
                    while ((readPtr == nullptr) && (readXfr->getNextTransfer() != nullptr))
                    {
                        readXfr = readXfr->getNextTransfer();
                        readXfr->resetRead();
                        readPtr = readXfr->getNextReadLocation();
                    }
                }
            }

            if (m_readsOutstanding > 0)
            {
                if (readWaitCount < 50)
                {
                    Sleep(20);
                    readWaitCount++;
                }
                else
                {
                    status = FALSE;
                    error = ERROR_RECEIVE_PARTIAL;
                }
            }
        }

        m_readWaitCount = readWaitCount;

        if (status)
        {
            if (m_cmdsOutstanding > 0)
            {
                status = FALSE;
                error = ERROR_MORE_DATA;
            }
            else if (m_readsOutstanding < 0)
            {
                status = FALSE;
                error = ERROR_DATA_NOT_ACCEPTED;
            }
            else if (m_cmdsOutstanding < 0)
            {
                status = FALSE;
                error = ERROR_TOO_MANY_CMDS;
            }
        }

        if (!status) { SetLastError(error); }
        return status;
    }

    // Method to shut down the I2C Controller after a transaction is done with it.
    BOOL _shutDownI2cAfterTransaction()
    {
        BOOL status = TRUE;
        BOOL error = ERROR_SUCCESS;
        ULONG loopCount = 0;


        // Wait for the TX FIFO to go empty.
        while (!g_i2c.txFifoEmpty())
        {
            // Voluntarily give up the CPU if another thread is waiting.
            Sleep(0);
        }

        // Disable the controller.
        g_i2c.disableController();

        // Wait for it to go inactive.
        while (g_i2c.isActive() && (loopCount < 5))
        {
            Sleep(1);
            loopCount++;
        }

        if (!status) { SetLastError(error); }
        return status;
    }
};


#endif // _I2C_CONTROLLER_H_