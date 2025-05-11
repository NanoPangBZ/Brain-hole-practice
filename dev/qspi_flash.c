#include "qspi_flash.h"
#include "stm32h7xx_hal.h"
#include "quadspi.h"

static int8_t QSPI_W25Qxx_WriteEnable(void)
{
	QSPI_CommandTypeDef     s_command;	   // QSPI传输配置
	QSPI_AutoPollingTypeDef s_config;		// 轮询比较相关配置参数

	s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;    	// 1线指令模式
	s_command.AddressMode 			= QSPI_ADDRESS_NONE;   		      // 无地址模式
	s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;  	// 无交替字节 
	s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;      	// 禁止DDR模式
	s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;  	// DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;		// 每次传输数据都发送指令	
	s_command.DataMode 				= QSPI_DATA_NONE;       	      // 无数据模式
	s_command.DummyCycles 			= 0;                   	         // 空周期个数
	s_command.Instruction	 		= W25Qxx_CMD_WriteEnable;      	// 发送写使能命令

	// 发送写使能命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
		return QSPI_FLASH_WRITE_ENABLE_ERR;	//
	}
	
// 不停的查询 W25Qxx_CMD_ReadStatus_REG1 寄存器，将读取到的状态字节中的 W25Qxx_Status_REG1_WEL 不停的与 0x02 作比较
// 读状态寄存器1的第1位（只读），WEL写使能标志位，该标志位为1时，代表可以进行写操作
	
	s_config.Match           = 0x02;  								// 匹配值
	s_config.Mask            = W25Qxx_Status_REG1_WEL;	 		// 读状态寄存器1的第1位（只读），WEL写使能标志位，该标志位为1时，代表可以进行写操作
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;			 	// 与运算
	s_config.StatusBytesSize = 1;									 	// 状态字节数
	s_config.Interval        = 0x10;							 		// 轮询间隔
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;	// 自动停止模式

	s_command.Instruction    = W25Qxx_CMD_ReadStatus_REG1;	// 读状态信息寄存器
	s_command.DataMode       = QSPI_DATA_1_LINE;					// 1线数据模式
	s_command.NbData         = 1;										// 数据长度

	// 发送轮询等待命令	
	if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_FLASH_TIMEOUT; 	// 轮询等待无响应
	}	
	return QSPI_FLASH_OK;  // 通信正常结束
}

static int8_t QSPI_W25Qxx_SectorErase(uint32_t SectorAddress)
{
	QSPI_CommandTypeDef s_command;	// QSPI传输配置
	
	s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;    // 1线指令模式
	s_command.AddressSize       	= QSPI_ADDRESS_24_BITS;       // 24位地址模式
	s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;  //	无交替字节 
	s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;      // 禁止DDR模式
	s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;  // DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;	// 每次传输数据都发送指令
	s_command.AddressMode 			= QSPI_ADDRESS_1_LINE;        // 1线地址模式
	s_command.DataMode 				= QSPI_DATA_NONE;             // 无数据
	s_command.DummyCycles 			= 0;                          // 空周期个数
	s_command.Address           	= SectorAddress;              // 要擦除的地址
	s_command.Instruction	 		= W25Qxx_CMD_SectorErase;     // 扇区擦除命令

	// 发送写使能
	if (QSPI_W25Qxx_WriteEnable() != QSPI_FLASH_OK)
	{
		return QSPI_FLASH_WRITE_ENABLE_ERR;		// 写使能失败
	}
	// 发出擦除命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_FLASH_ERASE_ERR;		// 擦除失败
	}
	// 使用自动轮询标志位，等待擦除的结束 
	if (qspi_flash_wait_busy() != QSPI_FLASH_OK)
	{
		return QSPI_FLASH_TIMEOUT;		// 轮询等待无响应
	}
	return QSPI_FLASH_OK; // 擦除成功
}

static int8_t QSPI_W25Qxx_BlockErase_32K (uint32_t SectorAddress)
{
	QSPI_CommandTypeDef s_command;	// QSPI传输配置
	
	s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;    // 1线指令模式
	s_command.AddressSize       	= QSPI_ADDRESS_24_BITS;       // 24位地址模式
	s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;  //	无交替字节 
	s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;      // 禁止DDR模式
	s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;  // DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;	// 每次传输数据都发送指令
	s_command.AddressMode 			= QSPI_ADDRESS_1_LINE;        // 1线地址模式
	s_command.DataMode 				= QSPI_DATA_NONE;             // 无数据
	s_command.DummyCycles 			= 0;                          // 空周期个数
	s_command.Address           	= SectorAddress;              // 要擦除的地址
	s_command.Instruction	 		= W25Qxx_CMD_BlockErase_32K;  // 块擦除命令，每次擦除32K字节

	// 发送写使能	
	if (QSPI_W25Qxx_WriteEnable() != QSPI_FLASH_OK)
	{
		return QSPI_FLASH_WRITE_ENABLE_ERR;		// 写使能失败
	}
	// 发出擦除命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_FLASH_ERASE_ERR;				// 擦除失败
	}
	// 使用自动轮询标志位，等待擦除的结束 
	if (qspi_flash_wait_busy() != QSPI_FLASH_OK)
	{
		return QSPI_FLASH_TIMEOUT;		// 轮询等待无响应
	}
	return QSPI_FLASH_OK;	// 擦除成功
}

