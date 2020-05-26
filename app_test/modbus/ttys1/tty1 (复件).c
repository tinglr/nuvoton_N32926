#include<stdio.h>
#include<stdlib.h>
#include"modbus.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#define SERVER_ID         17

int main(int argc, char*argv[])
{
     modbus_t *ctx;
     modbus_mapping_t *mb_mapping;

     ctx = modbus_new_rtu("/dev/ttyS1",19200,'N',8,1);//创建一个rtu类型的容器
     modbus_set_slave(ctx, SERVER_ID);//设置从机地址

     if (modbus_connect(ctx) == -1) {
            fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
            modbus_free(ctx);
            return -1;              }

   // modbus_set_debug(ctx, TRUE);

    mb_mapping = modbus_mapping_new(500，500，500，500);//0x08+0x1  
    header_length = modbus_get_header_length(ctx);

    for (;;) 
	{
	unsigned char query[MODBUS_RTU_MAX_ADU_LENGTH];
	int rc;
        rc = modbus_receive(ctx, query);
        if (rc >= 0) {
         	 modbus_reply(ctx, query, rc, mb_mapping);
                     }
	else { printf("Unable to connect \n"); }
	}

    modbus_mapping_free(mb_mapping);
    modbus_free(ctx);

    return 0;
}
