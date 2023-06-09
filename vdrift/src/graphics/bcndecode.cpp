/*
 * Decoder for DXTn-compressed data adapted from Python Imaging Library
 *
 * Format documentation:
 *   https://web.archive.org/web/20170802060935/http://oss.sgi.com/projects/ogl-sample/registry/EXT/texture_compression_s3tc.txt
 *
 * The contents of this file are in the public domain (CC0)
 * Full text of the CC0 license:
 *   https://creativecommons.org/publicdomain/zero/1.0/
 */

#include "bcndecode.h"

#include <cstdint>
#include <cstring>

using namespace std;

typedef struct {
	uint8_t r, g, b, a;
} rgba;

typedef struct {
	uint8_t l;
} lum;

typedef struct {
	float r, g, b;
} rgb32f;

typedef struct {
	uint16_t c0, c1;
	uint32_t lut;
} bc1_color;

typedef struct {
	uint8_t a0, a1;
	uint8_t lut[6];
} bc3_alpha;

#define LOAD16(p) \
	(p)[0] | ((p)[1] << 8)

#define LOAD32(p) \
	(p)[0] | ((p)[1] << 8) | ((p)[2] << 16) | ((p)[3] << 24)

static void bc1_color_load(bc1_color *dst, const uint8_t *src) {
	dst->c0 = LOAD16(src);
	dst->c1 = LOAD16(src + 2);
	dst->lut = LOAD32(src + 4);
}

static void bc3_alpha_load(bc3_alpha *dst, const uint8_t *src) {
	memcpy(dst, src, sizeof(bc3_alpha));
}

static rgba decode_565(uint16_t x) {
	rgba c;
	int r, g, b;
	r = (x & 0xf800) >> 8;
	r |= r >> 5;
	c.r = r;
	g = (x & 0x7e0) >> 3;
	g |= g >> 6;
	c.g = g;
	b = (x & 0x1f) << 3;
	b |= b >> 5;
	c.b = b;
	c.a = 0xff;
	return c;
}

static void decode_bc1_color(rgba *dst, const uint8_t *src) {
	bc1_color col;
	rgba p[4];
	int n, cw;
	uint16_t r0, g0, b0, r1, g1, b1;
	bc1_color_load(&col, src);

	p[0] = decode_565(col.c0);
	r0 = p[0].r;
	g0 = p[0].g;
	b0 = p[0].b;
	p[1] = decode_565(col.c1);
	r1 = p[1].r;
	g1 = p[1].g;
	b1 = p[1].b;
	if (col.c0 > col.c1) {
		p[2].r = (2*r0 + 1*r1) / 3;
		p[2].g = (2*g0 + 1*g1) / 3;
		p[2].b = (2*b0 + 1*b1) / 3;
		p[2].a = 0xff;
		p[3].r = (1*r0 + 2*r1) / 3;
		p[3].g = (1*g0 + 2*g1) / 3;
		p[3].b = (1*b0 + 2*b1) / 3;
		p[3].a = 0xff;
	} else {
		p[2].r = (r0 + r1) / 2;
		p[2].g = (g0 + g1) / 2;
		p[2].b = (b0 + b1) / 2;
		p[2].a = 0xff;
		p[3].r = 0;
		p[3].g = 0;
		p[3].b = 0;
		p[3].a = 0;
	}
	for (n = 0; n < 16; n++) {
		cw = 3 & (col.lut >> (2 * n));
		dst[n] = p[cw];
	}
}

static void decode_bc3_alpha(char *dst, const uint8_t *src, int stride, int o) {
	bc3_alpha b;
	uint16_t a0, a1;
	uint8_t a[8];
	int n, lut, aw;
	bc3_alpha_load(&b, src);

	a0 = b.a0;
	a1 = b.a1;
	a[0] = (uint8_t)a0;
	a[1] = (uint8_t)a1;
	if (a0 > a1) {
		a[2] = (6*a0 + 1*a1) / 7;
		a[3] = (5*a0 + 2*a1) / 7;
		a[4] = (4*a0 + 3*a1) / 7;
		a[5] = (3*a0 + 4*a1) / 7;
		a[6] = (2*a0 + 5*a1) / 7;
		a[7] = (1*a0 + 6*a1) / 7;
	} else {
		a[2] = (4*a0 + 1*a1) / 5;
		a[3] = (3*a0 + 2*a1) / 5;
		a[4] = (2*a0 + 3*a1) / 5;
		a[5] = (1*a0 + 4*a1) / 5;
		a[6] = 0;
		a[7] = 0xff;
	}
	lut = b.lut[0] | (b.lut[1] << 8) | (b.lut[2] << 16);
	for (n = 0; n < 8; n++) {
		aw = 7 & (lut >> (3 * n));
		dst[stride * n + o] = a[aw];
	}
	lut = b.lut[3] | (b.lut[4] << 8) | (b.lut[5] << 16);
	for (n = 0; n < 8; n++) {
		aw = 7 & (lut >> (3 * n));
		dst[stride * (8+n) + o] = a[aw];
	}
}

