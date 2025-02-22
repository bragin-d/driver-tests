///
/// Author: Daniil Bragin
/// Reference: 

#include "ti_can.h"
#include <assert.h>
#include <string.h>
#include "driver/spi_master.h"
#include "esp_log.h"
#include <string.h>
#include <unistd.h>

//#define SPI_HOST    HSPI_HOST
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5

spi_device_handle_t spi;

void spi_init() {
    esp_err_t ret;
    
    // Configure the SPI bus
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    // Initialize the SPI bus
    ret = spi_bus_initialize(VSPI_HOST, &buscfg, 1);
    assert(ret == ESP_OK);

    // Configure the SPI device
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 26 * 1000 * 1000,    // Clock out at 10 MHz
        .mode = 0,                             // SPI mode 0
        
        .spics_io_num = PIN_NUM_CS,            // CS pin
        .queue_size = 7,                     
        .pre_cb = NULL,
        .post_cb = NULL,
        .command_bits = 8,                     // Important! 
        .address_bits = 16,                     // Important!
        .dummy_bits = 0
    };

    // Attach the SPI device to the SPI bus
    ret = spi_bus_add_device(VSPI_HOST, &devcfg, &spi);
    assert(ret == ESP_OK);
}





uint8_t initCAN (const BitTimingParams  * bTParams, 
                 const TiMRAMParams     * MRAM)
{
    
    //uint32_t value = 0;

    spi_init();

    printf("Initialized SPI\n");

    // TODO: Cover edge cases for values in structs + error reporting

    uint32_t mode = 0;

    mode = spiRegisterRead(MODE_SEL);

    // Standby Mode Check
    if ((mode & 0xC0) != STANDBY_MODE)
    {   printf("Device not in STANDBY_MODE, MODE: %lu, setting STANDBY_MODE\n", mode & 0xC0);
        
        mode &= ~CLEAN_MODE;
        mode |= STANDBY_MODE;
        
        spiRegisterWrite(MODE_SEL, mode, NULL);
        printf("STANDBY_MODE set successfully\n");

        mode = spiRegisterRead(MODE_SEL);
        printf("New device mode: %lX, NEEDED: %X\n", mode & 0xC0, STANDBY_MODE);
    }
    else
    {
        printf("Device in STANDBY_MODE, mode: %lX\n", mode);
    }
    

    // Initialization Mode
    // TODO: Check whether CSR needs to be written before CCE and INIT
    uint32_t init = spiRegisterRead(CCCR);

    printf("Initial CCCR content: %lX\n", init);

    init &= ~(CAN_CCCR_CCE | CAN_CCCR_INIT);
    init |= (CAN_CCCR_CCE | CAN_CCCR_INIT);
    init &= ~CAN_CCCR_CSR;

    
    uint32_t bit_timing = 0;
    uint32_t trans_delay_comp = 0;

    uint8_t prescaler       =   bTParams -> prescaler;
    uint8_t prop_and_phase1 =   bTParams -> prop_and_phase1;
    uint8_t phase2          =   bTParams -> phase2;
    uint8_t sync_jump_width =   bTParams -> sync_jump_width;
    uint8_t tdc             =   bTParams -> tdc;


    // FD/BRS & Bit Timing Init Cofiguration
    if (CAN_MODE == 0)
    {
        printf("Device in CAN mode\n");
        spiRegisterWrite(CCCR, init, NULL);

        init = spiRegisterRead(CCCR);

        printf("New CCCR reg value: %lX\n", init);
    
        bit_timing = spiRegisterRead(NBTP); 

        printf("Initial NBTP content: %lu\n", bit_timing); 

        // Reset the NBTP register values
        bit_timing &= ~(CAN_NBTP_NSJW_MASK      | 
                        CAN_NBTP_NTSEG1_MASK    | 
                        CAN_NBTP_NTSEG2_MASK    | 
                        CAN_NBTP_NBRP_MASK);

        // Set the NBTP register values based on the provided settings
        bit_timing |= CAN_SYNC_JUMP_WIDTH(sync_jump_width);
        bit_timing |= CAN_TIME_SEG_1(prop_and_phase1);
        bit_timing |= CAN_TIME_SEG_2(phase2);
        bit_timing |= CAN_PRESCALER(prescaler);

        printf("Intended NBTP value: %lX\n", bit_timing);

        //
        // HARDCODED
        //
        //bit_timing = 0x02030601;

        spiRegisterWrite(NBTP, bit_timing, NULL);

        bit_timing = spiRegisterRead(NBTP);

        printf("New NBTP reg content: %lX\n", bit_timing);

    }
    else if (CAN_MODE == 1)
    {
        printf("Device in CAN-FD mode\n");
        // Check BRS and FD settings
        if (BIT_RATE_SWITCH)    init |= CCCR_BRSE;
        if (FD)     init |= CCCR_FDOE;

        printf("Intended CCCR reg value: %lX\n", init);

        spiRegisterWrite(CCCR, init, NULL);

        init = spiRegisterRead(CCCR);

        printf("New CCCR reg value: %lX\n", init);

        bit_timing       =  spiRegisterRead(DBTP);
        trans_delay_comp =  spiRegisterRead(TDCR);

        printf("BT reg value: %lX\n", bit_timing);
        printf("TDC reg value: %lX\n", trans_delay_comp);
        
        // Reset the DBTP register values
        bit_timing &= ~(CANFD_DBTP_DSJW_MASK      | 
                        CANFD_DBTP_DTSEG1_MASK    | 
                        CANFD_DBTP_DTSEG2_MASK    | 
                        CANFD_DBTP_DBRP_MASK);
        
        trans_delay_comp &= ~CANFD_TDCR_TDCO_MASK;
        
        // Set the DBTP register values based on the provided settings
        bit_timing |= CANFD_SYNC_JUMP_WIDTH(sync_jump_width);
        bit_timing |= CANFD_TIME_SEG_1_WIDTH(prop_and_phase1);
        bit_timing |= CANFD_TIME_SEG_2_WIDTH(phase2);
        bit_timing |= CANFD_PRESCALER(prescaler);

        trans_delay_comp |= CANFD_DELAY_COMPENSATION_OFFSET(tdc);

        printf("Intended BT reg value: %lX\n", bit_timing);
        printf("Intended TDC reg value: %lX\n", trans_delay_comp);

        spiRegisterWrite(DBTP, bit_timing, NULL);

        spiRegisterWrite(TDCR, trans_delay_comp, NULL);


        bit_timing = spiRegisterRead(DBTP);
        trans_delay_comp = spiRegisterRead(TDCR);

        printf("New BT reg value: %lX\n", bit_timing);
        printf("New TDC reg value: %lX\n", trans_delay_comp);
    }

    // MRAM Init

    spiRegisterWrite(SIDFC, 0x0, NULL);
    spiRegisterWrite(XIDFC, 0x0, NULL);
    spiRegisterWrite(RXF0C, 0x0, NULL);
    spiRegisterWrite(RXF1C, 0x0, NULL);
    spiRegisterWrite(RXBC, 0x0, NULL);
    spiRegisterWrite(RXESC, 0x0, NULL);
    spiRegisterWrite(TXEFC, 0x0, NULL);
    spiRegisterWrite(TXBC, 0x0, NULL);
    spiRegisterWrite(TXESC, 0x0, NULL);
    
    uint32_t sid        = spiRegisterRead(SIDFC);
    uint32_t xid        = spiRegisterRead(XIDFC);
    uint32_t rxf0       = spiRegisterRead(RXF0C);
    uint32_t rxf1       = spiRegisterRead(RXF1C);
    uint32_t rxb        = spiRegisterRead(RXBC);
    uint32_t rx         = spiRegisterRead(RXESC);
    uint32_t tx_fifo    = spiRegisterRead(TXEFC);
    uint32_t txb        = spiRegisterRead(TXBC);
    uint32_t tx         = spiRegisterRead(TXESC);

    printf("\n\n---------------------------------------------------------------------\n\n");
    printf("MRAM content:\nSID: %lX\nXID: %lX\nRXF0: %lX\nRXF1: %lX\nRXB: %lX\nRX: %lX\nTX_FIFO: %lX\nTXB: %lX\nTX: %lX\n",
            sid, xid, rxf0, rxf1, rxb, rx, tx_fifo, txb, tx);
    printf("---------------------------------------------------------------------\n\n");

    sid &= ~(SID_LSS_MASK |
             SID_FLSS_MASK);

    sid |= SID_LSS(MRAM -> SID_LSS);
    sid |= SID_FLSS(MRAM -> SID_FLSS);

    
    xid &= ~(XID_LSE_MASK |
             XID_FLSEA_MASK);

    xid |= XID_LSE(MRAM -> XID_LSE);
    xid |= XID_FLSEA(MRAM -> XID_FLSEA);
    
    
    rxf0 &= ~(RXF0_F0OM_MASK    |
              RXF0_F0WM_MASK    |
              RXF0_F0S_MASK     |
              RXF0_F0SA_MASK);    

    rxf0 |= RXF0_F0OM(MRAM -> RXF0_F0OM);
    rxf0 |= RXF0_F0WM(MRAM -> RXF0_F0WM);
    rxf0 |= RXF0_F0S(MRAM -> RXF0_F0S);
    rxf0 |= RXF0_F0SA(MRAM -> RXF0_F0SA);


    rxf1 &= ~(RXF1_F1OM_MASK    |
              RXF1_F1WM_MASK    |
              RXF1_F1S_MASK     |
              RXF1_F1SA_MASK);

    rxf1 |= RXF1_F1OM(MRAM -> RXF1_F1OM);
    rxf1 |= RXF1_F1WM(MRAM -> RXF1_F1WM);
    rxf1 |= RXF1_F1S(MRAM -> RXF1_F1S);
    rxf1 |= RXF1_F1SA(MRAM -> RXF1_F1SA);


    rxb &= ~(RXB_RBSA_MASK);

    rxb |= RXB_RBSA(MRAM -> RXB_RBSA);


    rx &= ~(RX_RBDS_MASK    |
            RX_F1DS_MASK    |
            RX_F0DS_MASK);
    
    rx |= RX_RBDS(MRAM -> RX_RBDS);
    rx |= RX_F1DS(MRAM -> RX_F1DS);
    rx |= RX_F0DS(MRAM -> RX_F0DS);


    tx_fifo &= ~(TXEVF_EFWM_MASK    |
                 TXEVF_EFS_MASK     |
                 TXEVF_EFSA_MASK);

    tx_fifo |= TXEVF_EFWM(MRAM -> TXEVF_EFWM);
    tx_fifo |= TXEVF_EFS(MRAM -> TXEVF_EFS);
    tx_fifo |= TXEVF_EFSA(MRAM -> TXEVF_EFSA);


    txb &= ~(TXB_TFQM_MASK  |
             TXB_TFQS_MASK  |
             TXB_NDTB_MASK  |
             TXB_TBSA_MASK);
    
    txb |= TXB_TFQM(MRAM -> TXB_TFQM);
    txb |= TXB_TFQS(MRAM -> TXB_TFQS);
    txb |= TXB_NDTB(MRAM -> TXB_NDTB);
    txb |= TXB_TBSA(MRAM -> TXB_TBSA);


    tx &= ~(TX_TBDS_MASK);

    tx |= TX_TBDS(MRAM -> TX_TBDS);

////////////////////////////////////////
/// HARDCODED ///
////////////////////////////////////////

    // sid = 0x00020000;
    // xid = 0x010008;
    // rxf0 = 0x02040010;
    // rxf1 = 0x030500F0;
    // rxb = 0x00;
    // rx = 0x076;
    // tx_fifo = 0x02030258;
    // txb = 0x0A000270;
    // tx = 0x07;

////////////////////////////////////////
/// HARDCODED ///
////////////////////////////////////////

    printf("Intended MRAM content:\nSID: %lX\nXID: %lX\nRXF0: %lX\nRXF1: %lX\nRXB: %lX\nRX: %lX\nTX_FIFO: %lX\nTXB: %lX\nTX: %lX\n",
            sid, xid, rxf0, rxf1, rxb, rx, tx_fifo, txb, tx);
    printf("---------------------------------------------------------------------\n\n");


    spiRegisterWrite(SIDFC, sid, NULL);
    spiRegisterWrite(XIDFC, xid, NULL);
    spiRegisterWrite(RXF0C, rxf0, NULL);
    spiRegisterWrite(RXF1C, rxf1, NULL);
    spiRegisterWrite(RXBC, rxb, NULL);
    spiRegisterWrite(RXESC, rx, NULL);
    spiRegisterWrite(TXEFC, tx_fifo, NULL);
    spiRegisterWrite(TXBC, txb, NULL);
    spiRegisterWrite(TXESC, tx, NULL);
    
    sid = spiRegisterRead(SIDFC);
    xid = spiRegisterRead(XIDFC);
    rxf0 = spiRegisterRead(RXF0C);
    rxf1 = spiRegisterRead(RXF1C);
    rxb = spiRegisterRead(RXBC);
    rx = spiRegisterRead(RXESC);
    tx_fifo = spiRegisterRead(TXEFC);
    txb = spiRegisterRead(TXBC);
    tx = spiRegisterRead(TXESC);
    


    printf("New MRAM regs content:\nSID: %lX\nXID: %lX\nRXF0: %lX\nRXF1: %lX\nRXB: %lX\nRX: %lX\nTX_FIFO: %lX\nTXB: %lX\nTX: %lX\n",
            sid, xid, rxf0, rxf1, rxb, rx, tx_fifo, txb, tx);
    printf("---------------------------------------------------------------------\n\n");

    // Put the TCAN45xx device into "NORMAL" mode

    uint32_t selected_mode = spiRegisterRead(MODE_SEL);

    printf("Initial device mode: %lX\n", selected_mode);

    selected_mode &= ~CLEAN_MODE;
    selected_mode |= NORMAL_MODE;

    printf("Intended mode: %lX\n", selected_mode);

    spiRegisterWrite(MODE_SEL, selected_mode, NULL);
    printf("Device in NORMAL_MODE, init_can successful\n");

    selected_mode = spiRegisterRead(MODE_SEL);

    printf("New selected mode: %lX\n", selected_mode);


    return 0;
}

