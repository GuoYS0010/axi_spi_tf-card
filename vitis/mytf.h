
#include <stdio.h>
#include "xil_io.h"

#define SPI_BASE_ADDR 0xa0000000


uint32_t tf_cmd(uint32_t cmd, uint32_t arg);
void tf_reset();
void tf_init();

void tf_block_read(uint32_t block_num);