static void decode_bc1_block(rgba *col, const uint8_t* src) {
	decode_bc1_color(col, src);
}

static void decode_bc2_block(rgba *col, const uint8_t* src) {
	int n, bitI, byI, av;
	decode_bc1_color(col, src + 8);
	for (n = 0; n < 16; n++) {
		bitI = n * 4;
		byI = bitI >> 3;
		av = 0xf & (src[byI] >> (bitI & 7));
		av = (av << 4) | av;
		col[n].a = av;
	}
}

static void decode_bc3_block(rgba *col, const uint8_t* src) {
	decode_bc1_color(col, src + 8);
	decode_bc3_alpha((char *)col, src, sizeof(col[0]), 3);
}

static void decode_bc4_block(lum *col, const uint8_t* src) {
	decode_bc3_alpha((char *)col, src, sizeof(col[0]), 0);
}

static void decode_bc5_block(rgba *col, const uint8_t* src) {
	decode_bc3_alpha((char *)col, src, sizeof(col[0]), 0);
	decode_bc3_alpha((char *)col, src + 8, sizeof(col[0]), 1);
}

/* BC6 and BC7 are described here:
   https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_compression_bptc.txt */

static uint8_t get_bit(const uint8_t* src, int bit) {
	int by = bit >> 3;
	bit &= 7;
	return (src[by] >> bit) & 1;
}

static uint8_t get_bits(const uint8_t* src, int bit, int count) {
	uint8_t v;
	int x;
	int by = bit >> 3;
	bit &= 7;
	if (!count) {
		return 0;
	}
	if (bit + count <= 8) {
		v = (src[by] >> bit) & ((1 << count) - 1);
	} else {
		x = src[by] | (src[by+1] << 8);
		v = (x >> bit) & ((1 << count) - 1);
	}
	return v;
}

/* BC7 */
typedef struct {
	char ns;
	char pb;
	char rb;
	char isb;
	char cb;
	char ab;
	char epb;
	char spb;
	char ib;
	char ib2;
} bc7_mode_info;

static const bc7_mode_info bc7_modes[] = {
	{3, 4, 0, 0, 4, 0, 1, 0, 3, 0},
	{2, 6, 0, 0, 6, 0, 0, 1, 3, 0},
	{3, 6, 0, 0, 5, 0, 0, 0, 2, 0},
	{2, 6, 0, 0, 7, 0, 1, 0, 2, 0},
	{1, 0, 2, 1, 5, 6, 0, 0, 2, 3},
	{1, 0, 2, 0, 7, 8, 0, 0, 2, 2},
	{1, 0, 0, 0, 7, 7, 1, 0, 4, 0},
	{2, 6, 0, 0, 5, 5, 1, 0, 2, 0}
};

/* Subset indices:
   Table.P2, 1 bit per index */
static const uint16_t bc7_si2[] = {
	0xcccc, 0x8888, 0xeeee, 0xecc8, 0xc880, 0xfeec, 0xfec8, 0xec80,
	0xc800, 0xffec, 0xfe80, 0xe800, 0xffe8, 0xff00, 0xfff0, 0xf000,
	0xf710, 0x008e, 0x7100, 0x08ce, 0x008c, 0x7310, 0x3100, 0x8cce,
	0x088c, 0x3110, 0x6666, 0x366c, 0x17e8, 0x0ff0, 0x718e, 0x399c,
	0xaaaa, 0xf0f0, 0x5a5a, 0x33cc, 0x3c3c, 0x55aa, 0x9696, 0xa55a,
	0x73ce, 0x13c8, 0x324c, 0x3bdc, 0x6996, 0xc33c, 0x9966, 0x0660,
	0x0272, 0x04e4, 0x4e40, 0x2720, 0xc936, 0x936c, 0x39c6, 0x639c,
	0x9336, 0x9cc6, 0x817e, 0xe718, 0xccf0, 0x0fcc, 0x7744, 0xee22};