uint8_t setSIDFilters(SID_filter * filters, TiMRAMParams * MRAM) 
{
    size_t size = (size_t) MRAM -> SID_LSS;
    uint32_t * filter_addr = (uint32_t *)MRAM -> SID_FLSS;
    uint32_t filter = 0;

    for (size_t i = 0; i < size; i++)
    {
        filter = 0;
        
        filter |= SID_SFT   (filters[i].SFT);
        filter |= SID_SFEC  (filters[i].SFEC);
        filter |= SID_SFID1 (filters[i].SFID_1);
        filter |= SID_SFID2 (filters[i].SFID_2);

        spiRegisterWrite(filter_addr + i * sizeof(uint32_t), filter, NULL);
    }

    return 0;
}

uint8_t setXIDFilters(XID_filter * filters, TiMRAMParams * MRAM)
{
    size_t   size           = (size_t) MRAM -> XID_LSE;
    uint32_t filter_addr    = MRAM -> XID_FLSEA;
    uint64_t filter         = 0; // Two words needed for XID
    uint32_t filter_1       = 0;
    uint32_t filter_2       = 0;

    for (size_t i = 0; i < size; i++)
    {
        filter      = 0;
        filter_1    = 0;
        filter_2    = 0;
        
        filter_1 |= XID_EFID2(filters[i].EFID1);
        filter_1 |= XID_EFT(filters[i].EFT);

        filter_2 |= XID_EFID2(filters[i].EFID2);
        filter_2 |= XID_EFEC(filters[i].EFEC);

        filter |= (filter_1 | filter_2);

        spiRegisterWrite(filter_addr + i, filter, NULL);
    }

    return 0;
}

