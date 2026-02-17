/*
 * @Description: API function of JBD013VGA panel
 * @version: 1.3
 * @Autor: lmx
 * @Date: 2020-05-23 15:24:54
 * @LastEditors:
 * @LastEditTime: 2022-06-14 09:11:45
 */
#include "jbd013_api.h"

/**
 * @description: Parse temperature sensor data
 * @param {u8} *pBuf
 * @return {*}
 * @author:
 */
u16 __parse_temperature_sensor_data(u8 *pBuf)
{
    u8 maskBuf[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
    u8 tmpNum;
    u8 isFlag;
    u16 tmpVal = 0;
    u16 i;
    char rxBitCnt = -2;
    char bitCnt;

    for (i = 0, isFlag = 0; i < 2000; i++) // Parsing data
    {
        if (pBuf[i] > 0)
        {
            tmpNum = pBuf[i];

            for (bitCnt = 7; bitCnt >= 0; bitCnt--)
            {
                if (rxBitCnt == -2)
                {
                    if (isFlag == 0 && ((tmpNum & maskBuf[bitCnt]) >= 1))
                    {
                        isFlag = 1;
                    }
                    else if (isFlag == 1 && ((tmpNum & maskBuf[bitCnt]) == 0))
                    {
                        isFlag = 2;
                    }
                    else if (isFlag == 2 && ((tmpNum & maskBuf[bitCnt]) == 0))
                    {
                        isFlag = 3;
                    }
                    else if (isFlag == 3 && ((tmpNum & maskBuf[bitCnt]) >= 1))
                    {
                        isFlag = 4;
                        rxBitCnt = 11;
                        continue;
                    }
                    else
                    {
                        isFlag = 0;
                    }
                }
                if (rxBitCnt >= 0)
                {
                    tmpVal |= (((tmpNum & maskBuf[bitCnt]) >> bitCnt) << rxBitCnt);
                    rxBitCnt--;
                    if (rxBitCnt == -1)
                        break;
                }
            }
        }
        else
        {
            isFlag = 0;
        }
        if (rxBitCnt == -1)
            break;
    }

    return tmpVal;
}

/**
 * @description: Get temperature compensation data
 * @param {u8} sensorId
 * @return {*}
 * @author:
 */
u16 __get_temperature_compensation_data(u16 ADDR_OTP_TEMP)
{
    u8 pBuf[12];
    u16 cData;

    spi_rd_opt_data(ADDR_OTP_TEMP, pBuf);
    cData = (((pBuf[1] & 0xF) << 8) + pBuf[0]);
    return cData;
}

/**
 * @description: Send command
 * @paran:
 * @return {*}
 * @author: lmx
 * @param {u8} cmd：SPI instruction of JBD013VGA panel
 */
void send_cmd(u8 cmd)
{
    spi_wr_byte(cmd);
}

/**
 * @description: Read the ID of the panel
 * @paran:
 * @return {u32}：ID of the panel
 * @author: lmx
 */
u32 read_id(void)
{
    u32 ret;
    u8 pBuf[3];

    spi_rd_bytes(SPI_RD_ID, pBuf, sizeof(pBuf));
    ret = pBuf[0] << 16 | pBuf[1] << 8 | pBuf[2];

    return ret;
}

/**
 * @description: Read the unique ID of the panel
 * @paran:
 * @return {*}
 * @author: lmx
 * @param {u8} *pBuf：The receive pointer of uid.
 * The corresponding cache space of the pointer should be greater than or equal to 15bytes
 */
void read_uid(u8 *pBuf)
{
    spi_rd_bytes(SPI_RD_UID, pBuf, 15);
}

/**
 * @description: Write status register
 * @paran:
 * @return {*}
 * @author: lmx
 * @param {u8} regAddr：Register address
 * @param {u8} data：Write data
 */
void wr_status_reg(u8 regAddr, u8 data)
{
    spi_wr_bytes(regAddr, &data, 1);
}

/**
 * @description: Read status register
 * @paran:
 * @return {u8}：Register data
 * @author: lmx
 * @param {u8} regAddr：Register address
 */
u8 rd_status_reg(u8 regAddr)
{
    return spi_rd_byte(regAddr);
}

/**
 * @description: Write offset register
 * @paran:
 * @return {*}
 * @author: lmx
 * @param {u8} row：Row offset address（0~31）
 * @param {u8} col：Col offset address（0~31）
 */
void wr_offset_reg(u8 row, u8 col)
{
    u8 pBuf[2];

    pBuf[0] = row;
    pBuf[1] = col;
    spi_wr_bytes(SPI_WR_OFFSET_REG, pBuf, 2);
    send_cmd(SPI_SYNC); // Synchronous offset reg data
    delay_ms(1);        // SYNC needs 1ms time(8MHz system clock) or 0.5ms time(16MHz system clock) to finish operation.
}

/**
 * @description: Read offset register
 * @paran:
 * @return {u16}：returnData = row << 8 | col
 * @author: lmx
 */
u16 rd_offset_reg(void)
{
    u8 pBuf[2];
    u16 ret;

    spi_rd_bytes(SPI_RD_OFFSET_REG, pBuf, sizeof(pBuf));
    ret = pBuf[0] << 8 | pBuf[1];

    return ret;
}

/**
 * @description: Write current register
 * @paran:
 * @return {*}
 * @author: lmx
 * @param {u8} param：Write data（0~63）
 */
void wr_cur_reg(u8 param)
{
    spi_wr_bytes(SPI_WR_CURRENT_REG, &param, 1);
}

/**
 * @description: Read current register
 * @paran:
 * @return {u8}：Read data
 * @author: lmx
 */
u8 rd_cur_reg(void)
{
    return spi_rd_byte(SPI_RD_CURRENT_REG);
}

/**
 * @description: Write luminance register
 * @paran:
 * @return {*}
 * @author: lmx
 * @param {u16} param：Write data
 * When the self refresh frequency is 25Hz, param (0 ~ 21331)
 * When the self refresh frequency is 50Hz, param (0 ~ 10664)
 * When the self refresh frequency is 75Hz, param (0 ~ 7109)
 * When the self refresh frequency is 100Hz, param (0 ~ 5331)
 * When the self refresh frequency is 125Hz, param (0 ~ 4264)
 * When the self refresh frequency is 150Hz, param (0 ~ 3366)
 * When the self refresh frequency is 175Hz, param (0 ~ 2907)
 * When the self refresh frequency is 200Hz, param (0 ~ 2558)
 */
void wr_lum_reg(u16 param)
{
    u8 pBuf[2];

    pBuf[0] = param >> 8;
    pBuf[1] = param;
    spi_wr_bytes(SPI_WR_LUM_REG, pBuf, 2);
}

/**
 * @description: Read luminance register
 * @paran:
 * @return {u16}：Read data
 * @author: lmx
 */
u16 rd_lum_reg(void)
{
    u8 pBuf[2];
    u16 ret;

    spi_rd_bytes(SPI_RD_LUM_REG, pBuf, sizeof(pBuf));
    ret = pBuf[0] << 8 | pBuf[1];

    return ret;
}

/**
 * @description: Set mirror mode
 * @paran:
 * @return {*}
 * @author: lmx
 * @param {u8} param：Set mirror mode
 * param = 0：Normal display
 * param = 1: Mirror left and right only
 * param = 2: Mirror up and down only
 * param = 3: Mirror up, down, left and right at the same time
 */
void set_mirror_mode(u8 param)
{
    send_cmd(SPI_DISPLAY_DEFAULT_MODE);
    if (param == 1 || param == 3)
    {
        send_cmd(SPI_DISPLAY_RL);
    }
    if (param == 2 || param == 3)
    {
        send_cmd(SPI_DISPLAY_UD);
    }
    send_cmd(SPI_SYNC);
    delay_ms(1);
}

/**
 * @description: Write the data in cache to 0
 * @paran:
 * @return {*}
 * @author: lmx
 */
void clr_cache(void)
{
    u8 pBuf[10];
    u8 addrStep;
    u32 pBufLen;
    u16 rowCnt;
    u16 colCnt;

    pBufLen = sizeof(pBuf);
    addrStep = pBufLen * 2;
    memset(pBuf, 0, pBufLen);
    for (rowCnt = 0; rowCnt < 480; rowCnt++)
    {
        for (colCnt = 0; colCnt < 640; colCnt += addrStep)
        {
            spi_wr_cache(colCnt, rowCnt, pBuf, pBufLen);
        }
        if (640 % addrStep != 0)
        {
            spi_wr_cache((640 - 640 % addrStep), rowCnt, pBuf, 640 % addrStep);
        }
    }
}

/**
 * @description: Display image
 * @paran:
 * @return {*}
 * @author: lmx
 * @param {u8} *pBuf：Pointer to image data
 * @param {u32} len：Length of image data（0~153600）
 */
void display_image(u8 *pBuf, u32 len)
{
    spi_wr_cache(0, 0, pBuf, len); // Write data to cache
    send_cmd(SPI_SYNC);            // Synchronous cache data
    delay_ms(1);                   // SYNC needs 1ms time(8MHz system clock) or 0.5ms time(16MHz system clock) to finish operation.
}

/**
 * @description: Reset panel
 * @paran:
 * @return {*}
 * @author: lmx
 */
void panel_rst(void)
{
    send_cmd(SPI_RST_EN);
    send_cmd(SPI_RST);
    delay_ms(50);
}

/**
 * @description: Initialize panel
 * @paran:
 * @return {*}
 * @author: lmx
 */
void panel_init(void)
{
    // Reset panel
    panel_rst();

    // Open status register write enable
    send_cmd(SPI_WR_ENABLE);

    // Close demura
    wr_status_reg(SPI_WR_STATUS_REG1, 0x10);

    // Set all cache data to 0
    clr_cache();

    // Set the offset in the upper left corner
    wr_offset_reg(0, 0);

    // Set the offset in the upper right corner
    wr_offset_reg(0, 20);

    // Set the offset in the lower left corner
    wr_offset_reg(24, 0);

    // Set the offset in the lower right corner
    wr_offset_reg(24, 20);

    // Set actual offset,center the screen
    wr_offset_reg(12, 10);

    // Set current reg
    wr_cur_reg(63);

    // Set display enable
    send_cmd(SPI_DISPLAY_ENABLE);

    // Synchronous setting
    send_cmd(SPI_SYNC);
    delay_ms(1);
}

/**
 * @description: Get temperature sensor data
 * @paran:
 * @return {float}：Temperature data(Unit:℃)
 * @author: lmx
 * @param {u8} sensorId：Temperature sensor ID（Range：0~3）
 */
float get_temperature_sensor_data(u8 sensorId)
{
    u8 pBuf[2000];
    int tmpVal;
    int T30_Sensor_data;
    int T80_Sensor_data;
    spi_rd_temperature_sensor(sensorId, pBuf, sizeof(pBuf));                                      // Read the data of the temperature sensor inside the panel
    tmpVal = __parse_temperature_sensor_data(pBuf);                                               // Parse temperature sensor data
    T30_Sensor_data = __get_temperature_compensation_data(OTP_T30_TEMP_ADDR);                     // Get temperature compensation data
    T80_Sensor_data = __get_temperature_compensation_data(OTP_T80_TEMP_ADDR);                     // Get temperature compensation data
    return (float)(((tmpVal - T30_Sensor_data) * 50) / (T80_Sensor_data - T30_Sensor_data)) + 30; // Return temperature data(Unit:℃)
}
