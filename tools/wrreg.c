#include<stdio.h>  
#include<unistd.h>  
#include<sys/mman.h>
#include<sys/types.h>  
#include<sys/stat.h>  
#include<fcntl.h>
#include<stdlib.h>

int main(int argc, char* argv[])
{
	if(argc != 3) {
		printf("insufficient arguments\n");
		return -1;
	}

	int fd;
	unsigned char * map_base;
	unsigned long PHYADDR,newval;
	int  pagesize = getpagesize();

	PHYADDR = strtoul(argv[1],NULL,16);
	printf("PHYADDR = 0x%08X\n",PHYADDR);
	newval = strtoul(argv[2],NULL,16);

	fd = open("/dev/mem", O_RDWR|O_SYNC);
	if(fd == -1)  
	{  
		printf("open /dev/mem failed\n");
		return -1;  
	}

	map_base = mmap(NULL, pagesize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, PHYADDR & 0xfffff000);
	if(map_base == MAP_FAILED) {
		perror("mmap");
	}
	else
		printf("mmap = 0x%08X\n", map_base);

	int pgoffset = PHYADDR & 0x00000fff;
	unsigned int c0,c1,c2,c3;
	/* seperate 4 bytes of a 32-bit register value */
	c0 = newval & 0x000000ff;
	c1 = (newval & 0x0000ff00) / 0x100;
	c2 = (newval & 0x00ff0000) / 0x10000;
	c3 = (newval & 0xff000000) / 0x1000000;

	/* write into registers */
	map_base[pgoffset] = (unsigned char)c0;
	map_base[pgoffset + 1] = (unsigned char)c1;
	map_base[pgoffset + 2] = (unsigned char)c2;
	map_base[pgoffset + 3] = (unsigned char)c3;

	printf("\nValue at 0x%08X: 0x%02X%02X%02X%02X\n", PHYADDR,
					(unsigned int)map_base[pgoffset + 3],
					(unsigned int)map_base[pgoffset + 2],
					(unsigned int)map_base[pgoffset + 1],
					(unsigned int)map_base[pgoffset]
					);
	
	close(fd);
	munmap(map_base, pagesize);
	return 0;
}
