#include "config.h"
#include <adc0832.h>
#include <timer.h>
typedef unsigned char uchar;

define DS1302_SEC_Reg 0x80
#define DS1302_MIN_Reg 0x82
#define DS1302_HR_Reg 0x84
#define DS1302_DATE_Reg 0x86
#define DS1302_MONTH_Reg 0x88
#define DS1302_DAY_Reg 0x8a
#define DS1302_YEAR_Reg 0x8c
#define DS1302_CONTROL_Reg 0x8e
#define DS1302_CHARGER_Reg 0x90
#define DS1302_CLKBURST_Reg 0xbe

/*********************************************************************/
/* 实时时钟模块 时钟芯片型号：DS1302 */
/*/
/*********************************************************************/
sbit T_CLK = P0^4; /*实时时钟时钟线引脚 */
sbit T_IO = P0^5; /*实时时钟数据线引脚 */
sbit T_RST = P0^3; /*实时时钟复位线引脚 */
/********************************************************************
*
* 名称: v_RTInputByte
* 说明:
* 功能: 往DS1302写入1Byte数据
* 调用:
* 输入: ucDa 写入的数据
* 返回值: 无
***********************************************************************/
void v_RTInputByte(uchar ucDa)
{
	uchar i;
	ACC = ucDa;
	for(i=8; i>0; i--)
	{
		T_IO = ACC0; /*相当于汇编中的 RRC */
		T_CLK = 1;
		T_CLK = 0;
		ACC = ACC >> 1;
	}
}
/********************************************************************
*
* 名称: uchar uc_RTOutputByte
* 说明:
* 功能: 从DS1302读取1Byte数据
* 调用:
* 输入:
* 返回值: ACC
***********************************************************************/
uchar uc_RTOutputByte(void)
{
uchar i;
for(i=8; i>0; i--)
{
ACC = ACC >>1; /*相当于汇编中的 RRC */
ACC7 = T_IO;
T_CLK = 1;
T_CLK = 0;
}
return(ACC);
}
/********************************************************************
*
* 名称: v_W1302
* 说明: 先写地址，后写命令/数据
* 功能: 往DS1302写入数据
* 调用: v_RTInputByte()
* 输入: ucAddr: DS1302地址, ucDa: 要写的数据
* 返回值: 无
***********************************************************************/
void v_W1302(uchar ucAddr, uchar ucDa)
{
T_RST = 0;
T_CLK = 0;
T_RST = 1;
v_RTInputByte(ucAddr); /* 地址，命令 */
v_RTInputByte(ucDa); /* 写1Byte数据*/
T_CLK = 1;
T_RST =0;
}
/********************************************************************
*
* 名称: uc_R1302
* 说明: 先写地址，后读命令/数据
* 功能: 读取DS1302某地址的数据
* 调用: v_RTInputByte() , uc_RTOutputByte()
* 输入: ucAddr: DS1302地址
* 返回值: ucDa :读取的数据
***********************************************************************/
uchar uc_R1302(uchar ucAddr)
{
uchar ucDa;
T_RST = 0;
T_CLK = 0;
T_RST = 1;
v_RTInputByte(ucAddr); /* 地址，命令 */
ucDa = uc_RTOutputByte(); /* 读1Byte数据 */
T_CLK = 1;
T_RST =0;
return(ucDa);
}
/********************************************************************
*
* 名称: v_BurstW1302T
* 说明: 先写地址，后写数据(时钟多字节方式)
* 功能: 往DS1302写入时钟数据(多字节方式)
* 调用: v_RTInputByte()
* 输入: pSecDa: 时钟数据地址 格式为: 秒 分 时 日 月 星期 年 控制
* 8Byte (BCD码) 1B 1B 1B 1B 1B 1B 1B 1B
* 返回值: 无
***********************************************************************/
void v_BurstW1302T(uchar *pSecDa)
{
uchar i;
v_W1302(0x8e,0x00); /* 控制命令,WP=0,写操作?*/
T_RST = 0;
T_CLK = 0;
T_RST = 1;
v_RTInputByte(0xbe); /* 0xbe:时钟多字节写命令 */
for (i=8;i>0;i--) /*8Byte = 7Byte 时钟数据 + 1Byte 控制*/
{
v_RTInputByte(*pSecDa);/* 写1Byte数据*/
pSecDa++;
}
T_CLK = 1;
T_RST =0;
}
/********************************************************************
*
* 名称: v_BurstR1302T
* 说明: 先写地址，后读命令/数据(时钟多字节方式)
* 功能: 读取DS1302时钟数据
* 调用: v_RTInputByte() , uc_RTOutputByte()
* 输入: pSecDa: 时钟数据地址 格式为: 秒 分 时 日 月 星期 年
* 7Byte (BCD码) 1B 1B 1B 1B 1B 1B 1B
* 返回值: ucDa :读取的数据
***********************************************************************/
void v_BurstR1302T(uchar *pSecDa)
{
uchar i;
T_RST = 0;
T_CLK = 0;
T_RST = 1;
v_RTInputByte(0xbf); /* 0xbf:时钟多字节读命令 */
for (i=8; i>0; i--)
{
*pSecDa = uc_RTOutputByte(); /* 读1Byte数据 */
pSecDa++;
}
T_CLK = 1;
T_RST =0;
}
/********************************************************************
*
* 名称: v_BurstW1302R
* 说明: 先写地址，后写数据(寄存器多字节方式)
* 功能: 往DS1302寄存器数写入数据(多字节方式)
* 调用: v_RTInputByte()
* 输入: pReDa: 寄存器数据地址
* 返回值: 无
***********************************************************************/
void v_BurstW1302R(uchar *pReDa)
{
uchar i;
v_W1302(0x8e,0x00); /* 控制命令,WP=0,写操作?*/
T_RST = 0;
T_CLK = 0;
T_RST = 1;
v_RTInputByte(0xfe); /* 0xbe:时钟多字节写命令 */
for (i=31;i>0;i--) /*31Byte 寄存器数据 */
{
v_RTInputByte(*pReDa); /* 写1Byte数据*/
pReDa++;
}
T_CLK = 1;
T_RST =0;
}
/********************************************************************
*
* 名称: uc_BurstR1302R
* 说明: 先写地址，后读命令/数据(寄存器多字节方式)
* 功能: 读取DS1302寄存器数据
* 调用: v_RTInputByte() , uc_RTOutputByte()
* 输入: pReDa: 寄存器数据地址
* 返回值: 无
***********************************************************************/
void v_BurstR1302R(uchar *pReDa)
{
uchar i;
T_RST = 0;
T_CLK = 0;
T_RST = 1;
v_RTInputByte(0xff); /* 0xbf:时钟多字节读命令 */
for (i=31; i>0; i--) /*31Byte 寄存器数据 */
{
*pReDa = uc_RTOutputByte(); /* 读1Byte数据 */
pReDa++;
}
T_CLK = 1;
T_RST =0;
}
/**************	/