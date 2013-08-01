#include<stdio.h>  
#include<unistd.h>  
#include<sys/mman.h>
#include<sys/types.h>  
#include<sys/stat.h>  
#include<fcntl.h>
#include<stdlib.h>

int main(int argc, char* argv[])
{
	/* main() will get 2 extra arguments, argv[1] is the PHY address, argv[2] is the word length to read. */
	if(argc != 3) {
		printf("insufficient arguments\n");
		return -1;
	}

	int fd;
	int i, linenum, count=0;
	unsigned char * map_base;
	unsigned long PHYADDR, length;
	/* get pagesize on the specific device */
	int  pagesize = getpagesize();

	/* turn a strig into a unsigned long integer */
	PHYADDR = strtoul(argv[1],NULL,16);
	printf("PHYADDR = 0x%08x\n", PHYADDR);
	length = strtoul(argv[2],NULL,16);
	printf("length = %d words\n", length);

	/* open a descriptor to /dev/mem */
	fd = open("/dev/mem", O_RDWR|O_SYNC);
	if(fd == -1)  
	{  
		printf("open /dev/mem failed\n");
		return -1;  
	}

	/* mmap() maps the physical address into virtual address, including IO and hardware registers
	* the start address is NULL, so kernel will locate it at any possible place.
	* with read and write permission
	* the offset is aligned with a memory page
	*/
	map_base = mmap(NULL, pagesize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, PHYADDR & 0xfffff000);
	if(map_base == MAP_FAILED) {
		perror("mmap");
	}
	else
		printf("map_base = 0x%08X\n", map_base);
	printf("\n");

	/* get the within-page offset */
	int pgoffset = PHYADDR & 0x00000fff;
	
	/* print 4 registers in a line */
	while(length>4) {
		linenum=4;
		length-=4;
		printf("%08X: ", (PHYADDR + count));
		for(i=0;i<4*linenum;i=i+4) {
			printf("%02X%02X%02X%02X    ",  (unsigned int)map_base[pgoffset + count + i+3],
							(unsigned int)map_base[pgoffset + count + i+2],
							(unsigned int)map_base[pgoffset + count + i+1],
							(unsigned int)map_base[pgoffset + count + i]
							);
		}
		printf("\n");
		count+=16;
	}
	printf("%08X: ", (PHYADDR + count));
	for(i=0;i<4*length;i=i+4) {
		printf("%02X%02X%02X%02X    ",  (unsigned int)map_base[pgoffset + count + i+3],
						(unsigned int)map_base[pgoffset + count + i+2],
						(unsigned int)map_base[pgoffset + count + i+1],
						(unsigned int)map_base[pgoffset + count + i]
						);
	}
	printf("\n");


	/* close file descriptor */
	close(fd);
	/* release map */
	munmap(map_base, pagesize);
	return 0;
}