static int8_t QSPI_W25Qxx_Write_Page(uint32_t addr , uint8_t* data, uint32_t len)
{
	if( len > 256 )
	{
		return QSPI_FLASH_ERR;	// 写入数据长度超过256字节
	}

	QSPI_CommandTypeDef s_command;	// QSPI传输配置	
	
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    			// 1线指令模式
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;           		// 24位地址
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  			// 无交替字节 
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     			// 禁止DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 			// DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;				// 每次传输数据都发送指令	
	s_command.AddressMode 		 = QSPI_ADDRESS_1_LINE; 				// 1线地址模式
	s_command.DataMode    		 = QSPI_DATA_4_LINES;    				// 4线数据模式
	s_command.DummyCycles 		 = 0;                    				// 空周期个数
	s_command.NbData      		 = len;      							// 数据长度，最大只能256字节
	s_command.Address     		 = addr;         						// 要写入 W25Qxx 的地址
	s_command.Instruction 		 = W25Qxx_CMD_QuadInputPageProgram; 	// 1-1-4模式下(1线指令1线地址4线数据)，页编程指令
	
	// 写使能
	if (QSPI_W25Qxx_WriteEnable() != QSPI_FLASH_OK)
	{
		return QSPI_FLASH_WRITE_ENABLE_ERR;	// 写使能失败
	}
	// 写命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_FLASH_TRANSMIT_ERR;		// 传输数据错误
	}
	// 开始传输数据
	if (HAL_QSPI_Transmit(&hqspi, data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_FLASH_TRANSMIT_ERR;		// 传输数据错误
	}
	// 使用自动轮询标志位，等待写入的结束 
	if (qspi_flash_wait_busy() != QSPI_FLASH_OK)
	{
		return QSPI_FLASH_TIMEOUT; // 轮询等待无响应
	}
	return QSPI_FLASH_OK;	// 写数据成功
}

static int8_t QSPI_W25Qxx_BlockErase_64K (uint32_t SectorAddress)	
{
	QSPI_CommandTypeDef s_command;	// QSPI传输配置
	
	s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;    // 1线指令模式
	s_command.AddressSize       	= QSPI_ADDRESS_24_BITS;       // 24位地址模式
	s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;  //	无交替字节 
	s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;      // 禁止DDR模式
	s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;  // DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;	// 每次传输数据都发送指令
	s_command.AddressMode 			= QSPI_ADDRESS_1_LINE;        // 1线地址模式
	s_command.DataMode 				= QSPI_DATA_NONE;             // 无数据
	s_command.DummyCycles 			= 0;                          // 空周期个数
	s_command.Address           	= SectorAddress;              // 要擦除的地址
	s_command.Instruction	 		= W25Qxx_CMD_BlockErase_64K;  // 块擦除命令，每次擦除64K字节	

	// 发送写使能
	if (QSPI_W25Qxx_WriteEnable() != QSPI_FLASH_OK)
	{
		return QSPI_FLASH_WRITE_ENABLE_ERR;	// 写使能失败
	}
	// 发出擦除命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_FLASH_ERASE_ERR;			// 擦除失败
	}
	// 使用自动轮询标志位，等待擦除的结束 
	if (qspi_flash_wait_busy() != QSPI_FLASH_OK)
	{
		return QSPI_FLASH_TIMEOUT;	// 轮询等待无响应
	}
	return QSPI_FLASH_OK;		// 擦除成功
}

qspi_flash_err_t QSPI_W25Qxx_Erase(uint32_t addr,uint32_t len)
{
	if( addr % len != 0 )	// 地址对齐
	{
		return QSPI_FLASH_ERASE_ERR;	// 擦除地址不对齐
	}

	if ( len == 0x10000 )	// 擦除64K字节
	{
		return QSPI_W25Qxx_BlockErase_64K(addr);	// 擦除64K字节
	}
	else if (len == 0x8000)	// 擦除32K字节
	{
		return QSPI_W25Qxx_BlockErase_32K(addr);	// 擦除32K字节
	}
	else if (len == 0x1000)	// 擦除4K字节
	{
		return QSPI_W25Qxx_SectorErase(addr);	// 擦除4K字节
	}
	else
	{
		return QSPI_FLASH_ERASE_ERR;				// 擦除失败
	}
}