/* Table.P3, 2 bits per index */
static const uint32_t bc7_si3[] = {
	0xaa685050, 0x6a5a5040, 0x5a5a4200, 0x5450a0a8,
	0xa5a50000, 0xa0a05050, 0x5555a0a0, 0x5a5a5050,
	0xaa550000, 0xaa555500, 0xaaaa5500, 0x90909090,
	0x94949494, 0xa4a4a4a4, 0xa9a59450, 0x2a0a4250,
	0xa5945040, 0x0a425054, 0xa5a5a500, 0x55a0a0a0,
	0xa8a85454, 0x6a6a4040, 0xa4a45000, 0x1a1a0500,
	0x0050a4a4, 0xaaa59090, 0x14696914, 0x69691400,
	0xa08585a0, 0xaa821414, 0x50a4a450, 0x6a5a0200,
	0xa9a58000, 0x5090a0a8, 0xa8a09050, 0x24242424,
	0x00aa5500, 0x24924924, 0x24499224, 0x50a50a50,
	0x500aa550, 0xaaaa4444, 0x66660000, 0xa5a0a5a0,
	0x50a050a0, 0x69286928, 0x44aaaa44, 0x66666600,
	0xaa444444, 0x54a854a8, 0x95809580, 0x96969600,
	0xa85454a8, 0x80959580, 0xaa141414, 0x96960000,
	0xaaaa1414, 0xa05050a0, 0xa0a5a5a0, 0x96000000,
	0x40804080, 0xa9a8a9a8, 0xaaaaaa44, 0x2a4a5254};

/* Anchor indices:
   Table.A2 */
static const char bc7_ai0[] = {
	15,15,15,15,15,15,15,15,
	15,15,15,15,15,15,15,15,
	15, 2, 8, 2, 2, 8, 8,15,
	 2, 8, 2, 2, 8, 8, 2, 2,
	15,15, 6, 8, 2, 8,15,15,
	 2, 8, 2, 2, 2,15,15, 6,
	 6, 2, 6, 8,15,15, 2, 2,
	15,15,15,15,15, 2, 2,15};

/* Table.A3a */
static const char bc7_ai1[] = {
	 3, 3,15,15, 8, 3,15,15,
	 8, 8, 6, 6, 6, 5, 3, 3,
	 3, 3, 8,15, 3, 3, 6,10,
	 5, 8, 8, 6, 8, 5,15,15,
	 8,15, 3, 5, 6,10, 8,15,
	15, 3,15, 5,15,15,15,15,
	 3,15, 5, 5, 5, 8, 5,10,
	 5,10, 8,13,15,12, 3, 3};

/* Table.A3b */
static const char bc7_ai2[] = {
	15, 8, 8, 3,15,15, 3, 8,
	15,15,15,15,15,15,15, 8,
	15, 8,15, 3,15, 8,15, 8,
	 3,15, 6,10,15,15,10, 8,
	15, 3,15,10,10, 8, 9,10,
	 6,15, 8,15, 3, 6, 6, 8,
	15, 3,15,15,15,15,15,15,
	15,15,15,15, 3,15,15, 8};

/* Interpolation weights */
static const char bc7_weights2[] = {0, 21, 43, 64};
static const char bc7_weights3[] = {0, 9, 18, 27, 37, 46, 55, 64};
static const char bc7_weights4[] = {
	0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64};

static const char *bc7_get_weights(int n) {
	if (n == 2) {
		return bc7_weights2;
	}
	if (n == 3) {
		return bc7_weights3;
	}
	return bc7_weights4;
}

static int bc7_get_subset(int ns, int partition, int n) {
	if (ns == 2) {
		return 1 & (bc7_si2[partition] >> n);
	}
	if (ns == 3) {
		return 3 & (bc7_si3[partition] >> (2 * n));
	}
	return 0;
}

static uint8_t expand_quantized(uint8_t v, int bits) {
	v = v << (8 - bits);
	return v | (v >> bits);
}

