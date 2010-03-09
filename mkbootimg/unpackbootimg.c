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

void read_padding(FILE* f, int size)
{
    byte buf[2048];
    int padding = 2048 - (size % 2048);
    fread(buf, padding, 1, f);
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: unpackbootimg <boot.img>\n");
        return 0;
    }
    
    FILE* f = fopen(argv[1], "rb");
    boot_img_hdr header;
    
    fread(&header, sizeof(header), 1, f);
    read_padding(f, sizeof(header));
    
    printf("BOARD_KERNEL_CMDLINE %s\n", header.cmdline);
    printf("BOARD_KERNEL_BASE %x\n", header.kernel_addr - 0x00008000);

    char* kernel_file[PATH_MAX];
    strcpy(kernel_file, argv[1]);
    strcat(kernel_file, "-kernel.gz");
    FILE *k = fopen(kernel_file, "wb");
    byte* kernel = (byte*)malloc(header.kernel_size);
    fread(kernel, header.kernel_size, 1, f);
    fwrite(kernel, header.kernel_size, 1, k);
    fclose(k);
    read_padding(f, sizeof(header.kernel_size));
    
    char* ramdisk_file[PATH_MAX];
    strcpy(ramdisk_file, argv[1]);
    strcat(ramdisk_file, "-ramdisk.cpio.gz");
    FILE *r = fopen(ramdisk_file, "wb");
    byte* ramdisk = (byte*)malloc(header.ramdisk_size);
    fread(ramdisk, header.ramdisk_size, 1, f);
    fwrite(ramdisk, header.ramdisk_size, 1, r);
    fclose(r);
    
    fclose(f);
    
    return 0;
}