/**
 * @brief qspi flash初始化
*/
qspi_flash_err_t qspi_flash_init(void)
{
    uint32_t	device_id;	// 器件ID

    qpsi_flash_reset();
    device_id = qspi_flash_read_id();	// 读取器件ID

    if( device_id == W25Qxx_FLASH_ID )		// 进行匹配
	{
		return QSPI_FLASH_OK;			// 返回成功标志		
	}
	else
	{
		return QSPI_FLASH_ERR;		// 返回错误标志
	}
}

qspi_flash_err_t qpsi_flash_reset(void)
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
		return QSPI_FLASH_TRANSMIT_ERR;			// 如果发送失败，返回错误信息
	}
	// 使用自动轮询标志位，等待通信结束
	if (qspi_flash_wait_busy() != QSPI_FLASH_OK)
	{
		return QSPI_FLASH_TIMEOUT;	// 轮询等待无响应
	}

	s_command.Instruction  = W25Qxx_CMD_ResetDevice;     // 复位器件命令    

	//发送复位器件命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
		return QSPI_FLASH_TRANSMIT_ERR;		  // 如果发送失败，返回错误信息
	}
	// 使用自动轮询标志位，等待通信结束
	if (qspi_flash_wait_busy() != QSPI_FLASH_OK)
	{
		return QSPI_FLASH_TIMEOUT;	// 轮询等待无响应
	}	
	return QSPI_FLASH_OK;	// 复位成功
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
		return QSPI_FLASH_TRANSMIT_ERR;		// 如果发送失败，返回错误信息
	}
	// 接收数据
	if (HAL_QSPI_Receive(&hqspi, QSPI_ReceiveBuff, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
		return QSPI_FLASH_TRANSMIT_ERR;  // 如果接收失败，返回错误信息
	}
	// 将得到的数据组合成ID
	W25Qxx_ID = (QSPI_ReceiveBuff[0] << 16) | (QSPI_ReceiveBuff[1] << 8 ) | QSPI_ReceiveBuff[2];

	return W25Qxx_ID; // 返回ID
}

qspi_flash_err_t qspi_flash_check(void)
{
	uint32_t	device_id;	// 器件ID
	device_id = qspi_flash_read_id();	// 读取器件ID

    if( device_id == W25Qxx_FLASH_ID )		// 进行匹配
	{
		return QSPI_FLASH_OK;			// 返回成功标志		
	}
	else
	{
		return QSPI_FLASH_ERR;		// 返回错误标志
	}
}

qspi_flash_err_t qspi_flash_wait_busy(void)
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
		return QSPI_FLASH_TIMEOUT; // 轮询等待无响应
	}
	return QSPI_FLASH_OK; // 通信正常结束

}

qspi_flash_err_t qspi_flash_enter_mem_map_mode(void)
{

}

qspi_flash_err_t qspi_flash_exit_mem_map_mode(void)
{

}

qspi_flash_err_t qspi_flash_erase_chip(void)
{
	QSPI_CommandTypeDef s_command;		// QSPI传输配置
	QSPI_AutoPollingTypeDef s_config;	// 轮询等待配置参数

	s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;    // 1线指令模式
	s_command.AddressSize       	= QSPI_ADDRESS_24_BITS;       // 24位地址模式
	s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;  //	无交替字节 
	s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;      // 禁止DDR模式
	s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;  // DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;	// 每次传输数据都发送指令
	s_command.AddressMode 			= QSPI_ADDRESS_NONE;       	// 无地址
	s_command.DataMode 				= QSPI_DATA_NONE;             // 无数据
	s_command.DummyCycles 			= 0;                          // 空周期个数
	s_command.Instruction	 		= W25Qxx_CMD_ChipErase;       // 擦除命令，进行整片擦除

	// 发送写使能	
	if (QSPI_W25Qxx_WriteEnable() != QSPI_FLASH_OK)
	{
		return QSPI_FLASH_WRITE_ENABLE_ERR;	// 写使能失败
	}
	// 发出擦除命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_FLASH_ERASE_ERR;		 // 擦除失败
	}

