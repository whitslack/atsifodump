#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <endian.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>

#define ATS_STRUCT struct __attribute__ ((__packed__))
#define ATS_SWAP16(v) ((v) = be16toh(v))
#define ATS_SWAP32(v) ((v) = be32toh(v))

typedef uint32_t sect_t; // sector
typedef uint32_t lptr_t; // long pointer
typedef uint16_t sptr_t; // short pointer
typedef uint32_t pts_t; // PTS ticks (= 1/90000 sec)
typedef uint8_t pad_t;

static const char ATS_IDENT[12] = "DVDAUDIO-ATS";

/*
 * Audio Title Set Index
 *
 * This structure is at the beginning of every ATS_xx_0.IFO file.
 */
ATS_STRUCT atsi_mat {
	/* 0x00..0x0b */ char ident[12];
	/* 0x0c..0x0f */ sect_t ats_end;
	/* 0x10..0x1b */ pad_t _reserved0[0x1c - 0x10];
	/* 0x1c..0x1f */ sect_t atsi_end;
	/* 0x20..0x21 */ uint16_t spec_ver;
	/* 0x22..0x7f */ pad_t _reserved1[0x80 - 0x22];
	/* 0x80..0x83 */ lptr_t mat_end;
	/* 0x84..0xcb */ pad_t _reserved2[0xcc - 0x84];
	/* 0xcc..0xcf */ sect_t pgci;
	/* 0xd0..0xe7 */ pad_t _reserved3[0xe8 - 0xd0];
};

/*
 * Program Chain Index
 *
 * This structure is at the beginning of the sector given by atsi_mat.pgci,
 * which is relative to the beginning of the ATSI. Immediately following this
 * structure are pgc_cnt contiguous instances of struct ats_pgci_elem.
 */
ATS_STRUCT ats_pgci {
	/* 0x00..0x01 */ uint16_t pgc_cnt;
	/* 0x02..0x03 */ pad_t _reserved0[0x04 - 0x02];
	/* 0x04..0x07 */ lptr_t pgci_end;
};

/*
 * Program Chain Index Element
 *
 * This structure associates with each program chain ID a byte offset (relative
 * to the beginning of the PGCI) to an instance of struct ats_pgc.
 */
ATS_STRUCT ats_pgci_elem {
	/* 0x00..0x00 */ uint8_t pgc_id;
	/* 0x01..0x03 */ pad_t _reserved0[0x04 - 0x01];
	/* 0x04..0x07 */ lptr_t pgc;
};

/*
 * Program Chain
 *
 * This structure lists the programs in a program chain. The tmap member
 * contains a byte offset (relative to the beginning of this structure) to
 * pg_cnt contiguous instances of struct ats_tmap_elem. The smap member
 * contains a byte offset to pg_cnt contiguous instances of struct
 * ats_smap_elem.
 */
ATS_STRUCT ats_pgc {
	/* 0x00..0x01 */ pad_t _reserved0[0x02 - 0x00];
	/* 0x02..0x02 */ uint8_t pg_cnt;
	/* 0x03..0x03 */ uint8_t cell_cnt;
	/* 0x04..0x07 */ pts_t pts_len;
	/* 0x08..0x09 */ pad_t _reserved1[0x0a - 0x08];
	/* 0x0a..0x0b */ sptr_t tmap;
	/* 0x0c..0x0d */ sptr_t smap;
	/* 0x0e..0x0f */ sptr_t _reserved2;
};

/*
 * Time Map Element
 *
 * This structure gives the program ID, start time, and duration of a program
 * in a program chain. The times are in PTS units (1/90000th of a second).
 */
ATS_STRUCT ats_tmap_elem {
	/* 0x00..0x03 */ pad_t _reserved0[0x04 - 0x00];
	/* 0x04..0x04 */ uint8_t pg_id;
	/* 0x05..0x05 */ pad_t _reserved1[0x06 - 0x05];
	/* 0x06..0x09 */ pts_t start;
	/* 0x0a..0x0d */ pts_t len;
	/* 0x0e..0x13 */ pad_t _reserved2[0x14 - 0x0e];
};

/*
 * Sector Map Element
 *
 * This structure gives the starting and ending sectors of a program in a
 * program chain, relative to the beginning of ATS_xx_1.AOB. The ending sector
 * is inclusive.
 */
ATS_STRUCT ats_smap_elem {
	/* 0x00..0x03 */ pad_t _reserved0[0x04 - 0x00];
	/* 0x04..0x07 */ sect_t start;
	/* 0x08..0x0b */ sect_t end;
};

