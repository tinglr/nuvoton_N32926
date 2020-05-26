

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define RESSUM	40950000	
unsigned int NTC[50]={24623,23643,22707,21814,20961,20146,19367,18623,17911,17231,
	    16580,15958,15362,14792,14246,13724,13223,12744,12284,11844,
			11422,11017,10629,10257,9900,9550,9215,8893,8584,8288,
			8004,7730,7468,7216,6974,6741,6518,6303,6096,5897,
			5706,5522,5345,5174,5010,4852,4700,4554,4412,4276};

int main(int argc,char **argv)
{
	
	int fd,u32_i,RES;
        unsigned char key_vals[4]={0};
        int ret,ntc1,ntc2,res1,res2,temp1,temp2;


        fd = open("/dev/ntc", O_RDWR);
	if (fd <0)printf("Cannot open /dev/ntc\n");
	ret = read(fd, key_vals, sizeof(key_vals));
        if (ret <0)printf("Cannot read /dev/ntc  ret = %d \n",ret);

         ntc1 = ((int)key_vals[0] << 8) | (int)(key_vals[1]);
         ntc2 = ((int)key_vals[2] << 8) | (int)(key_vals[3]);

	if(ntc1==0){printf("ntc1 Voltage Detect Value  = %d\n", ntc1);}	
	res1=(RESSUM/ntc1)-10000;
	for(u32_i=0;u32_i<50;u32_i++)		
	{if(res1>NTC[u32_i])  break;}	
	temp1=u32_i;	

        if(ntc2==0){printf("ntc2 Voltage Detect Value  = %d\n", ntc2);}	
	res2=(RESSUM/ntc2)-10000;
	for(u32_i=0;u32_i<50;u32_i++)		
	{if(res2>NTC[u32_i])  break;}	
	temp2=u32_i;
															
       printf("res1= %d, res2= %d, temp1= %d, temp2= %d \n",res1,res2,temp1,temp2);

       printf("vals :  %d, %d, %d, %d    ret=%d  ntc1=%d  ntc2=%d\n", key_vals[0],key_vals[1],key_vals[2],key_vals[3],ret,ntc1,ntc2);


   return 0;
}
