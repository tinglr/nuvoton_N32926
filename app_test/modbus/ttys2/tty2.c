#include<stdio.h>
#include<stdlib.h>
#include<modbus.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define SERVER_ID         17

//03和10是读写保持寄存器 holding register  0x100-0x130
const uint16_t UT_REGISTERS_TAB[49] = {
0x0100,0x0101,0x0102,0x0103,0x0104,0x0105,0x0106,0x0107,0x0108,0x0109,0x010A,0x010B,0x010C,0x010D,0x010E,0x010F,
0x0110,0x0111,0x0112,0x0113,0x0114,0x0115,0x0116,0x0117,0x0118,0x0119,0x011A,0x011B,0x011C,0x011D,0x011E,0x011F,
0x0120,0x0121,0x0122,0x0123,0x0124,0x0125,0x0126,0x0127,0x0128,0x0129,0x012A,0x012B,0x012C,0x012D,0x012E,0x012F,0x0130
};

int main(int argc, char*argv[])
{
int i;
     modbus_t *ctx;
     modbus_mapping_t *mb_mapping;

     ctx = modbus_new_rtu("/dev/ttyS1",19200,'N',8,1);//创建一个rtu类型的容器
     modbus_set_slave(ctx, SERVER_ID);//设置从机地址

     if (modbus_connect(ctx) == -1) {
           // fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
            modbus_free(ctx);
            return -1;              }

  //  modbus_set_debug(ctx, TRUE);

    mb_mapping = modbus_mapping_new(0,0, 0x100+49,0);//保持寄存器
        if (mb_mapping == NULL) {
       // fprintf(stderr, "Failed to allocate the mapping: %s\n",modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    for (i=0;i < 49; i++) {
        mb_mapping->tab_registers[0x0100+i] =UT_REGISTERS_TAB[i];
    }

  //  mb_mapping->start_registers=0x100;

    for (;;) 
	{
	unsigned char query[MODBUS_RTU_MAX_ADU_LENGTH];
	int rc;
        rc = modbus_receive(ctx, query);
        if (rc >= 0) {
         	 	modbus_reply(ctx, query, rc, mb_mapping);
                     }
	else { //printf("Unable to connect \n")
			; }
	}

    modbus_mapping_free(mb_mapping);
    modbus_free(ctx);

    return 0;
}