static void bc7_lerp(rgba *dst, const rgba *e, int s0, int s1) {
	int t0 = 64 - s0;
	int t1 = 64 - s1;
	dst->r = (uint8_t)((t0 * e[0].r + s0 * e[1].r + 32) >> 6);
	dst->g = (uint8_t)((t0 * e[0].g + s0 * e[1].g + 32) >> 6);
	dst->b = (uint8_t)((t0 * e[0].b + s0 * e[1].b + 32) >> 6);
	dst->a = (uint8_t)((t1 * e[0].a + s1 * e[1].a + 32) >> 6);
}

static void decode_bc7_block(rgba *col, const uint8_t* src) {
	rgba endpoints[6];
	int bit = 0, cibit, aibit;
	int mode = src[0];
	int i, j;
	int numep, cb, ab, ib, ib2, i0, i1, s;
	uint8_t index_sel, partition, rotation, val;
	const char *cw, *aw;
	const bc7_mode_info *info;

	/* mode is the number of unset bits before the first set bit: */
	if (!mode) {
		/* degenerate case when no bits set */
		for (i = 0; i < 16; i++) {
			col[i].r = col[i].g = col[i].b = 0;
			col[i].a = 255;
		}
		return;
	}
	while (!(mode & (1 << bit++))) ;
	mode = bit - 1;
	info = &bc7_modes[mode];
	/* color selection bits: {subset}{endpoint} */
	cb = info->cb;
	ab = info->ab;
	cw = bc7_get_weights(info->ib);
	aw = bc7_get_weights((ab && info->ib2) ? info->ib2 : info->ib);

#define LOAD(DST, N) \
	DST = get_bits(src, bit, N); \
	bit += N;
	LOAD(partition, info->pb);
	LOAD(rotation, info->rb);
	LOAD(index_sel, info->isb);
	numep = info->ns << 1;

	/* red */
	for (i = 0; i < numep; i++) {
		LOAD(val, cb);
		endpoints[i].r = val;
	}

	/* green */
	for (i = 0; i < numep; i++) {
		LOAD(val, cb);
		endpoints[i].g = val;
	}

	/* blue */
	for (i = 0; i < numep; i++) {
		LOAD(val, cb);
		endpoints[i].b = val;
	}

	/* alpha */
	for (i = 0; i < numep; i++) {
		if (ab) {
			LOAD(val, ab);
		} else {
			val = 255;
		}
		endpoints[i].a = val;
	}

	/* p-bits */
#define ASSIGN_P(x) x = (x << 1) | val
	if (info->epb) {
		/* per endpoint */
		cb++;
		if (ab) {
			ab++;
		}
		for (i = 0; i < numep; i++) {
			LOAD(val, 1);
			ASSIGN_P(endpoints[i].r);
			ASSIGN_P(endpoints[i].g);
			ASSIGN_P(endpoints[i].b);
			if (ab) {
				ASSIGN_P(endpoints[i].a);
			}
		}
	}
	if (info->spb) {
		/* per subset */
		cb++;
		if (ab) {
			ab++;
		}
		for (i = 0; i < numep; i+=2) {
			LOAD(val, 1);
			for (j = 0; j < 2; j++) {
				ASSIGN_P(endpoints[i+j].r);
				ASSIGN_P(endpoints[i+j].g);
				ASSIGN_P(endpoints[i+j].b);
				if (ab) {
					ASSIGN_P(endpoints[i+j].a);
				}
			}
		}
	}
#undef ASSIGN_P
#define EXPAND(x, b) x = expand_quantized(x, b)
	for (i = 0; i < numep; i++) {
		EXPAND(endpoints[i].r, cb);
		EXPAND(endpoints[i].g, cb);
		EXPAND(endpoints[i].b, cb);
		if (ab) {
			EXPAND(endpoints[i].a, ab);
		}
	}
#undef EXPAND
#undef LOAD
	cibit = bit;
	aibit = cibit + 16 * info->ib - info->ns;
	for (i = 0; i < 16; i++) {
		s = bc7_get_subset(info->ns, partition, i) << 1;
		ib = info->ib;
		if (i == 0) {
			ib--;
		} else if (info->ns == 2) {
			if (i == bc7_ai0[partition]) {
				ib--;
			}
		} else if (info->ns == 3) {
			if (i == bc7_ai1[partition]) {
				ib--;
			} else if (i == bc7_ai2[partition]) {
				ib--;
			}
		}
		i0 = get_bits(src, cibit, ib);
		cibit += ib;

		if (ab && info->ib2) {
			ib2 = info->ib2;
			if (ib2 && i == 0) {
				ib2--;
			}
			i1 = get_bits(src, aibit, ib2);
			aibit += ib2;
			if (index_sel) {
				bc7_lerp(&col[i], &endpoints[s], aw[i1], cw[i0]);
			} else {
				bc7_lerp(&col[i], &endpoints[s], cw[i0], aw[i1]);
			}
		} else {
			bc7_lerp(&col[i], &endpoints[s], cw[i0], cw[i0]);
		}
#define ROTATE(x, y) \
		val = x; \
		x = y; \
		y = val
		if (rotation == 1) {
			ROTATE(col[i].r, col[i].a);
		} else if (rotation == 2) {
			ROTATE(col[i].g, col[i].a);
		} else if (rotation == 3) {
			ROTATE(col[i].b, col[i].a);
		}
#undef ROTATE
	}
}

