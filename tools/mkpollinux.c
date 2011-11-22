/*
 * (C) Copyright 201
 * Laszlo Hegedues <laszlo.hegedues@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
 
#include <stdio.h> 
#include <string.h>
#include <stdint.h>

/* handy macros taken from tuxx's merge_loader.c */
#define BLOCKSIZE 0x200
#define NEXT_BLOCK(a) BLOCKSIZE-(a)%BLOCKSIZE

#define ERASEBLOCK 0x4000
#define NEXT_EBLOCK(a) ERASEBLOCK-(a)%ERASEBLOCK

static const char* header_wceo0 = "0ECW";
static const uint32_t header_const1 = 0x0003FE00;
static const uint32_t header_const2 = 0x00100000;

int create_pollinux_load(char *source_file, char *dest_file)
{
	FILE *src, *dst;
	int offset = 0;
	
	src = fopen(source_file, "r");
	if(src == NULL) {
		printf("input file does not exist!\n");
		return 1;
	}
	
	dst = fopen(dest_file, "w+");
	if(dst == NULL) {
		printf("Could not create destination file!\n");
		fclose(src);
		return 1;
	}
	
	/* construct file header */
	fwrite(header_wceo0, strlen(header_wceo0), 1, dst);
	fwrite(&header_const1, sizeof(header_const1), 1, dst);
	fwrite(header_wceo0, strlen(header_wceo0), 1, dst);
	fwrite(&header_const2, sizeof(header_const2), 1, dst);
	
	offset += 2 * strlen(header_wceo0) + sizeof(header_const1) + sizeof(header_const2);
	
	printf("Header written, moving offset: %d -> %d\n", offset, offset + NEXT_BLOCK(offset));
	offset += NEXT_BLOCK(offset);
	fseek(dst, offset, SEEK_SET);
	
	while(!feof(src)) {
		char buf[BLOCKSIZE];
		int read;
		
		read = fread(buf, 1, BLOCKSIZE, src);
		fwrite(buf, 1, read, dst);
		offset += read;
	}
	printf("Source file \"%s\" written\n", source_file);

	printf("filling to eraseblock, moving offset %d -> %d\n", offset, offset + NEXT_EBLOCK(offset));
	offset += NEXT_EBLOCK(offset);
	fseek(dst, offset - 1, SEEK_SET);
	fputc(0x00, dst);
	
	printf("Created \"%s\", total file size: %d bytes\n", dest_file, offset);
	
	fclose(src);
	fclose(dst);
	return 0;
}

int
main(int argc, char *argv[])
{
	int ret = 1;

	if(argc == 3) {
		ret = create_pollinux_load(argv[1], argv[2]);
	}
	else {
		printf("Usage: %s <input> <output>\n", argv[0]);
	}
	
	return ret;
}

