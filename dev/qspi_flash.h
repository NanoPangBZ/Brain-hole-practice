#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif  //__cplusplus

#define QSPI_FLASH_ERR_TYPE             int8_t
#define QSPI_FLASH_OK                   0
#define QSPI_FLASH_ERR                  -1
#define QSPI_FLASH_TIMEOUT              -2
#define QSPI_FLASH_TRANSMIT_ERR         -3
#define QSPI_FLASH_WRITE_ENABLE_ERR     -4
#define QSPI_FLASH_ERASE_ERR            -5

#define W25Qxx_CMD_EnableReset  	0x66		// 使能复位
#define W25Qxx_CMD_ResetDevice   	0x99		// 复位器件
#define W25Qxx_CMD_JedecID 			0x9F		// JEDEC ID  
#define W25Qxx_CMD_WriteEnable		0X06		// 写使能

#define W25Qxx_CMD_SectorErase 		0x20		// 扇区擦除，4K字节， 参考擦除时间 45ms
#define W25Qxx_CMD_BlockErase_32K 	0x52		// 块擦除，  32K字节，参考擦除时间 120ms
#define W25Qxx_CMD_BlockErase_64K 	0xD8		// 块擦除，  64K字节，参考擦除时间 150ms
#define W25Qxx_CMD_ChipErase 		0xC7		// 整片擦除，参考擦除时间 20S

#define W25Qxx_CMD_QuadInputPageProgram  	0x32  		// 1-1-4模式下(1线指令1线地址4线数据)，页编程指令，参考写入时间 0.4ms 
#define W25Qxx_CMD_FastReadQuad_IO       	0xEB  		// 1-4-4模式下(1线指令4线地址4线数据)，快速读取指令

#define W25Qxx_CMD_ReadStatus_REG1			0X05		// 读状态寄存器1
#define W25Qxx_Status_REG1_BUSY  			0x01		// 读状态寄存器1的第0位（只读），Busy标志位，当正在擦除/写入数据/写命令时会被置1
#define W25Qxx_Status_REG1_WEL  			0x02		// 读状态寄存器1的第1位（只读），WEL写使能标志位，该标志位为1时，代表可以进行写操作

#define W25Qxx_PageSize       				256			// 页大小，256字节
#define W25Qxx_FlashSize       				0x800000	// W25Q64大小，8M字节
#define W25Qxx_FLASH_ID           			0Xef4017    // W25Q64 JEDEC ID
#define W25Qxx_ChipErase_TIMEOUT_MAX		100000U		// 超时等待时间，W25Q64整片擦除所需最大时间是100S
#define W25Qxx_Mem_Addr						0x90000000 	// 内存映射模式的地址

#ifndef QSPI_FLASH_ERR_TYPE
    #define QSPI_FLASH_ERR_TYPE int
#endif  //QSPI_FLASH_ERR_TYPE

typedef QSPI_FLASH_ERR_TYPE qspi_flash_err_t;

qspi_flash_err_t qspi_flash_init(void);
qspi_flash_err_t qpsi_flash_reset(void);
uint32_t qspi_flash_read_id(void);
qspi_flash_err_t qspi_flash_check(void);

qspi_flash_err_t qspi_flash_auto_polling_mem_ready(void);
qspi_flash_err_t qspi_flash_enter_mem_map_mode(void);
qspi_flash_err_t qspi_flash_exit_mem_map_mode(void);

qspi_flash_err_t qspi_flash_erase_chip(void);
qspi_flash_err_t qspi_flash_erase(uint32_t addr,uint32_t len);
qspi_flash_err_t qspi_flash_programe(uint32_t addr , uint8_t* data_buf , uint32_t len);

qspi_flash_err_t qspi_flash_read(uint32_t addr , uint8_t* data_buf , uint32_t len);
qspi_flash_err_t qspi_flash_write(uint32_t addr , uint8_t* data_buf , uint32_t len);


#ifdef __cplusplus
}
#endif  //__cplusplus