/* BC6 */
typedef struct {
	char ns; /* number of subsets (also called regions) */
	char tr; /* whether endpoints are delta-compressed */
	char pb; /* partition bits */
	char epb; /* endpoint bits */
	char rb; /* red bits (delta) */
	char gb; /* green bits (delta) */
	char bb; /* blue bits (delta) */
} bc6_mode_info;

static const bc6_mode_info bc6_modes[] = {
	// 00
	{2, 1, 5, 10, 5, 5, 5},
	// 01
	{2, 1, 5,  7, 6, 6, 6},
	// 10
	{2, 1, 5, 11, 5, 4, 4},
	{2, 1, 5, 11, 4, 5, 4},
	{2, 1, 5, 11, 4, 4, 5},
	{2, 1, 5,  9, 5, 5, 5},
	{2, 1, 5,  8, 6, 5, 5},
	{2, 1, 5,  8, 5, 6, 5},
	{2, 1, 5,  8, 5, 5, 6},
	{2, 0, 5,  6, 6, 6, 6},
	// 11
	{1, 0, 0, 10, 10, 10, 10},
	{1, 1, 0, 11,  9,  9,  9},
	{1, 1, 0, 12,  8,  8,  8},
	{1, 1, 0, 16,  4,  4,  4}
};

/* Table.F, encoded as a sequence of bit indices */
static const uint8_t bc6_bit_packings[][75] = {
	{116, 132, 176, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 17, 18, 19, 20, 21, 22,
	 23, 24, 25, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 50, 51, 52,
	 164, 112, 113, 114, 115, 64, 65, 66, 67, 68, 172, 160, 161, 162, 163, 80,
	 81, 82, 83, 84, 173, 128, 129, 130, 131, 96, 97, 98, 99, 100, 174, 144,
	 145, 146, 147, 148, 175},
	{117, 164, 165, 0, 1, 2, 3, 4, 5, 6, 172, 173, 132, 16, 17, 18, 19, 20, 21,
	 22, 133, 174, 116, 32, 33, 34, 35, 36, 37, 38, 175, 177, 176, 48, 49, 50,
	 51, 52, 53, 112, 113, 114, 115, 64, 65, 66, 67, 68, 69, 160, 161, 162, 163,
	 80, 81, 82, 83, 84, 85, 128, 129, 130, 131, 96, 97, 98, 99, 100, 101, 144,
	 145, 146, 147, 148, 149},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 32,
	 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 50, 51, 52, 10, 112, 113, 114,
	 115, 64, 65, 66, 67, 26, 172, 160, 161, 162, 163, 80, 81, 82, 83, 42, 173,
	 128, 129, 130, 131, 96, 97, 98, 99, 100, 174, 144, 145, 146, 147, 148,
	 175},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 32,
	 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 50, 51, 10, 164, 112, 113, 114,
	 115, 64, 65, 66, 67, 68, 26, 160, 161, 162, 163, 80, 81, 82, 83, 42, 173,
	 128, 129, 130, 131, 96, 97, 98, 99, 172, 174, 144, 145, 146, 147, 116,
	 175},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 32,
	 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 50, 51, 10, 132, 112, 113, 114,
	 115, 64, 65, 66, 67, 26, 172, 160, 161, 162, 163, 80, 81, 82, 83, 84, 42,
	 128, 129, 130, 131, 96, 97, 98, 99, 173, 174, 144, 145, 146, 147, 176,
	 175},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 132, 16, 17, 18, 19, 20, 21, 22, 23, 24, 116,
	 32, 33, 34, 35, 36, 37, 38, 39, 40, 176, 48, 49, 50, 51, 52, 164, 112, 113,
	 114, 115, 64, 65, 66, 67, 68, 172, 160, 161, 162, 163, 80, 81, 82, 83, 84,
	 173, 128, 129, 130, 131, 96, 97, 98, 99, 100, 174, 144, 145, 146, 147, 148,
	 175},
	{0, 1, 2, 3, 4, 5, 6, 7, 164, 132, 16, 17, 18, 19, 20, 21, 22, 23, 174, 116,
	 32, 33, 34, 35, 36, 37, 38, 39, 175, 176, 48, 49, 50, 51, 52, 53, 112, 113,
	 114, 115, 64, 65, 66, 67, 68, 172, 160, 161, 162, 163, 80, 81, 82, 83, 84,
	 173, 128, 129, 130, 131, 96, 97, 98, 99, 100, 101, 144, 145, 146, 147, 148,
	 149},
	{0, 1, 2, 3, 4, 5, 6, 7, 172, 132, 16, 17, 18, 19, 20, 21, 22, 23, 117, 116,
	 32, 33, 34, 35, 36, 37, 38, 39, 165, 176, 48, 49, 50, 51, 52, 164, 112,
	 113, 114, 115, 64, 65, 66, 67, 68, 69, 160, 161, 162, 163, 80, 81, 82, 83,
	 84, 173, 128, 129, 130, 131, 96, 97, 98, 99, 100, 174, 144, 145, 146, 147,
	 148, 175},
	{0, 1, 2, 3, 4, 5, 6, 7, 173, 132, 16, 17, 18, 19, 20, 21, 22, 23, 133, 116,
	 32, 33, 34, 35, 36, 37, 38, 39, 177, 176, 48, 49, 50, 51, 52, 164, 112,
	 113, 114, 115, 64, 65, 66, 67, 68, 172, 160, 161, 162, 163, 80, 81, 82, 83,
	 84, 85, 128, 129, 130, 131, 96, 97, 98, 99, 100, 174, 144, 145, 146, 147,
	 148, 175},
	{0, 1, 2, 3, 4, 5, 164, 172, 173, 132, 16, 17, 18, 19, 20, 21, 117, 133,
	 174, 116, 32, 33, 34, 35, 36, 37, 165, 175, 177, 176, 48, 49, 50, 51, 52,
	 53, 112, 113, 114, 115, 64, 65, 66, 67, 68, 69, 160, 161, 162, 163, 80, 81,
	 82, 83, 84, 85, 128, 129, 130, 131, 96, 97, 98, 99, 100, 101, 144, 145,
	 146, 147, 148, 149},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 32,
	 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
	 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 80, 81, 82, 83, 84, 85, 86, 87, 88,
	 89},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 32,
	 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 50, 51, 52, 53, 54, 55, 56, 10,
	 64, 65, 66, 67, 68, 69, 70, 71, 72, 26, 80, 81, 82, 83, 84, 85, 86, 87, 88,
	 42},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 32,
	 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 50, 51, 52, 53, 54, 55, 11, 10,
	 64, 65, 66, 67, 68, 69, 70, 71, 27, 26, 80, 81, 82, 83, 84, 85, 86, 87, 43,
	 42},
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 32,
	 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 50, 51, 15, 14, 13, 12, 11, 10,
	 64, 65, 66, 67, 31, 30, 29, 28, 27, 26, 80, 81, 82, 83, 47, 46, 45, 44, 43,
	 42}};

