
#include <stdint.h>

typedef unsigned int u32;
typedef unsigned short u16;

typedef int s32;
typedef short s16;

static inline void sync_before_read(void *p, u32 len)
{
	u32 a, b;

	a = (u32)p & ~0x1f;
	b = ((u32)p + len + 0x1f) & ~0x1f;

	for ( ; a < b; a += 32)
		asm("dcbi 0,%0" : : "b"(a));

	asm("sync ; isync");
}

static inline void sync_after_write(const void *p, u32 len)
{
	u32 a, b;

	a = (u32)p & ~0x1f;
	b = ((u32)p + len + 0x1f) & ~0x1f;

	for ( ; a < b; a += 32)
		asm("dcbst 0,%0" : : "b"(a));

	asm("sync ; isync");
}

static inline u32 virt_to_phys(const void *p)
{
	return (u32)p & 0x7fffffff;
}

static inline void *phys_to_virt(u32 x)
{
	return (void *)(x | 0x80000000);
}

static inline u16 read16(u32 addr)
{
	u16 x;
	asm volatile("lhz %0,0(%1) ; sync" : "=r"(x) : "b"(0xc0000000 | addr));
	return x;
}

static inline u32 read32(u32 addr)
{
	u32 x;
	asm volatile("lwz %0,0(%1) ; sync" : "=r"(x) : "b"(0xc0000000 | addr));
	return x;
}

static inline void write16(u32 addr, u16 x)
{
	asm("sth %0,0(%1) ; eieio" : : "r"(x), "b"(0xc0000000 | addr));
}

static inline void write32(u32 addr, u32 x)
{
	asm("stw %0,0(%1) ; eieio" : : "r"(x), "b"(0xc0000000 | addr));
}

static inline void patch16(u32 addr, u32 x)
{
	write16(addr, x);
	sync_after_write((const void*)addr, 2);
}

static inline void patch32(u32 addr, u32 x)
{
	write32(addr, x);
	sync_after_write((const void*)addr, 4);
}

// https://wiibrew.org/wiki/Hollywood/Registers
#define MEM_PROT 0xd8b420a

typedef struct Patch
{
	u32 addr;
	u32 oldVal;
	u32 newVal;
} Patch;

static const Patch v97  = { 0x93A11304, 0x428BD001, 0x428BE001 };
static const Patch v98  = { 0x93A112F0, 0x428BD001, 0x428BE001 };
static const Patch v99  = { 0x939B66E4, 0x00014000, 0x7ED40000 };
static const Patch v100 = { 0x939B65F4, 0x00014000, 0x7ED40000 };
static const Patch v101 = { 0x939B6640, 0x00014000, 0x7ED40000 };
static const Patch v102 = { 0x939B636C, 0x00014000, 0x7ED40000 };

static const Patch v103[17] =
{
	{ 0x93A75624, 0xE0002007, 0xE0002000 },
	{ 0x939F0E74, 0xD123684B, 0x2803684B },
	{ 0x939F0EEC, 0xD121684B, 0x2803684B },
	{ 0x939F52E4, 0xD0102900, 0xE0102900 },
	{ 0x939F5324, 0xD4014C43, 0xE0014C43 },
	{ 0x939F5348, 0xD4014C3A, 0xE0014C3A },
	{ 0x939F5450, 0xD0032900, 0x46C02900 },
	{ 0x939F5454, 0xDB01290F, 0x46C0290F },
	{ 0x939F5458, 0xDD01480B, 0xE001480B },
	{ 0x939F54A0, 0xD0032900, 0x46C02900 },
	{ 0x939F54A4, 0xDB01290F, 0x46C0290F },
	{ 0x939F54A8, 0xDD01480B, 0xE001480D },
	{ 0x939F57BC, 0x42A3D12A, 0x42A346C0 },
	{ 0x939F2CB8, 0xD2014E56, 0xE0014E56 },
	{ 0x939F8560, 0x4299D800, 0x4299E000 },
	{ 0x00000000, 0x00000000, 0x00000000 },
	{ 0x00000000, 0x00000000, 0x00000000 }
};

static const Patch v104[17] =
{
	{ 0x93A75624, 0xE0002007, 0xE0002000 },
	{ 0x939F0E74, 0xD123684B, 0x2803684B },
	{ 0x939F0EEC, 0xD121684B, 0x2803684B },
	{ 0x939F5290, 0xD0102900, 0xE0102900 },
	{ 0x939F52D0, 0xD4014C43, 0xE0014C43 },
	{ 0x939F52F4, 0xD4014C3A, 0xE0014C3A },
	{ 0x939F53FC, 0xD0032900, 0x46C02900 },
	{ 0x939F5400, 0xDB01290F, 0x46C0290F },
	{ 0x939F5404, 0xDD01480B, 0xE001480B },
	{ 0x939F5498, 0xD0032900, 0x46C02900 },
	{ 0x939F549C, 0xDB01290F, 0x46C0290F },
	{ 0x939F54A0, 0xDD01480D, 0xE001480D },
	{ 0x939F5768, 0x42A3D12A, 0x42A3D12A },
	{ 0x939F2C74, 0xD2014E56, 0xE0014E56 },
	{ 0x939F8498, 0x4299D800, 0x4299E000 },
	{ 0x939F650C, 0xD0004803, 0xE0004803 },
	{ 0x00000000, 0x00000000, 0x00000000 }
};

