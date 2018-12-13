
#include "sigma_delta.h"
#include "ir_tx.h"
#include "infared/honyar_ir.h"
#include "honyar_common.h"


#define DELAN_IR_SIGMA_DELTA 0
#define DELAN_IR_IIS         1
#define DELAN_IR_SELECT      DELAN_IR_SIGMA_DELTA


static os_timer_t g_ir_tx_timer;
static IR_DATA_STATE_E g_ir_tx_state = IR_TX_IDLE;
static uint8_t *g_ir_tx_data = NULL;
static uint32_t g_ir_tx_data_len;
static uint32_t g_ir_tx_time_unit;
static uint32_t g_ir_tx_level_count;
static IR_DATA_MODE_E g_ir_data_mode = IR_UNKNOWN_MODE;


#if (DELAN_IR_SELECT == DELAN_IR_SIGMA_DELTA)
/*‘ÿ≤®38k*/
static void ICACHE_FLASH_ATTR
gen_carrier_clk()
{
#if 1
    uint8_t pin_num = honyar_gpio_find(DELAN_IR_TX_PIN_NUM);
    uint32_t pin_mux;
    uint32_t pin_fun;
    if(PIN_IO_NONE == pin_num)
    {
        return;
    }
    pin_mux = GPIO_PIN_REG(pin_num);
    if ((0x1 << pin_num) & (GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5))
    {
         pin_fun = 0;
    }
    else
    {
         pin_fun = 3;
    }
#else
    uint8_t pin_num = 14;
    uint32_t pin_mux = PERIPHS_IO_MUX_MTMS_U;
    uint32_t pin_fun = 3;
#endif
	sigma_delta_gen_38k(pin_mux, pin_num, pin_fun);
}


static void ICACHE_FLASH_ATTR
ir_tx_carrier_clr()
{
#if 1
    uint8_t pin_num = honyar_gpio_find(DELAN_IR_TX_PIN_NUM);
    if(PIN_IO_NONE == pin_num)
    {
        return;
    }
#else
    uint8_t pin_num = 14;
#endif
    sigma_delta_close(pin_num);
}

#else

#define I2S_BCK_DIV_NUM 		0x0000003F
#define I2S_BCK_DIV_NUM_S 		22
#define I2S_CLKM_DIV_NUM 		0x0000003F
#define I2S_CLKM_DIV_NUM_S 		16
#define I2S_BITS_MOD 			0x0000000F
#define I2S_BITS_MOD_S 			12
#define I2SCONF					(DR_REG_I2S_BASE + 0x0008)
#define DR_REG_I2S_BASE 		(0x60000e00)

#define i2c_bbpll         						0x67
#define i2c_bbpll_en_audio_clock_out        	4
#define i2c_bbpll_en_audio_clock_out_msb    	7
#define i2c_bbpll_en_audio_clock_out_lsb    	7
#define i2c_bbpll_hostid       					4

