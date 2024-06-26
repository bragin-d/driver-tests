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
        .address_bits = 16                     // Important!
    };

    // Attach the SPI device to the SPI bus
    ret = spi_bus_add_device(VSPI_HOST, &devcfg, &spi);
    assert(ret == ESP_OK);
}

uint8_t spiRegisterWrite(uint16_t reg_addr, uint32_t reg_value, uint32_t * pointer) {

    uint8_t tx_buffer[5];
    uint8_t rx_buffer[5];

    //tx_buffer[0] = 0xBB;
    tx_buffer[1] = 0xFF;
    tx_buffer[2] = 0xFF;
    tx_buffer[3] = 0xAA;
    tx_buffer[4] = 0xAA;
    
    spi_transaction_t t;

    memset(&t, 0, sizeof(spi_transaction_t));

    t.cmd = SPI_WRITE_OPCODE;
    t.addr = reg_addr;
    t.length = 5 * 8; // length byte
    t.rxlength = 5 * 8;
    t.rx_buffer = NULL;
    t.tx_buffer = tx_buffer;

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



uint8_t initCAN (const BitTimingParams  * bTParams, 
                 const TiMRAMParams     * MRAM)
{
    
    uint32_t value = 0;

    spi_init();

    printf("Initialized SPI\n");

    // TODO: Cover edge cases for values in structs + error reporting

    value = spiRegisterRead(0x0808);

    printf("Test reg value: %lX\n", value);

    value = spiRegisterRead(0x0808);

    printf("Test reg value: %lX\n", value);

    spiRegisterWrite(0x0808, 0xBB, NULL);

    value = spiRegisterRead(0x0808);

    printf("Test reg value: %lX\n", value);


    // init |= CAN_CCCR_CCE;

    // printf("Intended CCCR value: %lX\n", init);

    // spiRegisterWrite(CCCR, init, NULL);

    // init = spiRegisterRead(CCCR);

    // printf("New CCCR value: %lX\n", init);

////////////////////////////////////////////////////////////

    // init &= ~CAN_CCCR_CSR;

    // printf("Intended CCCR value: %lX\n", init);

    // spiRegisterWrite(CCCR, init, NULL);

    // init = spiRegisterRead(CCCR);

    // printf("New CCCR value: %lX\n", init);

////////////////////////////////////////////////////////////

    // init |= CAN_CCCR_CCE;

    // printf("Intended CCCR value: %lX\n", init);

    // spiRegisterWrite(CCCR, init, NULL);

    // init = spiRegisterRead(CCCR);

    // printf("New CCCR value: %lX\n", init);

////////////////////////////////////////////////
//     uint32_t check;
    
//     check = spiRegisterRead(0x0808);

//     printf("Reg content: %lX\n", check);

// ////////////////////////////////////////////////

//     uint32_t reg_end_check = 0;

//     reg_end_check = spiRegisterRead(0x1004);

//     printf("Endian reg: %lX\n", reg_end_check);

//     spiRegisterWrite(0x0808, value | (0x1 << 16U), NULL);

//     usleep(1000);

//     value = spiRegisterRead(0x0808);

//     printf("Prev reg value: %lX\n", check);
//     printf("New reg content: %lX\n", value);

///////////////////////////////////////////////


    // spiRegisterWrite(MODE_SEL, mode, &value);

    // printf("Was there: %lX\n", value);

    // spiRegisterWrite(MODE_SEL, mode, &value);

    // printf("Is there: %lX\n", value);


    // uint32_t mode = spiRegisterRead(MODE_SEL);
    // printf("MODE: %lX\n", mode);

    // mode |= STANDBY_MODE;
    // printf("Target value: %lX\n",mode);
    // spiRegisterWrite(MODE_SEL, mode);

    // usleep(1000000);

    // mode = spiRegisterRead(MODE_SEL);
    // printf("Actual value: %lX\n", mode);

    // uint32_t init = spiRegisterRead(CCCR);
    // printf("CCCR: %lu\n", init);

    // //init |= (CAN_CCCR_CCE | CAN_CCCR_INIT);
    // init &= ~CAN_CCCR_CSR;

    // printf("Updated CCCR value: %lu\n", init);

    // spiRegisterWrite(CCCR, (0b01 << 0U));

    // usleep(1000000);

    // uint32_t new_init = spiRegisterRead(CCCR);

    // printf("New CCCR: %lu\n", new_init);

/*
    // Standby Mode Check
    if ((mode & 0xC0) != STANDBY_MODE)
    {   printf("Device not in STANDBY_MODE, MODE: %lu, setting STANDBY_MODE\n", mode & 0xC0);
        mode |= STANDBY_MODE;
        spiRegisterWrite(MODE_SEL, mode);
        printf("STANDBY_MODE set successfully\n");

        mode = spiRegisterRead(MODE_SEL);
        printf("New device mode: %lu, NEEDED: %d\n", mode & 0xC0, STANDBY_MODE);
    }

    printf("Device in STANDBY_MODE, mode: %lu\n", mode);

    printf("MRAM gets zeroed out\n");


    // Initialization Mode
    // TODO: Check whether CSR needs to be written before CCE and INIT
    uint32_t init = spiRegisterRead(CCCR);

    printf("Initial CCCR content: %lu\n", init);

    init |= (CAN_CCCR_CCE | CAN_CCCR_INIT);
    init &= ~CAN_CCCR_CSR;

    printf("Updated CCCR value: %lu\n", init);
    
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
        spiRegisterWrite(CCCR, init);

        init = spiRegisterRead(CCCR);

        printf("New CCCR reg value: %lu\n", init);
    
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

        printf("Updated NBTP value: %lu\n", bit_timing);

        spiRegisterWrite(NBTP, bit_timing);

        bit_timing = spiRegisterRead(NBTP);

        printf("New NBTP reg content: %lu\n", bit_timing);

    }
    else if (CAN_MODE == 1)
    {
        printf("Device in CAN-FD mode\n");
        // Check BRS and FD settings
        if (BIT_RATE_SWITCH)    init |= CCCR_BRSE;
        if (FD)     init |= CCCR_FDOE;

        spiRegisterWrite(CCCR, init);

        bit_timing       =  spiRegisterRead(DBTP);
        trans_delay_comp =  spiRegisterRead(TDCR);
        
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

        spiRegisterWrite(DBTP, bit_timing);

        spiRegisterWrite(TDCR, trans_delay_comp);
    }

    // MRAM Init
    
    uint32_t sid        = spiRegisterRead(SIDFC);
    uint32_t xid        = spiRegisterRead(XIDFC);
    uint32_t rxf0       = spiRegisterRead(RXF0C);
    uint32_t rxf1       = spiRegisterRead(RXF1C);
    uint32_t rxb        = spiRegisterRead(RXBC);
    uint32_t rx         = spiRegisterRead(RXESC);
    uint32_t tx_fifo    = spiRegisterRead(TXEFC);
    uint32_t txb        = spiRegisterRead(TXBC);
    uint32_t tx         = spiRegisterRead(TXESC);

    printf("MRAM content:\nSID: %lu\nXID: %lu\nRXF0: %lu\nRXF1: %lu\nRXB: %lu\nRX: %lu\nTX_FIFO: %lu\nTXB: %lu\nTX: %lu\n",
            sid, xid, rxf0, rxf1, rxb, rx, tx_fifo, txb, tx);

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

    printf("Updated MRAM content:\nSID: %lu\nXID: %lu\nRXF0: %lu\nRXF1: %lu\nRXB: %lu\nRX: %lu\nTX_FIFO: %lu\nTXB: %lu\nTX: %lu\n",
            sid, xid, rxf0, rxf1, rxb, rx, tx_fifo, txb, tx);

    spiRegisterWrite(SIDFC, sid);
    spiRegisterWrite(XIDFC, xid);
    spiRegisterWrite(RXF0C, rxf0);
    spiRegisterWrite(RXF1C, rxf1);
    spiRegisterWrite(RXBC, rxb);
    spiRegisterWrite(RXESC, rx);
    spiRegisterWrite(TXEFC, tx_fifo);
    spiRegisterWrite(TXBC, txb);
    spiRegisterWrite(TXESC, tx);
    
    sid =spiRegisterRead(SIDFC);
    xid = spiRegisterRead(XIDFC);
    rxf0 = spiRegisterRead(RXF0C);
    rxf1 = spiRegisterRead(RXF1C);
    rxb = spiRegisterRead(RXBC);
    rx = spiRegisterRead(RXESC);
    tx_fifo = spiRegisterRead(TXEFC);
    txb = spiRegisterRead(TXBC);
    tx = spiRegisterRead(TXESC);
    

    printf("New MRAM regs content:\nSID: %lu\nXID: %lu\nRXF0: %lu\nRXF1: %lu\nRXB: %lu\nRX: %lu\nTX_FIFO: %lu\nTXB: %lu\nTX: %lu\n",
            sid, xid, rxf0, rxf1, rxb, rx, tx_fifo, txb, tx);

    // Put the TCAN45xx device into "NORMAL" mode
    spiRegisterWrite(MODE_SEL, NORMAL_MODE);
    printf("Device in NORMAL_MODE, init_can successful\n");

    uint32_t selected_mode = spiRegisterRead(MODE_SEL);

    printf("New selected mode: %lu\n", selected_mode);

*/
    return 0;
}
/*
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

        spiRegisterWrite(filter_addr + i * sizeof(uint32_t), filter);
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

        spiRegisterWrite(filter_addr + i, filter);
    }

    return 0;
}

uint8_t sendCAN(TiMRAMParams * MRAM, TXElement * TXE)
{
    // Check that any TX Buffer is vacant
    uint32_t free_level = spiRegisterRead(TXFQS);

    uint32_t tx_buffer_start_addr = MRAM -> TXB_TBSA;
    uint32_t tx_buffer_element_size = MRAM -> TX_TBDS + 8; // Additional 8 bytes of header


    
    if (!(TFFL(free_level)))
    {
        return 1;
    }

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

    spiRegisterWrite(memory_offset, word_0);

    word_1 |= ((TXE -> MM) << MM_SHFT);

    if (TXE -> EFC) word_1 |= (1 << EFC_SHFT);
    if (TXE -> FDF) word_1 |= (1 << FDF_SHFT);
    if (TXE -> BRS) word_1 |= (1 << BRS_SHFT);
    
    word_1 |= ((TXE -> DLC) << DLC_SHFT);

    spiRegisterWrite(memory_offset + sizeof(word_0), word_1);

    word_2 |= ( ((TXE -> data_byte_0) << DB0_SHFT)   |
                ((TXE -> data_byte_1) << DB1_SHFT)   |
                ((TXE -> data_byte_2) << DB2_SHFT)   |
                ((TXE -> data_byte_3) << DB3_SHFT)   );
    
    spiRegisterWrite(memory_offset + sizeof(word_0) + sizeof(word_1), word_2);

    uint32_t add_request = 0;

    add_request |= (1U << index);

    spiRegisterWrite(TXBUF_AR, add_request);

    return 0;
}

*/