static void bc6_sign_extend(uint16_t *v, int prec) {
	int x = *v;
	if (x & (1 << (prec - 1))) {
		x |= -1 << prec;
	}
	*v = (uint16_t)x;
}

static int bc6_unquantize(uint16_t v, int prec, int sign) {
	int s = 0;
	int x;
	if (!sign) {
		x = v;
		if (prec >= 15) return x;
		if (x == 0) return 0;
		if (x == ((1 << prec) - 1)) {
			return 0xffff;
		}
		return ((x << 15) + 0x4000) >> (prec - 1);
	} else {
		x = (int16_t)v;
		if (prec >= 16) return x;
		if (x < 0) {
			s = 1;
			x = -x;
		}

		if (x != 0) {
			if (x >= ((1 << (prec - 1)) - 1)) {
				x = 0x7fff;
			} else {
				x = ((x << 15) + 0x4000) >> (prec - 1);
			}
		}

		if (s) {
			return -x;
		}
		return x;
	}
}

static float half_to_float(uint16_t h) {
	/* https://gist.github.com/rygorous/2144712 */
	union {
		uint32_t u;
		float f;
	} o, m;
	m.u = 0x77800000;
	o.u = (h & 0x7fff) << 13;
	o.f *= m.f;
	m.u = 0x47800000;
	if (o.f >= m.f) {
		o.u |= 255 << 23;
	}
	o.u |= (h & 0x8000) << 16;
	return o.f;
}