static char * pts_str(char buf[/*16*/], size_t n_buf, pts_t ticks) {
	unsigned ms = ticks / 90;
	ticks %= 90;
	unsigned s = ms / 1000;
	ms %= 1000;
	unsigned m = s / 60;
	s %= 60;
	unsigned h = m / 60;
	m %= 60;
	size_t n = snprintf(buf, n_buf, "%u:%02u:%02u.%03u", h, m, s, ms);
	if (ticks != 0) {
		snprintf(buf + n, n_buf - n, "+%u", ticks);
	}
	return buf;
}

static const char * pts_str_cdda(char buf[/*9*/], size_t n_buf, pts_t ticks) {
	unsigned f = ticks / 1200;
	unsigned s = f / 75;
	f %= 75;
	unsigned m = s / 60;
	s %= 60;
	snprintf(buf, n_buf, "%02u:%02u:%02u", m, s, f);
	return buf;
}

int main(int argc, char *argv[]) {
	enum { TECHNICAL, CUESHEET, SCRIPT } format = TECHNICAL;
	unsigned pgc_id = ~0;
	bool usage = false;
	for (int c; (c = getopt(argc, argv, "cdst:")) >= 0;) {
		switch (c) {
			case 'c':
				format = CUESHEET;
				break;
			case 'd':
				format = TECHNICAL;
				break;
			case 's':
				format = SCRIPT;
				break;
			case 't':
				pgc_id = atoi(optarg);
				break;
			case '?':
			case ':':
				usage = true;
				continue;
		}
	}
	if (usage || optind != argc - 1) {
		fprintf(stderr,
				"usage: %s [-dcs] [-t<i>] ATS_xx_0.IFO\n"
				"  -d    Output technical details (default)\n"
				"  -c    Output cue sheet\n"
				"  -s    Output shell script\n"
				"  -t<i> Limit to program chain <i>\n",
				argv[0]);
		return -1;
	}
	const char *ifo_name = argv[optind];
	FILE *atsi;
	if (!(atsi = fopen(ifo_name, "r"))) {
		error(1, errno, "%s: failed to open", ifo_name);
	}
	struct atsi_mat mat;
	if (fread(&mat, sizeof mat, 1, atsi) < 1 ||
			memcmp(mat.ident, ATS_IDENT, sizeof ATS_IDENT) != 0) {
		error(1, errno, "%s: error reading ATSI_MAT; maybe not an ATS_xx_0.IFO file?", ifo_name);
	}
	ATS_SWAP32(mat.ats_end);
	ATS_SWAP32(mat.atsi_end);
	ATS_SWAP16(mat.spec_ver);
	ATS_SWAP32(mat.mat_end);
	ATS_SWAP32(mat.pgci);
	if (mat.spec_ver >> 4 != 1) {
		fprintf(stderr, "WARNING: This tool may not support DVD specification version %u.%u!\n",
				mat.spec_ver >> 4, mat.spec_ver & 0xF);
	}
	const char *prefix = ifo_name;
	size_t prefix_len = strlen(ifo_name);
	if (prefix_len >= 6 && strcasecmp(prefix + prefix_len - 6, "_0.IFO") == 0) {
		prefix_len -= 5;
	}
	else {
		prefix = "ATS_01_";
		prefix_len = 7;
	}
	switch (format) {
		case TECHNICAL:
			printf("Audio Title Set Index (ATSI)\n"
					"\tATS last sector: %u\n"
					"\tATSI last sector: %u\n"
					"\tDVD spec version: %u.%u\n"
					"\tPGCI first sector: %u\n"
					"\tProgram Chain Index (PGCI)\n",
					mat.ats_end,
					mat.atsi_end,
					mat.spec_ver >> 4, mat.spec_ver & 0xF,
					mat.pgci);
			break;
		case SCRIPT:
			printf("#!/bin/sh\ncat %.*s?.AOB | {\n", (int) prefix_len, prefix);
			break;
	}
	struct ats_pgci pgci;
	if (fseek(atsi, mat.pgci << 11, SEEK_SET) < 0 ||
			fread(&pgci, sizeof pgci, 1, atsi) < 1) {
		error(1, errno, "%s: error reading ATS_PGCI", ifo_name);
	}
	ATS_SWAP16(pgci.pgc_cnt);
	ATS_SWAP32(pgci.pgci_end);
	struct ats_pgci_elem pgci_elem[pgci.pgc_cnt];
	if (fread(pgci_elem, sizeof *pgci_elem, pgci.pgc_cnt, atsi) < pgci.pgc_cnt) {
		error(1, errno, "%s: error reading ATS_PGCI", ifo_name);
	}
	for (unsigned pgc_idx = 0; pgc_idx < pgci.pgc_cnt; ++pgc_idx) {
		ATS_SWAP32(pgci_elem[pgc_idx].pgc);
		if (!~pgc_id || (pgci_elem[pgc_idx].pgc_id & ~0x80) == pgc_id) {
			struct ats_pgc pgc;
			if (fseek(atsi, (mat.pgci << 11) + pgci_elem[pgc_idx].pgc, SEEK_SET) < 0 ||
					fread(&pgc, sizeof pgc, 1, atsi) < 1) {
				error(1, errno, "%s: error reading ATS_PGC", ifo_name);
			}
			ATS_SWAP32(pgc.pts_len);
			ATS_SWAP16(pgc.tmap);
			ATS_SWAP16(pgc.smap);
			switch (format) {
				case TECHNICAL: {
					char buf[16];
					printf("\t\tProgram Chain #%u\n"
							"\t\t\tProgram count: %u\n"
							"\t\t\tCell count: %u\n"
							"\t\t\tDuration: %u (%s)\n",
							pgci_elem[pgc_idx].pgc_id & ~0x80,
							pgc.pg_cnt,
							pgc.cell_cnt,
							pgc.pts_len, pts_str(buf, sizeof buf, pgc.pts_len));
					break;
				}
				case CUESHEET:
					puts("FILE \"\" FLAC");
					break;
			}
			struct ats_tmap_elem tmap[pgc.pg_cnt];
			if (fseek(atsi, (mat.pgci << 11) + pgci_elem[pgc_idx].pgc + pgc.tmap, SEEK_SET) < 0 ||
					fread(tmap, sizeof *tmap, pgc.pg_cnt, atsi) < pgc.pg_cnt) {
				error(1, errno, "%s: error reading ATS_TMAP", ifo_name);
			}
			struct ats_smap_elem smap[pgc.pg_cnt];
			if (fseek(atsi, (mat.pgci << 11) + pgci_elem[pgc_idx].pgc + pgc.smap, SEEK_SET) < 0 ||
					fread(smap, sizeof *smap, pgc.pg_cnt, atsi) < pgc.pg_cnt) {
				error(1, errno, "%s: error reading ATS_SMAP", ifo_name);
			}
			pts_t prev_pts_end = 0;
			sect_t prev_sect_end = 0;
			for (unsigned pg_idx = 0; pg_idx < pgc.pg_cnt; ++pg_idx) {
				ATS_SWAP32(tmap[pg_idx].start);
				ATS_SWAP32(tmap[pg_idx].len);
				ATS_SWAP32(smap[pg_idx].start);
				ATS_SWAP32(smap[pg_idx].end);
				pts_t pts_end = tmap[pg_idx].start + tmap[pg_idx].len;
				sect_t sect_cnt = smap[pg_idx].end + 1 - smap[pg_idx].start;
				switch (format) {
					case TECHNICAL: {
						char buf[3][16];
						printf("\t\t\tProgram #%u\n"
								"\t\t\t\tFirst PTS: %u (%s)\n"
								"\t\t\t\tLast PTS: %u (%s)\n"
								"\t\t\t\tDuration: %u (%s)\n"
								"\t\t\t\tFirst sector: %u\n"
								"\t\t\t\tLast sector: %u\n"
								"\t\t\t\tSector count: %u\n",
								tmap[pg_idx].pg_id,
								tmap[pg_idx].start, pts_str(buf[0], sizeof buf[0], tmap[pg_idx].start),
								pts_end - 1, pts_str(buf[1], sizeof buf[1], pts_end - 1),
								tmap[pg_idx].len, pts_str(buf[2], sizeof buf[2], tmap[pg_idx].len),
								smap[pg_idx].start,
								smap[pg_idx].end,
								sect_cnt);
						break;
					}
					case CUESHEET: {
						printf("  TRACK %02u AUDIO\n", pg_idx + 1);
						char buf[9];
						if (pg_idx > 0 && prev_pts_end < tmap[pg_idx].start) {
							printf("    INDEX 00 %s\n", pts_str_cdda(buf, sizeof buf, prev_pts_end));
						}
						printf("    INDEX 01 %s\n", pts_str_cdda(buf, sizeof buf, tmap[pg_idx].start));
						break;
					}
					case SCRIPT:
						if (pg_idx > 0 && smap[pg_idx].start > prev_sect_end + 1) {
							printf("\thead -c%u > /dev/null\n",
									(smap[pg_idx].start - (prev_sect_end + 1)) << 11);
						}
						printf("\thead -c%u | ffmpeg -i - Title%02u-Track%02u.flac\n",
								sect_cnt << 11,
								pgci_elem[pgc_idx].pgc_id & ~0x80,
								tmap[pg_idx].pg_id);
						break;
				}
				prev_pts_end = pts_end;
				prev_sect_end = smap[pg_idx].end;
			}
		}
	}
	if (format == SCRIPT) {
		puts("}");
	}
	return 0;
}