static void gen_carrier_clk()
{
    uint8_t pin_num = honyar_gpio_find(DELAN_IR_TX_PIN_NUM);
    uint32_t pin_mux;
    uint32_t pin_fun;
    if(PIN_IO_NONE == pin_num)
    {
        return;
    }
    pin_mux = GPIO_PIN_REG(pin_num);
    if ((0x1 << pin_num) & (GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5))
    {
         pin_fun = 0;
    }
    else
    {
         pin_fun = 3;
    }


    //ENABLE I2S CLK SOURCE
	rom_i2c_writeReg_Mask(i2c_bbpll, i2c_bbpll_hostid, i2c_bbpll_en_audio_clock_out, i2c_bbpll_en_audio_clock_out_msb, i2c_bbpll_en_audio_clock_out_lsb, 1);
	//CONFIG AS I2S 
//#if (pin_mux == PERIPHS_IO_MUX_GPIO2_U)
#if 0
	//set i2s clk freq 
	WRITE_PERI_REG(I2SCONF,  READ_PERI_REG(I2SCONF) & 0xf0000fff|
	                    ( (( 62&I2S_BCK_DIV_NUM )<<I2S_BCK_DIV_NUM_S)|
	                    ((2&I2S_CLKM_DIV_NUM)<<I2S_CLKM_DIV_NUM_S)|
	                    ((1&I2S_BITS_MOD  )   <<  I2S_BITS_MOD_S )  )  );

	WRITE_PERI_REG(pin_mux, (READ_PERI_REG(pin_mux)&0xfffffe0f)| (0x1<<4) );
	WRITE_PERI_REG(0x60000e08, READ_PERI_REG(0x60000e08) & 0xfffffdff | (0x1<<8) ); //i2s tx  start
#endif

//#if (pin_mux == PERIPHS_IO_MUX_MTMS_U)
#if 1
	//set i2s clk freq 
	WRITE_PERI_REG(I2SCONF,  READ_PERI_REG(I2SCONF) & 0xf0000fff|
	                    ( (( 62&I2S_BCK_DIV_NUM )<<I2S_BCK_DIV_NUM_S)|  
	                    ((2&I2S_CLKM_DIV_NUM)<<I2S_CLKM_DIV_NUM_S)|   
	                    ((1&I2S_BITS_MOD  )   <<  I2S_BITS_MOD_S )  )  );

	WRITE_PERI_REG(pin_mux, (READ_PERI_REG(pin_mux)&0xfffffe0f)| (0x1<<4) );
	WRITE_PERI_REG(0x60000e08, READ_PERI_REG(0x60000e08) & 0xfffffdff | (0x2<<8) ) ;//i2s rx  start
#endif
//#if (pin_mux == PERIPHS_IO_MUX_MTCK_U)
#if 0
	//set i2s clk freq 
	WRITE_PERI_REG(I2SCONF,  READ_PERI_REG(I2SCONF) & 0xf0000fff|
	                    ( (( 63&I2S_BCK_DIV_NUM )<<I2S_BCK_DIV_NUM_S)|
	                    ((63&I2S_CLKM_DIV_NUM)<<I2S_CLKM_DIV_NUM_S)| 
	                    ((1&I2S_BITS_MOD  )   <<  I2S_BITS_MOD_S )  )  );

	WRITE_PERI_REG(pin_mux, (READ_PERI_REG(pin_mux)&0xfffffe0f)| (0x1<<4) );
	WRITE_PERI_REG(0x60000e08, READ_PERI_REG(0x60000e08) & 0xfffffdff | (0x2<<8) ) ;//i2s rx  start
#endif
//#if (pin_mux == PERIPHS_IO_MUX_MTDO_U)
#if 0
	//set i2s clk freq 
	WRITE_PERI_REG(I2SCONF,  READ_PERI_REG(I2SCONF) & 0xf0000fff|
	                    ( (( 63&I2S_BCK_DIV_NUM )<<I2S_BCK_DIV_NUM_S)|
	                    ((63&I2S_CLKM_DIV_NUM)<<I2S_CLKM_DIV_NUM_S)|
	                    ((1&I2S_BITS_MOD  )   <<  I2S_BITS_MOD_S )));

	WRITE_PERI_REG(pin_mux, (READ_PERI_REG(pin_mux)&0xfffffe0f)| (0x1<<4) );
	WRITE_PERI_REG(0x60000e08, READ_PERI_REG(0x60000e08) & 0xfffffdff | (0x1<<8) ); //i2s tx  start
#endif

}

static void ir_tx_carrier_clr()
{
    uint8_t pin_num = honyar_gpio_find(DELAN_IR_TX_PIN_NUM);
    uint32_t pin_mux;
    uint32_t pin_fun;
    if(PIN_IO_NONE == pin_num)
    {
        return;
    }
    pin_mux = GPIO_PIN_REG(pin_num);
    if ((0x1 << pin_num) & (GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5))
    {
         pin_fun = 0;
    }
    else
    {
         pin_fun = 3;
    }

    PIN_FUNC_SELECT(pin_mux, pin_fun); 

    GPIO_OUTPUT_SET(pin_num, GPIO_HIGH);

    WRITE_PERI_REG(0x60000e08, READ_PERI_REG(0x60000e08) & 0xfffffdff | (0x0<<8) ) ; //i2s clk stop
}

#endif

static void ICACHE_FLASH_ATTR
dl_ir_tx_data_mode(void)
{
    
}

