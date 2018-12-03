#include "honyar_common.h"


void ICACHE_FLASH_ATTR honyar_flash_erase(uint32_t addr, uint32_t len)
{
    if(!len) {
        return;
    }
    uint32_t start_sec = addr / SPI_FLASH_SEC_SIZE;
    uint32_t end_sec = (addr + len - 1) / SPI_FLASH_SEC_SIZE;

    uint32_t sec = 0;
    hy_debug("erase from %u to %u.\r\n", start_sec, end_sec);
    for (sec = start_sec; sec <= end_sec; sec++)
    {
        uint8_t ret = spi_flash_erase_sector(sec);
        if(ret)
        {
            hy_error("erase failed. ret = %d \r\n", ret);
        }
    }
    hy_debug("erase done.\r\n");
}

void ICACHE_FLASH_ATTR honyar_flash_write(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint32_t real_addr = addr;
    uint32_t real_len = len;
    uint8_t *preal_buf = buf;
    uint32_t bufed_flag = 0;
    
    if (addr % 4 != 0)
    {
        real_addr = addr - (addr % 4);
        real_len += (addr % 4);
        bufed_flag = 1;
    }

    if (real_len % 4 != 0)
    {
        real_len += 4 - (real_len % 4);
        bufed_flag = 1;
    }

    preal_buf = (uint8_t *)os_malloc(real_len);
    if (bufed_flag)
    {
        spi_flash_read(real_addr, (uint32_t *)preal_buf, real_len);
    }
    memcpy(&preal_buf[(addr % 4)], buf, len);
    uint8_t ret = spi_flash_write(real_addr, (uint32_t *)preal_buf, real_len);
    if(ret)
    {
        hy_error("write failed. ret = %d \r\n", ret);
    }
    os_free(preal_buf);
}

void ICACHE_FLASH_ATTR honyar_flash_read(uint32_t addr, uint8_t *buf,uint32_t len)
{
    uint32_t real_addr = addr;
    uint32_t real_len = len;
    uint8_t *preal_buf = buf;
    
    if (addr % 4 != 0)
    {
        real_addr = addr - (addr % 4);
        real_len += (addr % 4);
    }
    if (real_len % 4 != 0)
    {
        real_len += 4 - (real_len % 4);
    }


    preal_buf = (uint8_t *)os_malloc(real_len);
    uint8_t ret = spi_flash_read(real_addr, (uint32_t *)preal_buf, real_len);
    if(ret)
    {
        hy_error("read failed. ret = %d \r\n", ret);
    }
    memcpy(buf, &preal_buf[(addr % 4)], len);
    os_free(preal_buf);
}