uint8_t sendCAN(TiMRAMParams * MRAM, TXFIFOElement * TXE)
{
    /*// Check that any TX Buffer is vacant
    uint32_t free_level = spiRegisterRead(TXFQS);

    uint32_t tx_buffer_start_addr = MRAM -> TXB_TBSA;
    uint32_t tx_buffer_element_size = MRAM -> TX_TBDS + 8; // Additional 8 bytes of header


    
    if (!(TFFL(free_level)))
    {
        printf("ERROR: No TX BUffers Available, return 1\n");
        return 1;
    }

    printf("Free buffers: %lX\n", TFFL(free_level));

    uint32_t index = TFQPI(free_level);

    uint32_t memory_offset = tx_buffer_start_addr + (tx_buffer_element_size * index);


    uint32_t word_0 = 0;
    uint32_t word_1 = 0;
    uint32_t word_2 = 0;

    if (TXE -> ESI) word_0 |= (1 << ESI_SHFT);
    if (TXE -> RTR) word_0 |= (1 << RTR_SHFT);
    if (TXE -> XTD)
    {
        word_0 |= (1 << XTD_SHFT);
        word_0 |= ((TXE -> ID) << XID_SHFT);
    } 
    else
    {
        word_0 |= ((TXE -> ID) << SID_SHFT);
    }

    spiRegisterWrite(memory_offset, word_0, NULL);

    word_1 |= ((TXE -> MM) << MM_SHFT);

    if (TXE -> EFC) word_1 |= (1 << EFC_SHFT);
    if (TXE -> FDF) word_1 |= (1 << FDF_SHFT);
    if (TXE -> BRS) word_1 |= (1 << BRS_SHFT);
    
    word_1 |= ((TXE -> DLC) << DLC_SHFT);

    spiRegisterWrite(memory_offset + sizeof(word_0), word_1, NULL);

    word_2 |= ( ((TXE -> data_byte_0) << DB0_SHFT)   |
                ((TXE -> data_byte_1) << DB1_SHFT)   |
                ((TXE -> data_byte_2) << DB2_SHFT)   |
                ((TXE -> data_byte_3) << DB3_SHFT)   );
    
    spiRegisterWrite(memory_offset + sizeof(word_0) + sizeof(word_1), word_2, NULL);

    printf("Words to send:\nWORD_0: %lX\nWORD_1: %lX\nWORD_2: %lX\n", word_0, word_1, word_2);

    uint32_t add_request = 0;

    add_request |= (1U << index);

    spiRegisterWrite(TXBUF_AR, add_request, NULL);*/

    // uint32_t free_level = spiRegisterRead(TXFQS);

    // if (!(TFFL(free_level)))
    // {
    //     printf("ERROR: No TX BUffers Available, return 1\n");
    //     return 1;
    // }

    // printf("Free buffers: %lX\n", TFFL(free_level));

    // uint32_t index = TFQPI(free_level);

    // printf("Index: %lX\n", index);

    // uint32_t reg_bits = spiRegisterRead(0x10C4);

    // printf("Reg value: %lX\n", reg_bits);

    spiRegisterWrite(0x8270, 0x52345678, NULL);
    spiRegisterWrite(0x8270 + 4, 0x01B70000, NULL);
    spiRegisterWrite(0x8270 + 2*4, 0x44332211, NULL);
    spiRegisterWrite(0x8270 + 3*4, 0x00776655, NULL);
    spiRegisterWrite(0x10D0, 0x01, NULL);


    return 0;
}


