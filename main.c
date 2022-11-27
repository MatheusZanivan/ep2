// Matheus Zanivan Andrade
// referencias
// https://wiki.osdev.org/MBR_(x86)
// https://cpl.li/posts/2019-03-12-mbrfat/
//
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MBR_SIZE 512
#define NUM_PARTITIONS 4
#define SEC_PARTITION_START	0x1BE
#define SEC_MAGICBYTES_START 0x1FE
#define OFF_START_CHS 0x1
#define OFF_PART_DESC 0x4
#define OFF_END_CHS	0x5
#define OFF_START_SEC 0x8
#define OFF_PART_SZ	0xC


static uint8_t mbr[512];

static char *	sector_to_size_str(uint32_t);
static void	check_starting_sector(uint32_t);
static void	check_partition_fs(uint32_t);
static void	check_boot_indicator(uint32_t);
static void	check_chs_values(char *, uint32_t);
static uint32_t check_partition_size(uint32_t);


int main(int agc, char *argv[]){
	
	FILE *fp = fopen(argv[1], "r");
	if (!fp) {
		printf("nao localizado %s, tente rodar como root\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	fread(&mbr, MBR_SIZE, 1, fp);

	if (mbr[SEC_MAGICBYTES_START+1]==0xaa && mbr[SEC_MAGICBYTES_START]==0x55)
		puts("** MBR magic bytes");
	else {
		puts("** Error -did not read magic bytes");
		exit(EXIT_FAILURE);
	}

	for (uint8_t i = 0; i < NUM_PARTITIONS; i++) {
		uint32_t start = SEC_PARTITION_START + i*0x10;
		printf("analizando %u\n", i);
		check_boot_indicator(start);
		if(!check_partition_size(start+OFF_PART_SZ))
			continue;
		check_chs_values("comeco", start+OFF_START_CHS);
		check_chs_values("fim", start+OFF_END_CHS);
		check_starting_sector(start+OFF_START_SEC);
		check_partition_fs(start+OFF_PART_DESC);
	}
}

static void check_boot_indicator(uint32_t start) {
	if (mbr[start]==0x00)
		puts("\tnull");
	else if(mbr[start]==0x80)
		puts("\tboot good");
	else
		puts("\tError");
}


static void check_chs_values(char *str, uint32_t start) {
	uint8_t c,h,s;
	c = mbr[start];
	h = mbr[start+0x1];
	s = mbr[start+0x2];
	printf("\t%s CHS values C=%u, H=%u, S=%u\n", str, c, h, s);
}


static void check_starting_sector(uint32_t start) {
	uint8_t *start_loc = &mbr[start];
	uint32_t start_sec = *(uint32_t *)start_loc;
	char *sz_str = sector_to_size_str(start_sec);
	printf("\tparticao comeca na sec %u (%s in)\n", start_sec, sz_str);
}


static uint32_t check_partition_size(uint32_t start) {
	uint8_t *size_loc = &mbr[start];
	uint32_t size = *(uint32_t *)size_loc;
	char *sz_str = sector_to_size_str(size);
	if (size) {
		sz_str = sector_to_size_str(size);
		printf("\tparticao %u sec (%s)\n", size, sz_str);
	} else
		puts("\te vazio");
	return size;
}


static void check_partition_fs(uint32_t start) {
	uint8_t fs_id = mbr[start];
	printf("\to id da particao e %u\n", fs_id);
}


static char * sector_to_size_str(uint32_t size) {
	double sz_kb, sz_mb, sz_gb;
	char *sz_str = malloc(10);
	sz_kb = size / 2;
	sz_mb = sz_kb / 1024;
	sz_gb = sz_mb / 1024;
	if (sz_gb>=1)
		sprintf(sz_str, "%.2fGB", sz_gb);
	else if (sz_mb>=1)
		sprintf(sz_str, "%.2fMB", sz_mb);
	else 
		sprintf(sz_str, "%.2fKB", sz_kb);
	return sz_str;
}