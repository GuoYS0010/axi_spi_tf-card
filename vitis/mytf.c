#include "mytf.h"
static void delay(){
	for (int i=0; i < 500000; i++){}
}
uint32_t tf_cmd(uint32_t cmd, uint32_t arg){
	Xil_Out32(SPI_BASE_ADDR + 0x200, arg);
	Xil_Out32(SPI_BASE_ADDR + 0x204, cmd);
	Xil_Out32(SPI_BASE_ADDR + 0x208, 1);
	int a = Xil_In32(SPI_BASE_ADDR + 0x208);
	while (a == 1) a = Xil_In32(SPI_BASE_ADDR + 0x208);
	printf("CMD%d is done\n\r", cmd);
	printf("r1:%x\n\r32:%x\n\r", Xil_In32(SPI_BASE_ADDR + 0x20c), Xil_In32(SPI_BASE_ADDR + 0x210));
	return Xil_In32(SPI_BASE_ADDR + 0x20c);
}

void tf_reset(){
	int a = tf_cmd(0, 0);
	while (a != 1){
		delay();
		tf_cmd(1, 0);
		a = tf_cmd(0, 0);
	}
	print("\033[1;31;40mtf reset is done!!\n\r\033[0m");
}

void tf_init(){
	tf_cmd(8, 0x1aa);
	tf_cmd(55, 0);
	int a = tf_cmd(41, 0x40000000);
	while (a != 0){
		delay();
		tf_cmd(55, 0);
		a = tf_cmd(41, 0x40000000);
	}
	print("\033[1;31;40mtf init is done!!\n\r\033[0m");
}

void tf_block_read(uint32_t block_num){
	tf_cmd(17, block_num);
	printf("\033[1;31;40mdata in this block:\n\r");
	for (int i=0; i < 128; i++){
		int tmpout = Xil_In32(SPI_BASE_ADDR + i * 4);
		printf("%04x", tmpout);
	}
	printf("\n\r\033[0m");


}