uint8_t spiRegisterWrite(uint16_t reg_addr, uint32_t reg_value, uint32_t * pointer) {

    uint8_t tx_buffer[] =
    {
        0x04,
        (uint8_t)(reg_value >> 24),
        (uint8_t)(reg_value >> 16), 
        (uint8_t)(reg_value >> 8),  
        (uint8_t)(reg_value) 
    };

    spi_transaction_t t;
    
    memset(&t, 0, sizeof(spi_transaction_t)); 

    t.cmd = SPI_WRITE_OPCODE;
    t.addr = reg_addr;
    t.length = 5 * 8; // length byte
    t.rxlength = 5 * 8;
    t.rx_buffer = NULL;
    t.tx_buffer = tx_buffer;

    // Transmit the dummy bytes to read the actual data
    spi_device_transmit(spi, &t);

    return 0;
}


uint32_t spiRegisterRead(uint16_t reg_addr) {
    
    // TODO: Process global status byte
    // TODO: Consider switching to working with pointers

    uint8_t rx_buffer[5];

    spi_transaction_t t;
    
    memset(&t, 0, sizeof(spi_transaction_t)); 

    t.cmd = SPI_READ_OPCODE;
    t.addr = reg_addr;
    t.length = 5 * 8; // length byte
    t.rxlength = 5 * 8;
    t.rx_buffer = rx_buffer;
    t.tx_buffer = NULL;

    // Transmit the dummy bytes to read the actual data
    spi_device_transmit(spi, &t);

    // Combine the received bytes into a 32-bit value
    uint32_t reg_value = ((uint64_t)rx_buffer[1] << 24) |
                ((uint64_t)rx_buffer[2] << 16) | ((uint64_t)rx_buffer[3] << 8) | (rx_buffer[4]);

    return reg_value;
}