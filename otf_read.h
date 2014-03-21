#ifndef OTF_READ_H
#define OTF_READ_H

#include <inttypes.h>

static inline uint8_t
get_u8(uint8_t **pp)
{
	uint8_t *p = *pp;
	uint8_t r = p[0];
	p++;
	*pp = p;
	return r;
}
static inline uint16_t
get_u16(uint8_t **pp)
{
	uint8_t *p = *pp;
	uint16_t r = (p[0]<<8) | p[1];
	p += 2;
	*pp = p;
	return r;
}
static inline int16_t
get_s16(uint8_t **pp)
{
	return (int16_t) get_u16(pp);
}
static inline uint32_t
get_u24(uint8_t **pp)
{
	uint8_t *p = *pp;
	uint32_t r = (p[0]<<16) | (p[1]<<8) | p[2];
	p += 3;
	*pp = p;
	return r;
}
static inline int32_t
get_s24(uint8_t **pp)
{
	/* TODO fix */
	return (int32_t) get_u24(pp);
}
static inline uint32_t
get_u32(uint8_t **pp)
{
	uint8_t *p = *pp;
	uint32_t r = (p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
	p += 4;
	*pp = p;
	return r;
}
static inline int32_t
get_s32(uint8_t **pp)
{
	return (int32_t) get_u32(pp);
}
static inline uint32_t
get_n_(uint8_t **pp, int n, char *file, int line)
{
	uint32_t r;
	switch(n) {
	case 1: r = get_u8 (pp); break;
	case 2: r = get_u16(pp); break;
	case 3: r = get_u24(pp); break;
	case 4: r = get_u32(pp); break;
	default:r = 0; fprintf(stderr, "%s(%d) bad [%s:%d]\n", __func__, n, file, line); exit(-1);
	}
	return r;
}
#define get_n(a,b) get_n_(a,b,__FILE__,__LINE__)

#endif /* OTF_READ_H */