static const Patch v105[17] =
{
	{ 0x93A754F8, 0xE0002007, 0xE0002000 },
	{ 0x93A756A4, 0xE0002007, 0xE0002000 },
	{ 0x939F0DA4, 0xD123684B, 0x2803684B },
	{ 0x939F0E1C, 0xD121684B, 0x2803684B },
	{ 0x939F4D60, 0xD0102900, 0xE0102900 },
	{ 0x939F4DA0, 0xD4014C43, 0xE0014C43 },
	{ 0x939F4DC4, 0xD4014C3A, 0xE0014C3A },
	{ 0x939F4ECC, 0xD0032900, 0x46C02900 },
	{ 0x939F4ED0, 0xDB01290F, 0x46C0290F },
	{ 0x939F4ED4, 0xDD01480B, 0xE001480B },
	{ 0x939F4F68, 0xD0032900, 0x46C02900 },
	{ 0x939F4F6C, 0xDB01290F, 0x46C0290F },
	{ 0x939F4F70, 0xDD01480D, 0xE001480D },
	{ 0x939F5238, 0x42A3D12A, 0x42A346C0 },
	{ 0x939F2800, 0xD2014E56, 0xE0014E56 },
	{ 0x939F7B30, 0x4299D800, 0x4299E000 },
	{ 0x939F5FD0, 0xD0004803, 0xE0004803 }
};

static const Patch v106[17] =
{
	{ 0x93A752E4, 0xE0002007, 0xE0002000 },
	{ 0x939F0D44, 0x2803D123, 0x28032803 },
	{ 0x939F0DBC, 0x2803D121, 0x28032803 },
	{ 0x939F4DF4, 0x429DD003, 0x429DE003 },
	{ 0x939F4E18, 0x07DAD401, 0x07DAE001 },
	{ 0x939F4E3C, 0x07D8D401, 0x07D8E001 },
	{ 0x939F4F38, 0xD0032900, 0x46C02900 },
	{ 0x939F4F3C, 0xDB01290F, 0x46C0290F },
	{ 0x939F4F40, 0xDD01480B, 0xE001480B },
	{ 0x939F4F88, 0xD0032900, 0x46C02900 },
	{ 0x939F4F8C, 0xDB01290F, 0x46C0290F },
	{ 0x939F4F90, 0xDD01480B, 0xE001480B },
	{ 0x939F52A4, 0x42A3D12A, 0x42A346C0 },
	{ 0x939F2818, 0xD2014E56, 0xE0014E56 },
	{ 0x939F7BA8, 0x4299D800, 0x4299E000 },
	{ 0x00000000, 0x00000000, 0x00000000 },
	{ 0x00000000, 0x00000000, 0x00000000 }
};

static inline s32 DoPatches(s32 enablePatches)
{
	const Patch *v7;
	switch (*(u32*)0x939B0040)
	{
	case 0x20207DB8:
		v7 = &v100;
		break;
	case 0x20207EA8:
		v7 = &v99;
		break;
	case 0x20207F40:
		v7 = &v101;
		break;
	case 0x20207C2C:
		v7 = &v102;
		break;
	default:
		return -199;
	}

	const Patch *v9;
	switch (*(u32*)0x93A10044)
	{
	case 0x20005D89:
		v9 = &v98;
		break;
	case 0x20006009:
		v9 = &v97;
		break;
	default:
		return -200;
	}

	const Patch *v5;
	switch (*(u32*)0x939F0044)
	{
	case 0x201014D5:
		v5 = v105;
		break;
	case 0x201015A5:
		v5 = v104;
		break;
	case 0x201015E9:
		v5 = v103;
		break;
	case 0x2010142D:
		v5 = v106;
		break;
	default:
		return -198;
	}

	if (enablePatches)
	{
		if (read32(v9->addr) != v9->oldVal)
		{
			return -203;
		}

		for (int i = 0; i < 17; ++i)
		{
			if (v5[i].addr && (read32(v5[i].addr) != v5[i].oldVal))
			{
				return -201;
			}
		}

		if (read32(v7->addr) != v7->oldVal)
		{
			return -201;
		}

		patch32(v9->addr, v9->newVal);
		for (int i = 0; i < 17; ++i)
		{
			if (v5[i].addr)
			{
				patch32(v5[i].addr, v5[i].newVal);
			}
		}
		patch32(v7->addr, v7->newVal);
	}
	else
	{
		if (read32(v9->addr) != v9->newVal)
		{
			return -203;
		}

		for (int i = 0; i < 17; ++i)
		{
			if (v5[i].addr && (read32(v5[i].addr) != v5[i].newVal))
			{
				return -201;
			}
		}

		if (read32(v7->addr) != v7->newVal)
		{
			return -201;
		}

		patch32(v9->addr, v9->oldVal);
		for (int i = 0; i < 17; ++i)
		{
			if (v5[i].addr)
			{
				patch32(v5[i].addr, v5[i].oldVal);
			}
		}
		patch32(v7->addr, v7->oldVal);
	}

	return 1;
}

s32 MagicPatches(s32 enablePatches)
{
	const u32 oldProtect = read32((u32)phys_to_virt(MEM_PROT));
	if (!oldProtect)
	{
		return -202;
	}

	patch16((u32)phys_to_virt(MEM_PROT), 0);
	const s32 result = DoPatches(enablePatches);
	patch32((u32)phys_to_virt(MEM_PROT), oldProtect);

	return result;
}
