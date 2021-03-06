// readmcr --- dump a microcode (MCR) file

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>

#include "usim.h"
#include "ucode.h"
#include "misc.h"

bool showmcr;
bool debug;
bool needswap;
int skip;

void
dump_i_mem(int fd, int start, int size)
{
	int loc;

	printf("i-memory; start %o, size %o\n", start, size);
	loc = start;
	for (int i = 0; i < size; i++) {
		uint16_t w1;
		uint16_t w2;
		uint16_t w3;
		uint16_t w4;
		ucw_t ll;

		w1 = read16(fd);
		w2 = read16(fd);
		w3 = read16(fd);
		w4 = read16(fd);
		ll =
			((ucw_t) w1 << 48) |
			((ucw_t) w2 << 32) |
			((ucw_t) w3 << 16) |
			((ucw_t) w4 << 0);

		if (showmcr)
			printf("%03o %016" PRIo64 "\n", loc, ll);

		loc++;
	}
}

void
dump_d_mem(int fd, int start, int size)
{
	printf("d-memory; start %o, size %o\n", start, size);
	for (int i = 0; i < size; i++) {
		read16(fd);
		read16(fd);
	}
}

void
dump_a_mem(int fd, int start, int size)
{
	printf("a-memory; start %o, size %o\n", start, size);
	for (int i = 0; i < size; i++) {
		uint32_t v;

		v = read32(fd);

		if ((i >= 0347 && i <= 0400) |
		    (i >= 0600 && i <= 0610) |
		    (i < 010)) {
			printf("%o <- %" PRIo32 "\n", i, v);
		}
	}
}

void
dump_main_mem(int fd, int start, int size)
{
	printf("main-memory; start %d, size %d\n", start, size);
	read32(fd);
	lseek(fd, 0, SEEK_CUR);
}

void
usage(void)
{
	fprintf(stderr, "usage: readmcr FILE\n");
	fprintf(stderr, "dump a microcode file\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  -b             swap bytes\n");
	fprintf(stderr, "  -d             extra debug info\n");
	fprintf(stderr, "  -s N           skip N * 32-bit values\n");
	fprintf(stderr, "  -m             show microcode\n");
	fprintf(stderr, "  -h             show help message\n");
}

int
main(int argc, char *argv[])
{
	int c;
	int fd;
	bool done;

	showmcr = false;
	needswap = true;
	skip = false;

	while ((c = getopt(argc, argv, "bds:mh")) != -1) {
		switch (c) {
		case 'b':
			needswap = false;
			break;
		case 'd':
			debug = true;
			break;
		case 's':
			skip = atoi(optarg);
			break;
		case 'm':
			showmcr = true;
			break;
		case 'h':
			usage();
			exit(0);
		default:
			usage();
			exit(1);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1) {
		usage();
		exit(1);
	}

	fd = open(argv[0], O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "%s: no such file or directory\n", argv[0]);
		exit(1);
	}

	if (skip) {
		while (skip--)
			read32(fd);
	}

	done = false;
	while (!done) {
		int code;
		int start;
		int size;

		code = read32(fd);
		start = read32(fd);
		size = read32(fd);

		switch (code) {
		case 1:
			dump_i_mem(fd, start, size);
			break;
		case 2:
			dump_d_mem(fd, start, size);
			break;
		case 3:
			dump_main_mem(fd, start, size);
			break;
		case 4:
			dump_a_mem(fd, start, size);
			done = true;
			break;
		}
	}

	exit(0);
}
