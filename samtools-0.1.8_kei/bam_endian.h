#ifndef BAM_ENDIAN_H
#define BAM_ENDIAN_H

#include <stdint.h>

static inline int bam_is_big_endian()
{
	long one= 1;
	return !(*((char *)(&one)));
}
static inline uint16_t bam_swap_endian_2(uint16_t v)
{
	return (uint16_t)(((v & 0x00FF00FFU) << 8) | ((v & 0xFF00FF00U) >> 8));
}
static inline void *bam_swap_endian_2p(void *x)
{
#ifdef _TARGET_KEI
	uint8_t	t = ((uint8_t*)x)[0];
	((uint8_t*)x)[0] = ((uint8_t*)x)[1];
	((uint8_t*)x)[1] = t;
#else
	*(uint16_t*)x = bam_swap_endian_2(*(uint16_t*)x);
#endif
	return x;
}
static inline uint32_t bam_swap_endian_4(uint32_t v)
{
	v = ((v & 0x0000FFFFU) << 16) | (v >> 16);
	return ((v & 0x00FF00FFU) << 8) | ((v & 0xFF00FF00U) >> 8);
}
static inline void *bam_swap_endian_4p(void *x)
{
#ifdef _TARGET_KEI
	uint8_t	t = ((uint8_t*)x)[0];
	((uint8_t*)x)[0] = ((uint8_t*)x)[3];
	((uint8_t*)x)[3] = t;
	t = ((uint8_t*)x)[1];
	((uint8_t*)x)[1] = ((uint8_t*)x)[2];
	((uint8_t*)x)[2] = t;
#else
	*(uint32_t*)x = bam_swap_endian_4(*(uint32_t*)x);
#endif
	return x;
}
static inline uint64_t bam_swap_endian_8(uint64_t v)
{
	v = ((v & 0x00000000FFFFFFFFLLU) << 32) | (v >> 32);
	v = ((v & 0x0000FFFF0000FFFFLLU) << 16) | ((v & 0xFFFF0000FFFF0000LLU) >> 16);
	return ((v & 0x00FF00FF00FF00FFLLU) << 8) | ((v & 0xFF00FF00FF00FF00LLU) >> 8);
}
static inline void *bam_swap_endian_8p(void *x)
{
#ifdef _TARGET_KEI
	uint8_t	t = ((uint8_t*)x)[0];
	((uint8_t*)x)[0] = ((uint8_t*)x)[7];
	((uint8_t*)x)[7] = t;
	t = ((uint8_t*)x)[1];
	((uint8_t*)x)[1] = ((uint8_t*)x)[6];
	((uint8_t*)x)[6] = t;
	t = ((uint8_t*)x)[2];
	((uint8_t*)x)[2] = ((uint8_t*)x)[5];
	((uint8_t*)x)[5] = t;
	t = ((uint8_t*)x)[3];
	((uint8_t*)x)[3] = ((uint8_t*)x)[4];
	((uint8_t*)x)[4] = t;
#else
	*(uint64_t*)x = bam_swap_endian_8(*(uint64_t*)x);
#endif
	return x;
}

#endif
