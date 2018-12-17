
#ifdef DL2106F

#include "dl2106f.h"
#include "honyar_common.h"

#define POWER_COUNT_CYCLE (2000)


static uint8_t g_socket_elec_cal_flag;

//unit: 0.1w
static uint32_t g_socket_cur_power;
static uint32_t g_electricity_sum;
static uint32_t g_socket_elec_energy_pluse_num;
static os_timer_t g_elec_cal_timer;

//unit:0.1v
static uint32_t g_socket_curent_voltage;
static uint32_t g_socket_voltage_220v_pluse_num;
static os_timer_t g_voltage_cal_timer;

static uint8_t g_socket_power_state = POWER_ON;

uint8_t ICACHE_FLASH_ATTR
dl2106f_get_socket_power_state(void)
{
    return g_socket_power_state;
}

static void ICACHE_FLASH_ATTR
dl2106f_set_socket_power_on(void)
{
    honyar_gpio_set_output(DL_SOCKET_POWER_PIN_NUM, GPIO_HIGH);
    g_socket_power_state = POWER_ON;
}

static void ICACHE_FLASH_ATTR
dl2106f_set_socket_power_off(void)
{
    honyar_gpio_set_output(DL_SOCKET_POWER_PIN_NUM, GPIO_LOW);
    g_socket_power_state = POWER_OFF;
}

void ICACHE_FLASH_ATTR
dl2106f_set_socket_power(uint8_t state)
{
    if(state == g_socket_power_state) {
        return;
    }
    
    if(state) {
        dl2106f_set_socket_power_on();
    } else {
        dl2106f_set_socket_power_off();
    }
    dl_config_commit_later();
}


uint32_t ICACHE_FLASH_ATTR
dl2106f_get_electricity_energe(void)
{
    if(g_socket_elec_cal_flag && g_socket_elec_energy_pluse_num) {
        return g_electricity_sum / g_socket_elec_energy_pluse_num;
    }

    return 0;
}

uint32_t ICACHE_FLASH_ATTR
dl2106f_get_electricity_cur_power(void)
{
    return g_socket_cur_power;
}

static void ICACHE_FLASH_ATTR
dl2106f_cal_electricity_task(void *args)
{
    static uint8_t last_cal_value = GPIO_UNKNOWN;
    static uint32_t electricity_record = 0;
    static uint32_t last_cal_time = 0;
    uint8_t cur_value = 0;
    uint32_t cur_time = system_get_time() / 1000;
    
    cur_value = honyar_gpio_get_input(DL_ELEC_CAL_PIN_NUM);
    if((GPIO_HIGH == cur_value) && (GPIO_LOW == last_cal_value)){
        g_electricity_sum++;
        electricity_record++;
    }
    last_cal_value = cur_value;
    
    /*¹¦ÂÊ¼ÆËã*/
    if(0 == last_cal_time || 0 == g_socket_elec_energy_pluse_num){
        g_socket_cur_power = 0;
        last_cal_time = cur_time;
        electricity_record = 0;
    } else if((electricity_record >= 10) && (cur_time - last_cal_time >= POWER_COUNT_CYCLE)){
        g_socket_cur_power =  (3600 * 1000  * electricity_record) / ((cur_time - last_cal_time) * g_socket_elec_energy_pluse_num);
        g_socket_cur_power *= 10;
        last_cal_time = cur_time;
        electricity_record = 0;
    } else if((electricity_record < 10) && (electricity_record >= 1 )
        && ((cur_time - last_cal_time) >= (3600 * 1000 / g_socket_elec_energy_pluse_num))){
        g_socket_cur_power =  (3600 * 10000  * electricity_record) / ((cur_time - last_cal_time) * g_socket_elec_energy_pluse_num);
        electricity_record = 0;
        last_cal_time = cur_time;
    } else if((electricity_record == 0) && ((cur_time - last_cal_time) >= (3600 * 1000 / g_socket_elec_energy_pluse_num))) {
        g_socket_cur_power = 0;
        last_cal_time = cur_time;
        electricity_record = 0;
    }
}


uint32_t ICACHE_FLASH_ATTR
dl2106f_get_socket_cur_voltage(void)
{
    return g_socket_curent_voltage;
}

static void ICACHE_FLASH_ATTR
dl2106f_cal_voltage_task(void *args)
{
    static uint8_t last_voltage_value = GPIO_UNKNOWN;
    static uint32_t voltage_record = 0;
    uint8_t cur_value = 0;
    uint32_t cur_time = system_get_time() / 1000;
    static uint32_t last_cal_time = 0;
    cur_value = honyar_gpio_get_input(DL_VOLTAGE_PIN_NUM);
    if((GPIO_HIGH == cur_value) && (GPIO_LOW == last_voltage_value)) {
        voltage_record++;
    }
    last_voltage_value = cur_value;

    if(0 == last_cal_time || 0 == g_socket_voltage_220v_pluse_num) {
        g_socket_curent_voltage = 0;
        voltage_record = 0;
        last_cal_time = cur_time;
    } else if(cur_time - last_cal_time >= 1000) {
        g_socket_curent_voltage = (voltage_record * 220 * 1000 * 10) / ((cur_time - last_cal_time) * g_socket_voltage_220v_pluse_num);
        voltage_record = 0;
        last_cal_time = cur_time;
    }

}

void ICACHE_FLASH_ATTR
dl2106f_config_init(void)
{
    DL_CONFIG_ITEM_S config_items[] = {
        {"_SOCKET_ELEC_CAL_FLAG", DL_CFG_ITEM_TYPE_DEC8, &g_socket_elec_cal_flag, sizeof(g_socket_elec_cal_flag), 0},
        {"_SOCKET_ELEC_ENERGY_PLUSE_NUM", DL_CFG_ITEM_TYPE_DEC32, &g_socket_elec_energy_pluse_num, sizeof(g_socket_elec_energy_pluse_num), 0},
        {"_SOCKET_VOLTAGE_220V_PLUSE_NUM", DL_CFG_ITEM_TYPE_DEC32, &g_socket_voltage_220v_pluse_num, sizeof(g_socket_voltage_220v_pluse_num), 0},
        {"_SOCKET_POWER_DEFAULT", DL_CFG_ITEM_TYPE_DEC8, &g_socket_power_state, sizeof(g_socket_power_state), 0},
	};

	uint32_t i;
	for (i = 0; i < sizeof(config_items)/sizeof(config_items[0]); i++) {
        dl_config_items_register_by_user(&config_items[i]);
    }
}

void ICACHE_FLASH_ATTR
dl2106f_init(void)
{
    os_timer_disarm(&g_voltage_cal_timer);
    os_timer_setfn(&g_voltage_cal_timer, dl2106f_cal_voltage_task, NULL);
    os_timer_arm_us(&g_voltage_cal_timer, 500, TRUE);

    os_timer_disarm(&g_elec_cal_timer);
    os_timer_setfn(&g_elec_cal_timer, dl2106f_cal_electricity_task, NULL);
    os_timer_arm_us(&g_elec_cal_timer, 500, TRUE);

    if(POWER_OFF == dl2106f_get_socket_power_state()) {
        dl2106f_set_socket_power_off();
    } else {
        dl2106f_set_socket_power_on();
    }
}


#endif