// 不停的查询 W25Qxx_CMD_ReadStatus_REG1 寄存器，将读取到的状态字节中的 W25Qxx_Status_REG1_BUSY 不停的与0作比较
// 读状态寄存器1的第0位（只读），Busy标志位，当正在擦除/写入数据/写命令时会被置1，空闲或通信结束为0
	
	s_config.Match           = 0;   							//	匹配值
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;	      		//	与运算
	s_config.Interval        = 0x10;	                     	//	轮询间隔
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;		// 自动停止模式
	s_config.StatusBytesSize = 1;	                        	//	状态字节数
	s_config.Mask            = W25Qxx_Status_REG1_BUSY;	   		// 对在轮询模式下接收的状态字节进行屏蔽，只比较需要用到的位
	
	s_command.Instruction    = W25Qxx_CMD_ReadStatus_REG1;		// 读状态信息寄存器
	s_command.DataMode       = QSPI_DATA_1_LINE;				// 1线数据模式
	s_command.NbData         = 1;								// 数据长度

	// W25Q64整片擦除的典型参考时间为20s，最大时间为100s，这里的超时等待值 W25Qxx_ChipErase_TIMEOUT_MAX 为 100S
	if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, W25Qxx_ChipErase_TIMEOUT_MAX) != HAL_OK)
	{
		return QSPI_FLASH_TIMEOUT;	 // 轮询等待无响应
	}
	return QSPI_FLASH_OK;
}

qspi_flash_err_t qspi_flash_erase(uint32_t addr,uint32_t len)
{
	return QSPI_W25Qxx_Erase(addr,len);
}

/**
 * @brief qspi flash烧写
 * @param addr 烧写地址
 * @param data_buf 烧写数据
 * @param len 烧写长度
 * @return 0:成功 其他:失败
 * @note 这个api直接对flash页编程，不进行"读取-缓存-擦除-写入"流程
*/
qspi_flash_err_t qspi_flash_programe(uint32_t addr , uint8_t* data_buf , uint32_t len)
{
	uint32_t curr_addr = addr;
	uint32_t end_addr = addr + len;
	uint32_t need_write_len = len;
	qspi_flash_err_t err = QSPI_FLASH_OK;

	while( curr_addr != end_addr )
	{
		uint32_t that_write_len = need_write_len > 256 ? 256 : need_write_len;
		err = QSPI_W25Qxx_Write_Page( curr_addr , data_buf , that_write_len );
		curr_addr += that_write_len;
		if( err != QSPI_FLASH_OK )
		{
			return err;
		}
	}

	return err;
}

/**
 * @brief qspi flash数据读取
 * @param addr 读取地址
 * @param data_buf 读取数据缓存
 * @param len 读取长度
 * @return 非负:读取到的数据长度 负数:失败
*/
qspi_flash_err_t qspi_flash_read(uint32_t addr , uint8_t* data_buf , uint32_t len)
{
	QSPI_CommandTypeDef s_command;	// QSPI传输配置
	
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    		// 1线指令模式
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;            // 24位地址
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  		// 无交替字节 
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     		// 禁止DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 		// DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;			// 每次传输数据都发送指令	
	s_command.AddressMode 		 = QSPI_ADDRESS_4_LINES; 				// 4线地址模式
	s_command.DataMode    		 = QSPI_DATA_4_LINES;    				// 4线数据模式
	s_command.DummyCycles 		 = 6;                    				// 空周期个数
	s_command.NbData      		 = len;      			   	// 数据长度，最大不能超过flash芯片的大小
	s_command.Address     		 = addr;         					// 要读取 W25Qxx 的地址
	s_command.Instruction 		 = W25Qxx_CMD_FastReadQuad_IO; 		// 1-4-4模式下(1线指令4线地址4线数据)，快速读取指令
	
	// 发送读取命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_FLASH_TRANSMIT_ERR;		// 传输数据错误
	}

	//	接收数据
	
	if (HAL_QSPI_Receive(&hqspi, data_buf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_FLASH_TRANSMIT_ERR;		// 传输数据错误
	}

	// 使用自动轮询标志位，等待接收的结束 
	if (qspi_flash_wait_busy() != QSPI_FLASH_OK)
	{
		return QSPI_FLASH_TIMEOUT; // 轮询等待无响应
	}
	return QSPI_FLASH_OK;	// 读取数据成功
}

/**
 * @brief qspi flash数据写入
 * @param addr 写入地址
 * @param data_buf 要写入的数据的缓存
 * @param len 写入的长度
 * @return 非负:写入的数据长度 负数:失败
 * @note 这个api会对写入覆盖到的页进行缓存、擦除和编程，即不影响相关页的其他数据
*/
qspi_flash_err_t qspi_flash_write(uint32_t addr , uint8_t* data_buf , uint32_t len)
{

}
