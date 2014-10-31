// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _I2C_CONTROLLER_H_
#define _I2C_CONTROLLER_H_

#include <Windows.h>
#include <functional>

#include "GalileoPins.h"
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
        m_controllerInitialized = FALSE;
    }

    virtual ~I2cControllerClass()
    {
        this->endExternal();
    }

    // This method is used to prepare to use the I2C bus to communicate with devices
    // that are external to the board.
    BOOL beginExternal();

    // This method returns the external I2C bus pins to their default configurations.
    void endExternal();

    // This method maps the I2C controller if needed.
    inline BOOL mapIfNeeded();

    // This method records that the controller has been initialized.
    inline void setInitialized()
    {
        m_controllerInitialized = TRUE;
    }

    // This method returns TRUE if the I2C Controller has been initialized, FALSE otherwise.
    inline BOOL isInitialized()
    {
        return m_controllerInitialized;
    }

    //
    // I2C Controller accessor methods.  These methods assume the I2C Controller
    // has already been mapped using mapIfNeeded().
    //

    inline BOOL masterModeIsSet() const
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

    inline BOOL isEnabled() const
    {
        return (m_controller->IC_ENABLE_STATUS.IC_EN == 1);
    }

    inline void setStandardSpeed()
    {
        m_controller->IC_SS_SCL_HCNT.IC_SS_SCL_HCNT = 0x92;
        m_controller->IC_SS_SCL_LCNT.IC_SS_SCL_LCNT = 0xAB;
        m_controller->IC_CON.SPEED = 1;
    }

    inline void setFastSpeed()
    {
        m_controller->IC_FS_SCL_HCNT.IC_FS_SCL_HCNT = 0x14;
        m_controller->IC_FS_SCL_LCNT.IC_FS_SCL_LCNT = 0x2E;
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

    inline void setAddress(const ULONG adr)       // Supports 7 or 10 bit addressing
    {
        m_controller->IC_TAR.IC_TAR = (adr & 0x3FF);
    }

    inline ULONG getAddress() const
    {
        return m_controller->IC_TAR.IC_TAR;
    }

    inline BOOL txFifoNotFull() const
    {
        return (m_controller->IC_STATUS.TFNF == 1);
    }

    inline BOOL txFifoFull() const
    {
        return (m_controller->IC_STATUS.TFNF == 0);
    }

    inline BOOL txFifoEmpty() const
    {
        return (m_controller->IC_STATUS.TFE == 1);
    }

    inline BOOL rxFifoNotEmtpy() const
    {
        return (m_controller->IC_STATUS.RFNE == 1);
    }

    inline BOOL rxFifoEmpty() const
    {
        return (m_controller->IC_STATUS.RFNE == 0);
    }

    inline void issueCmd(ULONG cmd) const
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

    inline BOOL isActive() const
    {
        return (m_controller->IC_STATUS.MST_ACTIVITY == 1);
    }

    inline void clearAllInterrupts()
    {
        ULONG dummy = m_controller->IC_CLR_INTR.ALL_BITS;
    }

    /// Determine whether a TX Error has occurred or not.
    /**
    All I2C bus errors we are interested in are TX errors: failure to ACK an
    an address or write data.
    \return TRUE, an error occured.  FALSE, no error has occured.
    */
    inline BOOL errorOccured()
    {
        return (m_controller->IC_RAW_INTR_STAT.TX_ABRT == 1);
    }

    /// Determine if an I2C address was sent but not acknowledged by any slave.
    /**
    This method tests for the error that is expected to occur if an attempt
    is made talk to an I2C slave that does not exist.
    \return TRUE, an I2C address was not acknowdged.  FALSE, all addresses sent
    have been acknowledged by at least one slave.
    */
    inline BOOL addressWasNacked()
    {
        return (m_controller->IC_TX_ABRT_SOURCE.ABRT_7B_ADDR_NOACK == 1);
    }

    /// Determine if I2C data was sent but not acknowledged by a slave.
    /**
    This method tests for the error that occurs if a slave has been found, 
    but then fails to acknowledge a data byte sent by the master.
    \return TRUE, I2C data was not acknowdged.  FALSE, all data sent has 
    been acknowledged by a slave.
    */
    inline BOOL dataWasNacked()
    {
        return (m_controller->IC_TX_ABRT_SOURCE.ABRT_TXDATA_NOACK == 1);
    }

    /// Reset the controller after a bus error has occured.
    /**
    The actul error condition is cleard by reading the IC_CLR_TX_ABRT register.
    */
    inline void clearErrors()
    {
        ULONG dummy = m_controller->IC_CLR_TX_ABRT.ALL_BITS;
    }

private:

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

    //
    // I2cControllerClass private data members.
    //

    // Handle to the device that can be used to map I2C Controller registers
    // into user-mode address space.
    HANDLE m_hController;

    // Pointer to the object used to address the I2C Controller registers after
    // they are mapped into this process' address space.
    PI2C_CONTROLLER m_controller;

    // TRUE if the controller has been initialized.
    BOOL m_controllerInitialized;

    // Method to map the I2C controller into this process' virtual address space.
    BOOL _mapController();
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

    inline void setBuffer(PUCHAR buffer, const ULONG bufBytes)
    {
        m_pBuffer = buffer;
        m_bufBytes = bufBytes;
    }

    inline PUCHAR getBuffer() const
    {
        return m_pBuffer;
    }

    inline ULONG getBufferSize() const
    {
        return m_bufBytes;
    }

    inline BOOL transferIsRead() const
    {
        return m_isRead;
    }

    inline BOOL preResart() const
    {
        return m_preRestart;
    }

    inline void chainNextTransfer(I2cTransferClass* pNext)
    {
        m_pNextXfr = pNext;
    }

    // Returns nullptr when there is no "next transfer".
    inline I2cTransferClass* getNextTransfer() const
    {
        return m_pNextXfr;
    }

    // Gets the next command/write byte.  Returns FALSE if there is none.
    inline BOOL getNextCmd(UCHAR & next);

    // Returns TRUE is the last command byte has been fetched from buffer.
    inline BOOL lastCmdFetched() const
    {
        return m_lastCmdFetched;
    }

    // Return the next available location in the read buffer, or nullptr
    // if the read buffer is full.
    inline PUCHAR getNextReadLocation();

    // Method to associate a callback routine with this transfer.
    BOOL setCallback(std::function<BOOL()> callBack)
    {
        m_callBack = callBack;
        return TRUE;
    }

    // Method to invoke any callback routine associated with this transfer.
    inline BOOL invokeCallback()
    {
        if (hasCallback())
        {
            return m_callBack();
        }
        return TRUE;
    }

    // Return TRUE if this transfer specifies a callback function.
    inline BOOL hasCallback() const
    {
        return (!m_callBack._Empty() && (m_callBack != nullptr));
    }

private:

    //
    // I2cTransferClass data members.
    //

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

    // Pointer to any callback function associated with this transfer.
    std::function<BOOL()> m_callBack;
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
        m_abort = FALSE;
        m_error = SUCCESS;
        m_isIncomplete = FALSE;
        m_useHighSpeed = FALSE;
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
    void reset();

    // Sets the 7-bit address of the slave for this transaction.
    BOOL setAddress(ULONG slaveAdr);

    // Gets the 7-bit address of the slave for this transaction.
    ULONG getAddress()
    {
        return m_slaveAddress;
    }

    // Add a write transfer to the transaction.
    BOOL queueWrite(PUCHAR buffer, const ULONG bufferBytes)
    {
        return queueWrite(buffer, bufferBytes, FALSE);
    }

    BOOL queueWrite(PUCHAR buffer, const ULONG bufferBytes, const BOOL preRestart);

    // Add a read transfer to the transaction.
    BOOL queueRead(PUCHAR buffer, const ULONG bufferBytes)
    {
        return queueRead(buffer, bufferBytes, FALSE);
    }

    BOOL queueRead(PUCHAR buffer, const ULONG bufferBytes, const BOOL preRestart);

    // Method to queue a callback routine at the current point in the transaction.
    BOOL queueCallback(const std::function<BOOL()> callBack);

    // Method to perform the transfers associated with this transaction.
    BOOL execute();

    // Method to get the number of 1 mSec ticsk that occurred while waiting for outstanding reads.
    inline void getReadWaitTicks(ULONG & waits) const
    {
        waits = m_maxWaitTicks;
    }

    // Method to determine if a transfer is the last transfer in the transaction.
    inline BOOL isLastTransfer(I2cTransferClass* pXfr) const
    {
        return (pXfr == m_pXfrQueueTail);
    }

    /// Method to abort any remaining transfers.
    inline void abort()
    {
        m_abort = TRUE;
    }

    /// Enum for transaction error codes;
    const enum ERROR_CODE {
        SUCCESS,                ///< No Error has occured
        ADR_NACK,               ///< Slave address was not acknowledged
        DATA_NACK,              ///< Slave did not acknowledge data
        OTHER                   ///< Some other error occured
    };

    /// Get the current error code for this transaction.
    inline ERROR_CODE getError()
    {
        return m_error;
    }

    /// Method to determine if an error occured during this transaction.
    inline BOOL errorOccured()
    {
        return (m_error != SUCCESS);
    }

    /// Method to determine if this transaction has been completed or not.
    inline BOOL isIncomplete()
    {
        return m_isIncomplete;
    }

    /// Method to signal high speed can be sued for this transaction.
    inline void useHighSpeed()
    {
        m_useHighSpeed = TRUE;
    }

private:

    //
    // I2cTransactionClass data members.
    //

    // The address of the I2C slave for this transaction.
    // Currently, only 7-bit addresses are supported.
    ULONG m_slaveAddress;

    // Queue of transfers for this transaction.
    I2cTransferClass* m_pFirstXfr;

    // Address of transfer queue tail.
    I2cTransferClass* m_pXfrQueueTail;

    // Number of I2C command operations outstanding.
    LONG m_cmdsOutstanding;

    // Number of I2C reads outstanding.
    LONG m_readsOutstanding;

    // The max wait time (in mSec) for outstanding reads.
    ULONG m_maxWaitTicks;

    // The I2C Mutex handle.
    HANDLE m_hI2cLock;

    // Set to TRUE to abort the remainder of the transaction.
    BOOL m_abort;

    /// Error code 
    ERROR_CODE m_error;

    /// TRUE if one or more incompleted transfers exist on this transaction.
    BOOL m_isIncomplete;

    /// TRUE to allow use of high speed for this transaction.
    BOOL m_useHighSpeed;

    //
    // I2cTransactionClass private member functions.
    //

    // Method to queue a transfer as part of this transaction.
    void _queueTransfer(I2cTransferClass* pXfr);

    // Method to initialize the I2C Controller at the start of a transaction.
    BOOL _initializeI2cForTransaction();

    // Method to process each transfer in this transaction.
    BOOL _processTransfers();

    // Method to perform a set of transfers that happen together on the I2C bus.
    BOOL _performContiguousTransfers(I2cTransferClass* & pXfr);

    // Method to shut down the I2C Controller after a transaction is done with it.
    BOOL _shutDownI2cAfterTransaction();

    // Method to calculate the command and read counts for the current section
    // of the transaction.
    BOOL _calculateCurrentCounts(I2cTransferClass* nextXfr);

    /// Method to handle any errors that occured during this transaction.
    BOOL _handleErrors();
};

#endif // _I2C_CONTROLLER_H_