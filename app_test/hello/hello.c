

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc,char **argv)
{
	

	printf("hello1_exit\n");
        sleep(1);
	printf("hello2_exit\n");
	//exit(0);//处理缓存后关闭程序
	//_exit(0); //立即结束
        sleep(1);

	printf("hello3_exit\n");

        sleep(1);
while(1){;}
return 0;
	
}
