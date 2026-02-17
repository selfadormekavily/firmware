/*
 * @Description: 
 * @Version: 1.0
 * @Autor: lmx
 * @Date: 2022-02-23 14:52:23
 * @LastEditors:  
 * @LastEditTime: 2022-02-23 15:07:39
 */
#include "jbd013/jbd013_api.h"
#include <string.h>

/**
 * @description: 
 * @param {*}
 * @return {*}
 * @author: lmx
 */
int main(void)
{
    u32 col, row;
    u8 imgBuf[153600];

    panel_init(); //Initialize panel

    /* Set brightness, not required */
    wr_lum_reg(1234); //Write luminance register
    wr_cur_reg(56);   //Write current register

    while (1)
    {
        /* Show 2x2 squares */
        for (row = 0; row < 480; row++)
        {
            for (col = 0; col < 640 / 2; col++)
            {
                if (((col < (640 / 2 / 2)) && (row < (480 / 2))) || ((col >= (640 / 2 / 2)) && (row >= (480 / 2))))
                {
                    imgBuf[col + row * 320] = 0xff;
                }
                else
                    imgBuf[col + row * 320] = 0x00;
            }
        }
        display_image(imgBuf, sizeof(imgBuf)); //Display
        delay_ms(1000);

        /* clear screen */
        memset(imgBuf, 0, sizeof(imgBuf));     //Clear buffer
        display_image(imgBuf, sizeof(imgBuf)); //Display
        delay_ms(1000);
    }

    return 0;
}