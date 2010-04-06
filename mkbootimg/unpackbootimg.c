#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

#include "mincrypt/sha.h"
#include "bootimg.h"

typedef unsigned char byte;

int read_padding(FILE* f, unsigned itemsize)
{
    byte buf[2048];
    int pagesize = 2048;
    unsigned pagemask = pagesize - 1;
    unsigned count;

    if((itemsize & pagemask) == 0) {
        return 0;
    }

    count = pagesize - (itemsize & pagemask);

    if(fread(buf, count, 1, f)) {
        return -1;
    } else {
        return 0;
    }
}


int main(int argc, char** argv)
{
    if (argc != 2 && argc != 3)
    {
        printf("Usage:\n");
        printf("\tunpackbootimg <boot.img>\n");
        printf("\tunpackbootimg <boot.img> <output_directory>\n");
        return 0;
    }
    
    char* directory = "./";
    if (argc == 3)
    {
        directory = argv[2];
    }
    
    int total_read = 0;
    FILE* f = fopen(argv[1], "rb");
    boot_img_hdr header;

    //printf("Reading header...\n");
    fread(&header, sizeof(header), 1, f);
    printf("BOARD_KERNEL_CMDLINE %s\n", header.cmdline);
    printf("BOARD_KERNEL_BASE %x\n", header.kernel_addr - 0x00008000);
    
    total_read += sizeof(header);
    //printf("total read: %d\n", total_read);
    read_padding(f, sizeof(header));

    char* kernel_file[PATH_MAX];
    sprintf(kernel_file, "%s/%s", directory, basename(argv[1]));
    strcat(kernel_file, "-zImage");
    FILE *k = fopen(kernel_file, "wb");
    byte* kernel = (byte*)malloc(header.kernel_size);
    //printf("Reading kernel...\n");
    fread(kernel, header.kernel_size, 1, f);
    fwrite(kernel, header.kernel_size, 1, k);
    fclose(k);
    
    total_read += sizeof(header);
    //printf("total read: %d\n", header.kernel_size);
    read_padding(f, header.kernel_size);

    char* ramdisk_file[PATH_MAX];
    sprintf(ramdisk_file, "%s/%s", directory, basename(argv[1]));
    strcat(ramdisk_file, "-ramdisk.img");
    FILE *r = fopen(ramdisk_file, "wb");
    byte* ramdisk = (byte*)malloc(header.ramdisk_size);
    //printf("Reading ramdisk...\n");
    fread(ramdisk, header.ramdisk_size, 1, f);
    fwrite(ramdisk, header.ramdisk_size, 1, r);
    fclose(r);
    
    fclose(f);
    
    return 0;
}