static float bc6_finalize(int v, int sign) {
	if (sign) {
		if (v < 0) {
			v = ((-v) * 31) / 32;
			return half_to_float((uint16_t)(0x8000 | v));
		} else {
			return half_to_float((uint16_t)((v * 31) / 32));
		}
	} else {
		return half_to_float((uint16_t)((v * 31) / 64));
	}
}

static void bc6_lerp(rgb32f *col, int *e0, int *e1, int s, int sign) {
	int r, g, b;
	int t = 64 - s;
	r = (e0[0] * t + e1[0] * s) >> 6;
	g = (e0[1] * t + e1[1] * s) >> 6;
	b = (e0[2] * t + e1[2] * s) >> 6;
	col->r = bc6_finalize(r, sign);
	col->g = bc6_finalize(g, sign);
	col->b = bc6_finalize(b, sign);
}

static void decode_bc6_block(rgb32f *col, const uint8_t* src, int sign) {
	uint16_t endpoints[12]; /* storage for r0, g0, b0, r1, ... */
	int ueps[12];
	int i, i0, ib2, di, dw, mask, numep, s;
	uint8_t partition;
	const bc6_mode_info *info;
	const char *cw;
	int bit = 5;
	int epbits = 75;
	int ib = 3;
	int mode = src[0] & 0x1f;
	if ((mode & 3) == 0 || (mode & 3) == 1) {
		mode &= 3;
		bit = 2;
	} else if ((mode & 3) == 2) {
		mode = 2 + (mode >> 2);
		epbits = 72;
	} else {
		mode = 10 + (mode >> 2);
		epbits = 60;
		ib = 4;
	}
	if (mode >= 14) {
		/* invalid block */
		memset(col, 0, 16 * sizeof(col[0]));
		return;
	}
	info = &bc6_modes[mode];
	cw = bc7_get_weights(ib);
	numep = info->ns == 2 ? 12 : 6;
	for (i = 0; i < 12; i++) {
		endpoints[i] = 0;
	}
	for (i = 0; i < epbits; i++) {
		di = bc6_bit_packings[mode][i];
		dw = di >> 4;
		di &= 15;
		endpoints[dw] |= (uint16_t)get_bit(src, bit + i) << di;
	}
	bit += epbits;
	partition = get_bits(src, bit, info->pb);
	bit += info->pb;
	mask = (1 << info->epb) - 1;
	if (sign) { /* sign-extend e0 if signed */
		bc6_sign_extend(&endpoints[0], info->epb);
		bc6_sign_extend(&endpoints[1], info->epb);
		bc6_sign_extend(&endpoints[2], info->epb);
	}
	if (sign || info->tr) { /* sign-extend e1,2,3 if signed or deltas */
		for (i = 3; i < numep; i += 3) {
			bc6_sign_extend(&endpoints[i+0], info->rb);
			bc6_sign_extend(&endpoints[i+1], info->gb);
			bc6_sign_extend(&endpoints[i+2], info->bb);
		}
	}
	if (info->tr) { /* apply deltas */
		for (i = 3; i < numep; i++) {
			endpoints[i] = (endpoints[i] + endpoints[0]) & mask;
		}
		if (sign) {
			for (i = 3; i < numep; i += 3) {
				bc6_sign_extend(&endpoints[i+0], info->rb);
				bc6_sign_extend(&endpoints[i+1], info->gb);
				bc6_sign_extend(&endpoints[i+2], info->bb);
			}
		}
	}
	for (i = 0; i < numep; i++) {
		ueps[i] = bc6_unquantize(endpoints[i], info->epb, sign);
	}
	for (i = 0; i < 16; i++) {
		s = bc7_get_subset(info->ns, partition, i) * 6;
		ib2 = ib;
		if (i == 0) {
			ib2--;
		} else if (info->ns == 2) {
			if (i == bc7_ai0[partition]) {
				ib2--;
			}
		}
		i0 = get_bits(src, bit, ib2);
		bit += ib2;

		bc6_lerp(&col[i], &ueps[s], &ueps[s+3], cw[i0], sign);
	}
}

