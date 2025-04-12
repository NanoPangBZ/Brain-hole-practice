#include "qspi_flash.h"
#include "stm32h7xx_hal.h"
#include "quadspi.h"

int8_t qspi_flash_init(void)
{
    uint32_t	device_id;	// 器件ID

    qpsi_flash_reset();
    device_id = qspi_flash_read_id();	// 读取器件ID

    if( device_id == W25Qxx_FLASH_ID )		// 进行匹配
	{
		return QSPI_W25Qxx_OK;			// 返回成功标志		
	}
	else
	{
		return W25Qxx_ERROR_INIT;		// 返回错误标志
	}
}

int8_t qpsi_flash_reset(void)
{
    QSPI_CommandTypeDef s_command;	// QSPI传输配置

	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;   	// 1线指令模式
	s_command.AddressMode 		 = QSPI_ADDRESS_NONE;   		// 无地址模式
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; 	// 无交替字节 
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     	// 禁止DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 	// DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 	// 每次传输数据都发送指令
	s_command.DataMode 			 = QSPI_DATA_NONE;       		// 无数据模式	
	s_command.DummyCycles 		 = 0;                     		// 空周期个数
	s_command.Instruction 		 = W25Qxx_CMD_EnableReset;      // 执行复位使能命令

	// 发送复位使能命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
		return W25Qxx_ERROR_INIT;			// 如果发送失败，返回错误信息
	}
	// 使用自动轮询标志位，等待通信结束
	if (qspi_flash_auto_polling_mem_ready() != QSPI_W25Qxx_OK)
	{
		return W25Qxx_ERROR_AUTOPOLLING;	// 轮询等待无响应
	}

	s_command.Instruction  = W25Qxx_CMD_ResetDevice;     // 复位器件命令    

	//发送复位器件命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
		return W25Qxx_ERROR_INIT;		  // 如果发送失败，返回错误信息
	}
	// 使用自动轮询标志位，等待通信结束
	if (qspi_flash_auto_polling_mem_ready() != QSPI_W25Qxx_OK)
	{
		return W25Qxx_ERROR_AUTOPOLLING;	// 轮询等待无响应
	}	
	return QSPI_W25Qxx_OK;	// 复位成功
}

uint32_t qspi_flash_read_id(void)
{
	QSPI_CommandTypeDef s_command;	    // QSPI传输配置
	uint8_t	QSPI_ReceiveBuff[3];		// 存储QSPI读到的数据
	uint32_t	W25Qxx_ID;			    // 器件的ID

	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;      // 1线指令模式
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;         // 24位地址
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;    // 无交替字节 
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;        // 禁止DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;    // DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	    // 每次传输数据都发送指令
	s_command.AddressMode		= QSPI_ADDRESS_NONE;            // 无地址模式
	s_command.DataMode			= QSPI_DATA_1_LINE;       	    // 1线数据模式
	s_command.DummyCycles 		= 0;                            // 空周期个数
	s_command.NbData 			= 3;                            // 传输数据的长度
	s_command.Instruction 		= W25Qxx_CMD_JedecID;           // 执行读器件ID命令

	// 发送指令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
		return W25Qxx_ERROR_INIT;		// 如果发送失败，返回错误信息
	}
	// 接收数据
	if (HAL_QSPI_Receive(&hqspi, QSPI_ReceiveBuff, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
		return W25Qxx_ERROR_TRANSMIT;  // 如果接收失败，返回错误信息
	}
	// 将得到的数据组合成ID
	W25Qxx_ID = (QSPI_ReceiveBuff[0] << 16) | (QSPI_ReceiveBuff[1] << 8 ) | QSPI_ReceiveBuff[2];

	return W25Qxx_ID; // 返回ID
}

int8_t qspi_flash_auto_polling_mem_ready(void)
{
	QSPI_CommandTypeDef     s_command;	   // QSPI传输配置
	QSPI_AutoPollingTypeDef s_config;		// 轮询比较相关配置参数

	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;			// 1线指令模式
	s_command.AddressMode       = QSPI_ADDRESS_NONE;				// 无地址模式
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;		//	无交替字节 
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;	     	// 禁止DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;	   	// DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	   	    //	每次传输数据都发送指令	
	s_command.DataMode          = QSPI_DATA_1_LINE;					// 1线数据模式
	s_command.DummyCycles       = 0;								//	空周期个数
	s_command.Instruction       = W25Qxx_CMD_ReadStatus_REG1;	    // 读状态信息寄存器
																					
    // 不停的查询 W25Qxx_CMD_ReadStatus_REG1 寄存器，将读取到的状态字节中的 W25Qxx_Status_REG1_BUSY 不停的与0作比较
    // 读状态寄存器1的第0位（只读），Busy标志位，当正在擦除/写入数据/写命令时会被置1，空闲或通信结束为0
	
	s_config.Match           = 0;   					    //	匹配值
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;	      	//	与运算
	s_config.Interval        = 0x10;	                    //	轮询间隔
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;	// 自动停止模式
	s_config.StatusBytesSize = 1;	                        //	状态字节数
	s_config.Mask            = W25Qxx_Status_REG1_BUSY;     // 对在轮询模式下接收的状态字节进行屏蔽，只比较需要用到的位
		
	// 发送轮询等待命令
	if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return W25Qxx_ERROR_AUTOPOLLING; // 轮询等待无响应
	}
	return QSPI_W25Qxx_OK; // 通信正常结束

}
