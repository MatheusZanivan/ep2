/*#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//SENAC - BCC - 2022


#define MBR_SIZE 512
#define BOOT_SIG_START 510
#define NUM_PARTITIONS 4
#define PARTITION_START 446

typedef struct Partitions {
  uint32_t start_block;
  uint32_t end_block;
  uint32_t sector;
  uint8_t type;
  uint8_t bootable;
} partition;

int is_little_endian(void);
int is_mbr_valid(uint8_t*);
int is_bootable(uint32_t, uint8_t *);
uint32_t get_disk_size(uint32_t, uint8_t*);
char *str_sector(uint32_t);
char *get_format(uint8_t);

int main(void) {
  uint8_t mbr[MBR_SIZE];
  uint32_t disk_size;

  //usa o struct para armazenar os dados das particoes
  partition *partitions = malloc(NUM_PARTITIONS * sizeof(partition));

  FILE *file = fopen("mbr.bin", "rb");
  if (!file) {
    printf("\nError opening mbr.bin file!");
    exit(-1);
  }

  int result = fread(&mbr, MBR_SIZE, 1, file);

  //checa o boot signature
  if (!is_mbr_valid(mbr)) {
    printf("\nMRB Invalido. Abortando..");
    exit(1);
  }

  //coleta a informacao buscando nos enderecos.
  for (int i=0; i<NUM_PARTITIONS; i++) {
    uint32_t start = PARTITION_START + i * 16;
    partitions[i].start_block = *(uint32_t*) &mbr[start + 8];
    partitions[i].end_block = *(uint32_t*) &mbr[start + 12];
    partitions[i].sector = *(uint32_t*) &mbr[start + 12];
    partitions[i].type = *(uint8_t*) &mbr[start + 4];
    partitions[i].bootable = is_bootable(start, mbr);
  }

  disk_size = *(uint32_t*) &mbr[PARTITION_START + 12];

  printf("\nDisk /dev/sda: %s, %u bytes, %u sectors \n", str_sector(disk_size), disk_size, &mbr[PARTITION_START + 12]);
  printf("\n\x1B[1mDevice    Start        End          Sectors    Size     Type\x1B[0m");

  for (int i=0; i<NUM_PARTITIONS; i++) {
    printf("\n/dev/sda%d %-12d %-12d %-10d %-8s %-10s", i+1,
    partitions[i].start_block, 
    partitions[i].end_block, 
    partitions[i].sector,
    str_sector(partitions[i].sector),
    get_format(partitions[i].type));
  }

  free(partitions);
  return 0;
}

uint32_t get_disk_size(uint32_t start, uint8_t *mbr) {
  uint32_t size = *(uint32_t*) &mbr[start];
  if (size) {
    return size * 512;
  }
  return 0;
}

//pega o formato da particao
char * get_format(uint8_t form) {
  char *format = malloc(20);

  switch (form) {
  case 0x0: sprintf(format, "Empty");
    break;
  case 0xbf: sprintf(format, "Solaris");
    break;
  case 0x82: sprintf(format, "Linux Swap");
    break;
  case 0x83: sprintf(format, "Linux");
    break;
  case 0x84: sprintf(format, "Win HP");
    break;
  case 0x85: sprintf(format, "Linux Extended");
    break;
  case 0x86: sprintf(format, "NTFS");
    break;
  case 0x87: sprintf(format, "NTFS");
    break;
  case 0xee: sprintf(format, "MS GPT");
    break;
  case 0x7: sprintf(format, "HPFS");
    break;
  default: sprintf(format, "Unkown");
  }

  return format;
}

int is_mbr_valid(uint8_t* mbr) {
  if (mbr[BOOT_SIG_START] == 0x55 && mbr[BOOT_SIG_START + 1] == 0xaa) {
    return 1;
  }
  return 0;
}

int is_bootable(uint32_t pos, uint8_t* mbr) {
  if (mbr[pos] == 0x80) { 
    return 1;
  }
  return 0;
}

char * str_sector(uint32_t size) {
  double kb, mb, gb;
  char *f_size = malloc(10);
  kb = size / 2;
  mb = kb / 1024;
  gb = mb / 1024;

  if (gb > 1) {
    sprintf(f_size, "%2.fGB", gb);
  }
  else if (mb >= 1) {
    sprintf(f_size, "%2.fMB", mb);
  }
  else {
    sprintf(f_size, "%2.fKB", kb);
  }
  return f_size;
}

int is_little_endian() {
  unsigned int x = 0x76543210;
  char *c = (char*) &x;
  if (*c == 0x10) {
    printf("LITTLE ENDIAN");
    return 1;
  }
  else {
    printf("BIG ENDIAN");
    return 0;
  }
}
*/
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

// gcc ep.c -o main
// ./main mbr.bin
//colocar em uma estrutura de gente os printsS 

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