typedef struct {
	uint8_t * dst;		// destination buffer
	int width, height;  // destination size
	int xoff, yoff;		// destination offset
	int x, y;			// pixel to be written
	int8_t ystep;		// flip on the y-axis if ystep < 0
} DecoderState;

static void put_block(DecoderState *state, const uint8_t *col, int sz, int C) {
	int width = state->width;
	int height = state->height;
	int xmax = width + state->xoff;
	int ymax = height + state->yoff;
	int j, i, y, x;
	uint8_t *dst;
	for (j = 0; j < 4; j++) {
		y = state->y + j;
		if (C) {
			if (y >= height) {
				continue;
			}
			if (state->ystep < 0) {
				y = state->yoff + ymax - y - 1;
			}
			dst = state->dst + sz * (width * y);
			for (i = 0; i < 4; i++) {
				x = state->x + i;
				if (x >= width) {
					continue;
				}
				memcpy(dst + sz * x, col + sz * (j * 4 + i), sz);
			}
		} else {
			if (state->ystep < 0) {
				y = state->yoff + ymax - y - 1;
			}
			x = state->x;
			dst = state->dst + sz * (width * y + x);
			memcpy(dst, col + sz * (j * 4), 4 * sz);
		}
	}
	state->x += 4;
	if (state->x >= xmax) {
		state->y += 4;
		state->x = state->xoff;
	}
}

static int decode_bcn(DecoderState *state, const uint8_t *src, int src_size, int bcn, int sign) {
	int c = (state->width & 3) | (state->height & 3) ? 1 : 0;
	int ymax = state->height + state->yoff;
	const uint8_t *ptr = src;
	switch (bcn) {
#define DECODE_LOOP(NN, SZ, TY)											\
	case NN:															\
		while (src_size >= SZ) {										\
			TY col[16];													\
			memset(col, 0, 16 * sizeof(col[0]));						\
			decode_bc##NN##_block(col, ptr);							\
			put_block(state, (const uint8_t *)col, sizeof(col[0]), c);	\
			ptr += SZ;													\
			src_size -= SZ;												\
			if (state->y >= ymax) return -1;							\
		}																\
		break
	DECODE_LOOP(1, 8, rgba);
	DECODE_LOOP(2, 16, rgba);
	DECODE_LOOP(3, 16, rgba);
	DECODE_LOOP(4, 8, lum);
	DECODE_LOOP(5, 16, rgba);
	case 6:
		while (src_size >= 16) {
			rgb32f col[16];
			decode_bc6_block(col, ptr, sign);
			put_block(state, (const uint8_t *)col, sizeof(col[0]), c);
			ptr += 16;
			src_size -= 16;
			if (state->y >= ymax) return -1;
		}
		break;
	DECODE_LOOP(7, 16, rgba);
#undef DECODE_LOOP
	}
	return (int)(ptr - src);
}

int BcnDecode(
	void *dst, int dst_size,
	const void *src, int src_size,
	int width, int height,
	int bcn, int sign, int yflip)
{
	if (width == 0 || height == 0)
		return 0;

	if (dst_size < 4 * width * height)
		return -1;

	if (bcn < 1 || bcn > 6)
		return -1;

	DecoderState state = {0};
	state.width = width;
	state.height = height;
	state.dst = (uint8_t*)dst;
	state.ystep = yflip ? -1 : 1;

	return decode_bcn(&state, (const uint8_t*)src, src_size, bcn, sign);
}