static void ICACHE_FLASH_ATTR
dl_ir_tx_wave_mode(void)
{
    static uint32_t ir_tx_data_cnt = 0;
    static uint8_t ir_tx_level = 0;
    uint32_t ir_tx_time_out = 0;
    uint32_t ir_tx_data_offset = 0;
    if(IR_TX_IDLE == g_ir_tx_state)
    {
        g_ir_tx_state = IR_TX_DATA;
        ir_tx_data_cnt = 0;
        ir_tx_data_offset = ir_tx_data_cnt * 2;
        ir_tx_level = 1;
        ir_tx_time_out = g_ir_tx_data[ir_tx_data_offset] + (g_ir_tx_data[ir_tx_data_offset + 1] << 8);
        ir_tx_time_out *= g_ir_tx_time_unit;
        ir_tx_data_cnt++;
        gen_carrier_clk();
        //os_timer_arm_us(&g_ir_tx_timer, ir_tx_time_out, 0);
        hw_timer_arm(ir_tx_time_out);
        TM1_EDGE_INT_ENABLE();
    }
    else
    {
        if(ir_tx_data_cnt >= g_ir_tx_level_count)
        {
            ir_tx_carrier_clr();
            if(g_ir_tx_data)
            {
                os_free(g_ir_tx_data);
                g_ir_tx_data = NULL;
            }
            g_ir_tx_state = IR_TX_IDLE;
            //dl_gpio_config(DELAN_IR_TX_PIN_NUM, DL_INTPUT, DL_PULL_DOWN);
			hy_info("ir send over.\r\n");
            return;
        }
        ir_tx_data_offset = ir_tx_data_cnt * 2;
        ir_tx_time_out = g_ir_tx_data[ir_tx_data_offset] + (g_ir_tx_data[ir_tx_data_offset + 1] << 8);
        ir_tx_time_out *= g_ir_tx_time_unit;
        if(ir_tx_level)
        {
            ir_tx_carrier_clr();
            ir_tx_level = 0;
        }
        else
        {
            gen_carrier_clk();
            ir_tx_level = 1;
        }
        
        //os_timer_arm_us(&g_ir_tx_timer, ir_tx_time_out, 0);
        hw_timer_arm(ir_tx_time_out);
        ir_tx_data_cnt++;
        TM1_EDGE_INT_ENABLE();
    }
}

static void ICACHE_FLASH_ATTR
dl_ir_tx_timer_cb(void)
{
    //os_timer_disarm(&g_ir_tx_timer);
    TM1_EDGE_INT_DISABLE();
    if(IR_DATA_MODE == g_ir_data_mode)
    {
        dl_ir_tx_data_mode();
    }
    else if(IR_WAVE_MODE == g_ir_data_mode)
    {
        dl_ir_tx_wave_mode();
    }
    
}

uint32_t ICACHE_FLASH_ATTR
dl_irda_send(uint8_t *data, uint32_t data_len, IR_DATA_MODE_E ir_mode)
{
#if 0
    if(IR_DATA_MAX_LEN < data_len)
    {
        return 0;
    }
#endif
//    PrintHexData("IR_SEND:",data, data_len);
    if(IR_TX_IDLE != g_ir_tx_state)
    {
        hy_error("IR TX BUSY.\r\n");
        return 0;
    }
    
    if(IR_DATA_MODE != ir_mode && IR_WAVE_MODE != ir_mode)
    {
        return 0;
    }
#if 1
    if(0x26 != data[0])
    {
        hy_error("Not 38k.\r\n");
        return 0;
    }

    g_ir_tx_time_unit = data[1];
    g_ir_tx_level_count = data[4] + (data[5] << 8);
    if((g_ir_tx_level_count * 2) != (data_len - 6))
    {
        return 0;
    }
#endif
    
    if(g_ir_tx_data)
    {
        os_free(g_ir_tx_data);
        g_ir_tx_data = NULL;
    }

    g_ir_tx_data = (uint8_t *)os_malloc(data_len - 6);
    memset(g_ir_tx_data, 0, data_len - 6);
    memcpy(g_ir_tx_data, data + 6, data_len - 6);
    g_ir_tx_data_len = data_len - 6;

    hex_printf("IR_SEND:", g_ir_tx_data, g_ir_tx_data_len);
    g_ir_data_mode = ir_mode;
    dl_ir_tx_timer_cb();
    return 1;
}

void ICACHE_FLASH_ATTR
dl_ir_tx_init()
{
#if 0
    os_timer_disarm(&g_ir_tx_timer);
    os_timer_setfn(&g_ir_tx_timer, dl_ir_tx_timer_cb, NULL);
#else
    hw_timer_init(0);
    hw_timer_set_func(dl_ir_tx_timer_cb);
    //dl_gpio_config(DELAN_IR_TX_PIN_NUM, DL_INTPUT, DL_PULL_NONE);
#endif
}

