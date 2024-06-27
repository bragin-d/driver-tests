#include <stdio.h>
#include "ti_can.h"

void app_main(void)
{
    BitTimingParams   bTParams;
    TiMRAMParams      MRAM;
    TXFIFOElement     TX;

    bTParams.prescaler       = 3;
    bTParams.prop_and_phase1 = 6;
    bTParams.phase2          = 1;
    bTParams.sync_jump_width = bTParams.phase2;
    bTParams.tdc             = 0;

    // SID
    MRAM.SID_LSS  = 2;
    MRAM.SID_FLSS = 0x8000;

    // XID
    MRAM.XID_LSE = 1;
    MRAM.XID_FLSEA = 0x8008;

    // Rx FIFO 0
    MRAM.RXF0_F0OM = 0;
    MRAM.RXF0_F0WM = 2;
    MRAM.RXF0_F0S = 4;
    MRAM.RXF0_F0SA = 0x80F0;

    // Rx FIFO 1
    MRAM.RXF1_F1OM = 0;
    MRAM.RXF1_F1WM = 3;
    MRAM.RXF1_F1S = 4;
    MRAM.RXF1_F1SA = 0x80F0;

    // Rx Buffer
    MRAM.RXB_RBSA = 0;

    // Rx Element Size Config
    MRAM.RX_RBDS = 0x0;
    MRAM.RX_F1DS = 0x7;
    MRAM.RX_F0DS = 0x6;

    // Tx Event FIFO Config
    MRAM.TXEVF_EFWM = 2;
    MRAM.TXEVF_EFS  = 3;
    MRAM.TXEVF_EFSA = 0x8258;

    // Tx Buffer Config
    MRAM.TXB_TFQM = 0;
    MRAM.TXB_TFQS = 10;
    MRAM.TXB_NDTB = 0;
    MRAM.TXB_TBSA = 0x8270;

    // TX Element Size Config
    MRAM.TX_TBDS = 0x7;


    // Testing INIT

    uint8_t result = initCAN(&bTParams, &MRAM);

    if (!result)
    {
        printf("CAN initialized successfully\n");
    }
    else
    {
        printf("CAN initialization failed\n");
    }


    // Testing sendCAN, message sending

    TX.ESI = false;
    TX.XTD = false;
    TX.RTR = false;

    TX.ID = 0b11010100101;
    TX.MM = 0x1;
    TX.EFC = false;

    TX.FDF = false;
    TX.BRS = false;
    TX.DLC = 0x04;

    TX.data_byte_0 = 0x11;
    TX.data_byte_1 = 0x22;
    TX.data_byte_2 = 0x33;
    TX.data_byte_3 = 0x44;


    uint8_t send_result = sendCAN(&MRAM, &TX);

    if (!result)
    {
        printf("CAN message sent successfully\n");
    }
    else
    {
        printf("CAN message sending failed\n");
    }

}
