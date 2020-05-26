#include<stdio.h>
#include<stdlib.h>
#include"modbus.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "unit-test.h"
#include <errno.h>


int main(int argc, char*argv[])
{
    int socket;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;
    int rc;
    int i;
    int use_backend;
    uint8_t *query;
    int header_length;


  //       use_backend = RTU;

        ctx = modbus_new_rtu("/dev/ttyS1",19200,'N',8,1);//open port
        modbus_set_slave(ctx, SERVER_ID);//设置从机地址
        query = malloc(MODBUS_RTU_MAX_ADU_LENGTH);
   
    header_length = modbus_get_header_length(ctx);

   // modbus_set_debug(ctx, TRUE);

    mb_mapping = modbus_mapping_new(
        UT_BITS_ADDRESS + UT_BITS_NB,//0x13+0x25
        UT_INPUT_BITS_ADDRESS + UT_INPUT_BITS_NB,//0xc4+0x16
        UT_REGISTERS_ADDRESS + UT_REGISTERS_NB,//0x6b+0x3
        UT_INPUT_REGISTERS_ADDRESS + UT_INPUT_REGISTERS_NB);//0x08+0x1
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Examples from PI_MODBUS_300.pdf.
       Only the read-only input values are assigned. */

    /** INPUT STATUS **/
    modbus_set_bits_from_bytes(mb_mapping->tab_input_bits,
                               UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB,
                               UT_INPUT_BITS_TAB);

    /** INPUT REGISTERS **/  //测试ok
    for (i=0; i < UT_INPUT_REGISTERS_NB; i++) {
        mb_mapping->tab_input_registers[UT_INPUT_REGISTERS_ADDRESS+i] =
            UT_INPUT_REGISTERS_TAB[i];;
    }


    /** REGISTERS **/       //测试ok
    for (i=0; i < UT_REGISTERS_NB; i++) {
        mb_mapping->tab_registers[UT_REGISTERS_ADDRESS+i] =
            UT_REGISTERS_TAB[i];;
    }

        rc = modbus_connect(ctx);
        if (rc == -1) {
            fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
            modbus_free(ctx);
            return -1;
        
    }

    for (;;) {
        rc = modbus_receive(ctx, query);
        if (rc == -1) {
            /*Connection closed by the client or error */
            break;
        }

        /* Read holding registers *///读保持寄存器
        if (query[header_length] == 0x03) { //如果头部是0x03
            if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 3)
                == UT_REGISTERS_NB_SPECIAL) {
          //      printf("Set an incorrect number of values\n"); //设置不正确的数值
                MODBUS_SET_INT16_TO_INT8(query, header_length + 3,
                                         UT_REGISTERS_NB_SPECIAL - 1);
            } else if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
                == UT_REGISTERS_ADDRESS_SPECIAL) {
           //     printf("Reply to this special register address by an exception\n");//对特别注册的地址额外的答复
                modbus_reply_exception(ctx, query,
                                       MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY);
                continue;
            }
        }

        rc = modbus_reply(ctx, query, rc, mb_mapping);
        if (rc == -1) {
            break;
        }
    }

    printf("Quit the loop: %s\n", modbus_strerror(errno));


    modbus_mapping_free(mb_mapping);
    free(query);
    modbus_free(ctx);

    return 0;
}
