 /*
  * UAE - The Un*x Amiga Emulator
  *
  * ROM file management
  *
  */ 

#include "sysconfig.h"
#include "sysdeps.h"

#include "options.h"
#include "uae.h"
#include "gui.h"
#include "uae/memory.h"
#include "rommgr.h"
#include "zfile.h"
#include "crc32.h"
#include "fsdb.h"
#include "autoconf.h"
#include "filesys.h"

#ifdef FSUAE // NL
#include "uae/fs.h"
#endif

#define SAVE_ROM 0

static struct romlist *rl;
static int romlist_cnt;

struct romlist *romlist_getit (void)
{
	return rl;
}

int romlist_count (void)
{
	return romlist_cnt;
}

TCHAR *romlist_get (const struct romdata *rd)
{
	int i;

	if (!rd)
		return 0;
	for (i = 0; i < romlist_cnt; i++) {
		if (rl[i].rd->id == rd->id)
			return rl[i].path;
	}
	return 0;
}

static struct romlist *romlist_getrl (const struct romdata *rd)
{
	int i;

	if (!rd)
		return 0;
	for (i = 0; i < romlist_cnt; i++) {
		if (rl[i].rd == rd)
			return &rl[i];
	}
	return 0;
}

static void romlist_cleanup (void);
void romlist_add (const TCHAR *path, struct romdata *rd)
{
	struct romlist *rl2;

	if (path == NULL || rd == NULL) {
		romlist_cleanup ();
		return;
	}
	romlist_cnt++;
	rl = xrealloc (struct romlist, rl, romlist_cnt);
	rl2 = rl + romlist_cnt - 1;
	rl2->path = my_strdup (path);
	rl2->rd = rd;
	struct romdata *rd2 = getromdatabyid (rd->id);
	if (rd2 != rd && rd2) // replace "X" with parent name
		rd->name = rd2->name;
}


struct romdata *getromdatabypath (const TCHAR *path)
{
	int i;
	for (i = 0; i < romlist_cnt; i++) {
		struct romdata *rd = rl[i].rd;
		if (rd->configname && path[0] == ':') {
			if (!_tcscmp(path + 1, rd->configname))
				return rd;
		}
		if (my_issamepath(rl[i].path, path))
			return rl[i].rd;
	}
	return NULL;
}

#define NEXT_ROM_ID 142

#define ALTROM(id,grp,num,size,flags,crc32,a,b,c,d,e) \
{ _T("X"), 0, 0, 0, 0, 0, size, id, 0, 0, flags, (grp << 16) | num, 0, NULL, crc32, a, b, c, d, e },
#define ALTROMPN(id,grp,num,size,flags,pn,crc32,a,b,c,d,e) \
{ _T("X"), 0, 0, 0, 0, 0, size, id, 0, 0, flags, (grp << 16) | num, 0, pn, crc32, a, b, c, d, e },

static struct romdata roms[] = {
	{ _T(" AROS KS ROM (built-in)"), 0, 0, 0, 0, _T("AROS\0"), 524288 * 2, 66, 0, 0, ROMTYPE_KICK, 0, 0, NULL,
	0xffffffff, 0, 0, 0, 0, 0, _T("AROS") },
	{ _T(" ROM Disabled"), 0, 0, 0, 0, _T("NOROM\0"), 0, 87, 0, 0, ROMTYPE_NONE, 0, 0, NULL,
	0xffffffff, 0, 0, 0, 0, 0, _T("NOROM") },

	{ _T("Cloanto Amiga Forever ROM key"), 0, 0, 0, 0, 0, 2069, 0, 0, 1, ROMTYPE_KEY, 0, 0, NULL,
	0x869ae1b1, 0x801bbab3,0x2e3d3738,0x6dd1636d,0x4f1d6fa7,0xe21d5874 },
	{ _T("Cloanto Amiga Forever 2006 ROM key"), 0, 0, 0, 0, 0, 750, 48, 0, 1, ROMTYPE_KEY, 0, 0, NULL,
	0xb01c4b56, 0xbba8e5cd,0x118b8d92,0xafed5693,0x5eeb9770,0x2a662d8f },
	{ _T("Cloanto Amiga Forever 2010 ROM key"), 0, 0, 0, 0, 0, 1544, 73, 0, 1, ROMTYPE_KEY, 0, 0, NULL,
	0x8c4dd05c, 0x05034f62,0x0b5bb7b2,0x86954ea9,0x164fdb90,0xfb2897a4 },

	{ _T("KS ROM Velvet 23.93"), 23, 93, 23, 93, _T("VELVET\0"), 131072, 125, 0, 0, ROMTYPE_KICK, 0, 0, NULL,
	0xadcb44c9, 0x7c36b2ba,0x298da3da,0xce60d0ba,0x8511d470,0x76a40d5c, NULL, NULL },
	ALTROMPN(125, 1, 1, 32768, ROMTYPE_QUAD | ROMTYPE_EVEN | ROMTYPE_8BIT, NULL, 0x1d988ab8, 0xee3988a2, 0xb2693334, 0x0239d1d9, 0xf50d4fb3, 0xe0daf3bc)
	ALTROMPN(125, 1, 2, 32768, ROMTYPE_QUAD | ROMTYPE_ODD  | ROMTYPE_8BIT, NULL, 0xe466b28f, 0x3e197d69, 0xcffa3e1a, 0x0c291d57, 0xb53f7d1f, 0xcb858cf7)
	ALTROMPN(125, 1, 3, 32768, ROMTYPE_QUAD | ROMTYPE_EVEN | ROMTYPE_8BIT, NULL, 0x715988a9, 0x08c36600, 0x3948c4c5, 0x4216ef8c, 0x17ebe16c, 0xc91d3b7a)
	ALTROMPN(125, 1, 4, 32768, ROMTYPE_QUAD | ROMTYPE_ODD  | ROMTYPE_8BIT, NULL, 0xc4dc7e6a, 0x66b231d0, 0x8425c858, 0xdfcd36d2, 0xd38a0df8, 0x518e06a4)

	{ _T("KS ROM v1.0 (A1000)(NTSC)"), 1, 0, 1, 0, _T("A1000\0"), 262144, 1, 0, 0, ROMTYPE_KICK, 0, 0, NULL,
	0x299790ff, 0x00C15406,0xBEB4B8AB,0x1A16AA66,0xC05860E1,0xA7C1AD79 },
	{ _T("KS ROM v1.1 (A1000)(NTSC)"), 1, 1, 31, 34, _T("A1000\0"), 262144, 2, 0, 0, ROMTYPE_KICK, 0, 0, NULL,
	0xd060572a, 0x4192C505,0xD130F446,0xB2ADA6BD,0xC91DAE73,0x0ACAFB4C},
	{ _T("KS ROM v1.1 (A1000)(PAL)"), 1, 1, 31, 34, _T("A1000\0"), 262144, 3, 0, 0, ROMTYPE_KICK, 0, 0, NULL,
	0xec86dae2, 0x16DF8B5F,0xD524C5A1,0xC7584B24,0x57AC15AF,0xF9E3AD6D },
	{ _T("KS ROM v1.2 (A1000)"), 1, 2, 33, 166, _T("A1000\0"), 262144, 4, 0, 0, ROMTYPE_KICK, 0, 0, NULL,
	0x9ed783d0, 0x6A7BFB5D,0xBD6B8F17,0x9F03DA84,0xD8D95282,0x67B6273B },
	{ _T("KS ROM v1.2 (A500,A1000,A2000)"), 1, 2, 33, 180, _T("A500\0A1000\0A2000\0"), 262144, 5, 0, 0, ROMTYPE_KICK, 0, 0, _T("315093-01"),
	0xa6ce1636, 0x11F9E62C,0xF299F721,0x84835B7B,0x2A70A163,0x33FC0D88 },
	{ _T("KS ROM v1.3 (A500,A1000,A2000)"), 1, 3, 34, 5, _T("A500\0A1000\0A2000\0"), 262144, 6, 0, 0, ROMTYPE_KICK, 0, 0, _T("315093-02"),
	0xc4f0f55f, 0x891E9A54,0x7772FE0C,0x6C19B610,0xBAF8BC4E,0xA7FCB785 },
	{ _T("KS ROM v1.3 (A3000)(SK)"), 1, 3, 34, 5, _T("A3000\0"), 262144, 32, 0, 0, ROMTYPE_KICK, 0, 0, NULL,
	0xe0f37258, 0xC39BD909,0x4D4E5F4E,0x28C1411F,0x30869504,0x06062E87 },
	{ _T("KS ROM v1.4 (A3000)"), 1, 4, 36, 16, _T("A3000\0"), 524288, 59, 3, 0, ROMTYPE_KICK, 0, 0, NULL,
	0xbc0ec13f, 0xF76316BF,0x36DFF14B,0x20FA349E,0xD02E4B11,0xDD932B07 },
	ALTROMPN(59, 1, 1, 262144, ROMTYPE_EVEN, _T("390629-02"), 0x58327536,0xd1713d7f,0x31474a59,0x48e6d488,0xe3368606,0x1cf3d1e2)
	ALTROMPN(59, 1, 2, 262144, ROMTYPE_ODD , _T("390630-02"), 0xfe2f7fb9,0xc05c9c52,0xd014c66f,0x9019152b,0x3f2a2adc,0x2c678794)
	{ _T("KS ROM v2.04 (A500+)"), 2, 4, 37, 175, _T("A500+\0"), 524288, 7, 0, 0, ROMTYPE_KICK, 0, 0, _T("390979-01"),
	0xc3bdb240, 0xC5839F5C,0xB98A7A89,0x47065C3E,0xD2F14F5F,0x42E334A1 },
	{ _T("KS ROM v2.05 (A600)"), 2, 5, 37, 299, _T("A600\0"), 524288, 8, 0, 0, ROMTYPE_KICK, 0, 0, _T("391388-01"),
	0x83028fb5, 0x87508DE8,0x34DC7EB4,0x7359CEDE,0x72D2E3C8,0xA2E5D8DB },
	{ _T("KS ROM v2.05 (A600HD)"), 2, 5, 37, 300, _T("A600HD\0A600\0"), 524288, 9, 0, 0, ROMTYPE_KICK, 0, 0, _T("391304-01"),
	0x64466c2a, 0xF72D8914,0x8DAC39C6,0x96E30B10,0x859EBC85,0x9226637B },
	{ _T("KS ROM v2.05 (A600HD)"), 2, 5, 37, 350, _T("A600HD\0A600\0"), 524288, 10, 0, 0, ROMTYPE_KICK, 0, 0, _T("391304-02"),
	0x43b0df7b, 0x02843C42,0x53BBD29A,0xBA535B0A,0xA3BD9A85,0x034ECDE4 },
	{ _T("KS ROM v2.04 (A3000)"), 2, 4, 37, 175, _T("A3000\0"), 524288, 71, 8, 0, ROMTYPE_KICK, 0, 0, NULL,
	0x234a7233, 0xd82ebb59,0xafc53540,0xddf2d718,0x7ecf239b,0x7ea91590 },
	ALTROMPN(71, 1, 1, 262144, ROMTYPE_EVEN, _T("390629-03"), 0xa245dbdf,0x83bab8e9,0x5d378b55,0xb0c6ae65,0x61385a96,0xf638598f)
	ALTROMPN(71, 1, 2, 262144, ROMTYPE_ODD , _T("390630-03"), 0x7db1332b,0x48f14b31,0x279da675,0x7848df6f,0xeb531881,0x8f8f576c)

	{ _T("KS ROM v3.0 (A1200)"), 3, 0, 39, 106, _T("A1200\0"), 524288, 11, 0, 0, ROMTYPE_KICK, 0, 0, NULL,
	0x6c9b07d2, 0x70033828,0x182FFFC7,0xED106E53,0x73A8B89D,0xDA76FAA5 },
	ALTROMPN(11, 1, 1, 262144, ROMTYPE_EVEN, _T("391523-01"), 0xc742a412,0x999eb81c,0x65dfd07a,0x71ee1931,0x5d99c7eb,0x858ab186)
	ALTROMPN(11, 1, 2, 262144, ROMTYPE_ODD , _T("391524-01"), 0xd55c6ec6,0x3341108d,0x3a402882,0xb5ef9d3b,0x242cbf3c,0x8ab1a3e9)
	{ _T("KS ROM v3.0 (A4000)"), 3, 0, 39, 106, _T("A4000\0"), 524288, 12, 2 | 4, 0, ROMTYPE_KICK, 0, 0, NULL,
	0x9e6ac152, 0xF0B4E9E2,0x9E12218C,0x2D5BD702,0x0E4E7852,0x97D91FD7 },
	ALTROMPN(12, 1, 1, 262144, ROMTYPE_EVEN, _T("391513-02"), 0x36f64dd0,0x196e9f3f,0x9cad934e,0x181c07da,0x33083b1f,0x0a3c702f)
	ALTROMPN(12, 1, 2, 262144, ROMTYPE_ODD , _T("391514-02"), 0x17266a55,0x42fbed34,0x53d1f11c,0xcbde89a9,0x826f2d11,0x75cca5cc)
	{ _T("KS ROM v3.1 (A4000)"), 3, 1, 40, 70, _T("A4000\0"), 524288, 13, 2 | 4, 0, ROMTYPE_KICK, 0, 0, NULL,
	0x2b4566f1, 0x81c631dd,0x096bbb31,0xd2af9029,0x9c76b774,0xdb74076c },
	ALTROM(13, 1, 1, 262144, ROMTYPE_EVEN, 0xf9cbecc9,0x138d8cb4,0x3b8312fe,0x16d69070,0xde607469,0xb3d4078e)
	ALTROM(13, 1, 2, 262144, ROMTYPE_ODD , 0xf8248355,0xc2379547,0x9fae3910,0xc185512c,0xa268b82f,0x1ae4fe05)
	{ _T("KS ROM v3.1 (A500,A600,A2000)"), 3, 1, 40, 63, _T("A500\0A600\0A2000\0"), 524288, 14, 0, 0, ROMTYPE_KICK, 0, 0, NULL,
	0xfc24ae0d, 0x3B7F1493,0xB27E2128,0x30F989F2,0x6CA76C02,0x049F09CA },
	{ _T("KS ROM v3.1 (A1200)"), 3, 1, 40, 68, _T("A1200\0"), 524288, 15, 1, 0, ROMTYPE_KICK, 0, 0, NULL,
	0x1483a091, 0xE2154572,0x3FE8374E,0x91342617,0x604F1B3D,0x703094F1 },
	ALTROMPN(15, 1, 1, 262144, ROMTYPE_EVEN, _T("391773-01"), 0x08dbf275,0xb8800f5f,0x90929810,0x9ea69690,0xb1b8523f,0xa22ddb37)
	ALTROMPN(15, 1, 2, 262144, ROMTYPE_ODD , _T("391774-01"), 0x16c07bf8,0x90e331be,0x1970b0e5,0x3f53a9b0,0x390b51b5,0x9b3869c2)
	{ _T("KS ROM v3.1 (A3000)"), 3, 1, 40, 68, _T("A3000\0"), 524288, 61, 2, 0, ROMTYPE_KICK, 0, 0, NULL,
	0xefb239cc, 0xF8E210D7,0x2B4C4853,0xE0C9B85D,0x223BA20E,0x3D1B36EE },
	ALTROM(61, 1, 1, 262144, ROMTYPE_EVEN, 0x286b9a0d,0x6763a225,0x8ec493f7,0x408cf663,0x110dae9a,0x17803ad1)
	ALTROM(61, 1, 2, 262144, ROMTYPE_ODD , 0x0b8cde6a,0x5f02e97b,0x48ebbba8,0x7d516a56,0xb0400c6f,0xc3434d8d)
	{ _T("KS ROM v3.1 (A4000)(Cloanto)"), 3, 1, 40, 68, _T("A4000\0"), 524288, 31, 2 | 4, 1, ROMTYPE_KICK, 0, 0, NULL,
	0x43b6dd22, 0xC3C48116,0x0866E60D,0x085E436A,0x24DB3617,0xFF60B5F9 },
	{ _T("KS ROM v3.1 (A4000)"), 3, 1, 40, 68, _T("A4000\0"), 524288, 16, 2 | 4, 0, ROMTYPE_KICK, 0, 0, NULL,
	0xd6bae334, 0x5FE04842,0xD04A4897,0x20F0F4BB,0x0E469481,0x99406F49 },
	ALTROM(16, 1, 1, 262144, ROMTYPE_EVEN, 0xb2af34f8,0x24e52b5e,0xfc020495,0x17387ab7,0xb1a1475f,0xc540350e)
	ALTROM(16, 1, 2, 262144, ROMTYPE_ODD , 0xe65636a3,0x313c7cbd,0xa5779e56,0xf19a41d3,0x4e760f51,0x7626d882)
	{ _T("KS ROM v3.1 (A4000T)"), 3, 1, 40, 70, _T("A4000T\0"), 524288, 17, 2 | 4, 0, ROMTYPE_KICK, 0, 0, NULL,
	0x75932c3a, 0xB0EC8B84,0xD6768321,0xE01209F1,0x1E6248F2,0xF5281A21 },
	ALTROMPN(17, 1, 1, 262144, ROMTYPE_EVEN, _T("391657-01"), 0x0ca94f70,0xb3806eda,0xcb3362fc,0x16a154ce,0x1eeec5bf,0x5bc24789)
	ALTROMPN(17, 1, 2, 262144, ROMTYPE_ODD , _T("391658-01"), 0xdfe03120,0xcd7a706c,0x431b04d8,0x7814d3a2,0xd8b39710,0x0cf44c0c)
	{ _T("KS ROM v3.X (A4000)(Cloanto)"), 3, 10, 45, 57, _T("A4000\0"), 524288, 46, 2 | 4, 0, ROMTYPE_KICK, 0, 0, NULL,
	0x3ac99edc, 0x3cbfc9e1,0xfe396360,0x157bd161,0xde74fc90,0x1abee7ec },

	{ _T("CD32 KS ROM v3.1"), 3, 1, 40, 60, _T("CD32\0"), 524288, 18, 1, 0, ROMTYPE_KICKCD32, 0, 0, NULL,
	0x1e62d4a5, 0x3525BE88,0x87F79B59,0x29E017B4,0x2380A79E,0xDFEE542D },
	{ _T("CD32 extended ROM"), 3, 1, 40, 60, _T("CD32\0"), 524288, 19, 1, 0, ROMTYPE_EXTCD32, 0, 0, NULL,
	0x87746be2, 0x5BEF3D62,0x8CE59CC0,0x2A66E6E4,0xAE0DA48F,0x60E78F7F },

	/* plain CD32 rom */
	{ _T("CD32 ROM (KS + extended)"), 3, 1, 40, 60, _T("CD32\0"), 2 * 524288, 64, 1, 0, ROMTYPE_KICKCD32 | ROMTYPE_EXTCD32 | ROMTYPE_CD32, 0, 0, NULL,
	0xf5d4f3c8, 0x9fa14825,0xc40a2475,0xa2eba5cf,0x325bd483,0xc447e7c1 },
	/* real CD32 rom dump 391640-03 */
	ALTROMPN(64, 1, 1, 2 * 524288, ROMTYPE_CD32, _T("391640-03"), 0xa4fbc94a, 0x816ce6c5,0x07787585,0x0c7d4345,0x2230a9ba,0x3a2902db )

	{ _T("CD32 Full Motion Video Cartridge ROM"), 3, 1, 40, 30, _T("CD32FMV\0"), 262144, 23, 1, 0, ROMTYPE_CD32CART, 0, 0, NULL,
	0xc35c37bf, 0x03ca81c7,0xa7b259cf,0x64bc9582,0x863eca0f,0x6529f435 },
	{ _T("CD32 Full Motion Video Cartridge ROM"), 3, 1, 40, 22, _T("CD32FMV\0"), 262144, 74, 1, 0, ROMTYPE_CD32CART, 0, 0, _T("391777-01"),
	0xf11158eb, 0x94e469a7,0x6030dcb2,0x99ebc752,0x0aaeef9d,0xb54284cf },

	{ _T("CDTV extended ROM v1.00"), 1, 0, 1, 0, _T("CDTV\0"), 262144, 20, 0, 0, ROMTYPE_EXTCDTV, 0, 0, NULL,
	0x42baa124, 0x7BA40FFA,0x17E500ED,0x9FED041F,0x3424BD81,0xD9C907BE },
	ALTROMPN(20, 1, 1, 131072, ROMTYPE_EVEN | ROMTYPE_8BIT, _T("252606-01"), 0x791cb14b,0x277a1778,0x92449635,0x3ffe56be,0x68063d2a,0x334360e4)
	ALTROMPN(20, 1, 2, 131072, ROMTYPE_ODD  | ROMTYPE_8BIT, _T("252607-01"), 0xaccbbc2e,0x41b06d16,0x79c6e693,0x3c3378b7,0x626025f7,0x641ebc5c)
	{ _T("CDTV extended ROM v2.07"), 2, 7, 2, 7, _T("CDTV\0"), 262144, 22, 0, 0, ROMTYPE_EXTCDTV, 0, 0, NULL,
	0xceae68d2, 0x5BC114BB,0xA29F60A6,0x14A31174,0x5B3E2464,0xBFA06846 },
	ALTROM(22, 1, 1, 131072, ROMTYPE_EVEN | ROMTYPE_8BIT, 0x36d73cb8,0x9574e546,0x4b390697,0xf28f9a43,0x4e604e5e,0xf5e5490a)
	ALTROM(22, 1, 2, 131072, ROMTYPE_ODD  | ROMTYPE_8BIT, 0x6e84dce7,0x01a0679e,0x895a1a0f,0x559c7253,0xf539606b,0xd447b54f)
	{ _T("CDTV/A570 extended ROM v2.30"), 2, 30, 2, 30, _T("CDTV\0"), 262144, 21, 0, 0, ROMTYPE_EXTCDTV, 0, 0, _T("391298-01"),
	0x30b54232, 0xED7E461D,0x1FFF3CDA,0x321631AE,0x42B80E3C,0xD4FA5EBB },
	ALTROM(21, 1, 1, 131072, ROMTYPE_EVEN | ROMTYPE_8BIT, 0x48e4d74f,0x54946054,0x2269e410,0x36018402,0xe1f6b855,0xfd89092b)
	ALTROM(21, 1, 2, 131072, ROMTYPE_ODD  | ROMTYPE_8BIT, 0x8a54f362,0x03df800f,0x032046fd,0x892f6e7e,0xec08b76d,0x33981e8c)
	{ _T("CDTV-CR extended ROM v3.32"), 3, 32, 3, 32, _T("CDTVCR\0"), 262144, 107, 0, 0, ROMTYPE_EXTCDTV, 0, 0, NULL,
	0x581a85cf, 0xd6b8d3f2,0x854eba9b,0x2d514579,0x9529e8b3,0x3b85e0b4 },
	{ _T("CDTV-CR extended ROM v3.44"), 3, 44, 3, 44, _T("CDTVCR\0"), 262144, 108, 0, 0, ROMTYPE_EXTCDTV, 0, 0, NULL,
	0x0b7bd64f, 0x3b160c5a,0xbe79f10a,0xe6924332,0x8004bb9e,0x3162b648 },

	{ _T("A1000 bootstrap ROM"), 0, 0, 0, 0, _T("A1000\0"), 65536, 24, 0, 0, ROMTYPE_KICK, 0, 0, NULL,
	0x0b1ad2d0, 0xBA93B8B8,0x5CA0D83A,0x68225CC3,0x3B95050D,0x72D2FDD7 },
	ALTROM(24, 1, 1, 8192,           0, 0x62f11c04, 0xC87F9FAD,0xA4EE4E69,0xF3CCA0C3,0x6193BE82,0x2B9F5FE6)
	ALTROMPN(24, 2, 1, 4096, ROMTYPE_EVEN | ROMTYPE_8BIT, _T("252179-01"), 0x42553bc4,0x8855a97f,0x7a44e3f6,0x2d1c88d9,0x38fee1f4,0xc606af5b)
	ALTROMPN(24, 2, 2, 4096, ROMTYPE_ODD  | ROMTYPE_8BIT, _T("252180-01"), 0x8e5b9a37,0xd10f1564,0xb99f5ffe,0x108fa042,0x362e877f,0x569de2c3)

	{ _T("The Diagnostic 2.0 (Logica)"), 2, 0, 2, 0, _T("LOGICA\0"), 524288, 72, 0, 0, ROMTYPE_KICK | ROMTYPE_SPECIALKICK, 0, 0, NULL,
	0x8484f426, 0xba10d161,0x66b2e2d6,0x177c979c,0x99edf846,0x2b21651e },

	{ _T("Freezer: Action Replay Mk I v1.00"), 1, 0, 1, 0, _T("AR\0"), 65536, 52, 0, 0, ROMTYPE_AR, 0, 1, NULL,
	0x2d921771, 0x1EAD9DDA,0x2DAD2914,0x6441F5EF,0x72183750,0x22E01248 },
	ALTROM(52, 1, 1, 32768, ROMTYPE_EVEN | ROMTYPE_8BIT, 0x82d6eb87, 0x7c9bac11,0x28666017,0xeee6f019,0x63fb3890,0x7fbea355)
	ALTROM(52, 1, 2, 32768, ROMTYPE_ODD  | ROMTYPE_8BIT, 0x40ae490c, 0x81d8e432,0x01b73fd9,0x2e204ebd,0x68af8602,0xb62ce397)
	{ _T("Freezer: Action Replay Mk I v1.50"), 1, 50, 1, 50, _T("AR\0"), 65536, 25, 0, 0, ROMTYPE_AR, 0, 1, NULL,
	0xf82c4258, 0x843B433B,0x2C56640E,0x045D5FDC,0x854DC6B1,0xA4964E7C },
	ALTROM(25, 1, 1, 32768, ROMTYPE_EVEN | ROMTYPE_8BIT, 0x7fbd6de2, 0xb5f71a5c,0x09d65ecc,0xa8a3bc93,0x93558461,0xca190228)
	ALTROM(25, 1, 2, 32768, ROMTYPE_ODD  | ROMTYPE_8BIT, 0x43018069, 0xad8ff242,0xb2cbf125,0x1fc53a73,0x581cf57a,0xb69cee00)
	{ _T("Freezer: Action Replay Mk II v2.05"), 2, 5, 2, 5, _T("AR\0"), 131072, 26, 0, 0, ROMTYPE_AR2, 0, 1, NULL,
	0x1287301f, 0xF6601DE8,0x888F0050,0x72BF562B,0x9F533BBC,0xAF1B0074 },
	{ _T("Freezer: Action Replay Mk II v2.12"), 2, 12, 2, 12, _T("AR\0"), 131072, 27, 0, 0, ROMTYPE_AR2, 0, 1, NULL,
	0x804d0361, 0x3194A07A,0x0A82D8B5,0xF2B6AEFA,0x3CA581D6,0x8BA8762B },
	{ _T("Freezer: Action Replay Mk II v2.14"), 2, 14, 2, 14, _T("AR\0"), 131072, 28, 0, 0, ROMTYPE_AR2, 0, 1, NULL,
	0x49650e4f, 0x255D6DF6,0x3A4EAB0A,0x838EB1A1,0x6A267B09,0x59DFF634 },
	{ _T("Freezer: Action Replay Mk III v3.09"), 3, 9, 3, 9, _T("AR\0"), 262144, 29, 0, 0, ROMTYPE_AR2, 0, 1, NULL,
	0x0ed9b5aa, 0x0FF3170A,0xBBF0CA64,0xC9DD93D6,0xEC0C7A01,0xB5436824 },
	ALTROM(29, 1, 1, 131072, ROMTYPE_EVEN | ROMTYPE_8BIT, 0x2b84519f, 0x7841873b,0xf009d834,0x1dfa2794,0xb3751bac,0xf86adcc8)
	ALTROM(29, 1, 2, 131072, ROMTYPE_ODD  | ROMTYPE_8BIT, 0x1d35bd56, 0x6464be16,0x26b51949,0x9e76e4e3,0x409e8016,0x515d48b6)
	{ _T("Freezer: Action Replay Mk III v3.17"), 3, 17, 3, 17, _T("AR\0"), 262144, 30, 0, 0, ROMTYPE_AR2, 0, 1, NULL,
	0xc8a16406, 0x5D4987C2,0xE3FFEA8B,0x1B02E314,0x30EF190F,0x2DB76542 },
	{ _T("Freezer: Action Replay 1200"), 0, 0, 0, 0, _T("AR\0"), 262144, 47, 0, 0, ROMTYPE_AR, 0, 1, NULL,
	0x8d760101, 0x0F6AB834,0x2810094A,0xC0642F62,0xBA42F78B,0xC0B07E6A },

	{ _T("Freezer: Action Cartridge Super IV Professional"), 0, 0, 0, 0, _T("SUPERIV\0"), 0, 62, 0, 0, ROMTYPE_SUPERIV, 0, 1, NULL,
	0xffffffff, 0, 0, 0, 0, 0, _T("SuperIV") },
	{ _T("Freezer: Action Cart. Super IV Pro (+ROM v4.3)"), 4, 3, 4, 3, _T("SUPERIV\0"), 170368, 60, 0, 0, ROMTYPE_SUPERIV, 0, 1, NULL,
	0xe668a0be, 0x633A6E65,0xA93580B8,0xDDB0BE9C,0x9A64D4A1,0x7D4B4801 },
	{ _T("Freezer: X-Power Professional 500 v1.2"), 1, 2, 1, 2, _T("XPOWER\0"), 131072, 65, 0, 0, ROMTYPE_XPOWER, 0, 1, NULL,
	0x9e70c231, 0xa2977a1c,0x41a8ca7d,0x4af4a168,0x726da542,0x179d5963 },
	ALTROM(65, 1, 1, 65536, ROMTYPE_EVEN|ROMTYPE_SCRAMBLED|ROMTYPE_8BIT, 0xf98742e4,0xe8e683ba,0xd8b38d1f,0x79f3ad83,0xa9e67c6f,0xa91dc96c)
	ALTROM(65, 1, 2, 65536, ROMTYPE_ODD |ROMTYPE_SCRAMBLED|ROMTYPE_8BIT, 0xdfb9984b,0x8d6bdd49,0x469ec8e2,0x0143fbb3,0x72e92500,0x99f07910)
	{ _T("Freezer: X-Power Professional 500 v1.3"), 1, 3, 1, 3, _T("XPOWER\0"), 131072, 68, 0, 0, ROMTYPE_XPOWER, 0, 1, NULL,
	0x31e057f0, 0x84650266,0x465d1859,0x7fd71dee,0x00775930,0xb7e450ee },
	ALTROM(68, 1, 1, 65536, ROMTYPE_EVEN|ROMTYPE_SCRAMBLED|ROMTYPE_8BIT, 0x0b2ce0c7,0x45ad5456,0x89192404,0x956f47ce,0xf66a5274,0x57ace33b)
	ALTROM(68, 1, 2, 65536, ROMTYPE_ODD |ROMTYPE_SCRAMBLED|ROMTYPE_8BIT, 0x34580c35,0x8ad42566,0x7364f238,0x978f4381,0x08f8d5ec,0x470e72ea)
	{ _T("Freezer: Nordic Power v1.5"), 1, 5, 1, 5, _T("NPOWER\0"), 65536, 69, 0, 0, ROMTYPE_NORDIC, 0, 1, NULL,
	0x83b4b21c, 0xc56ced25,0x506a5aab,0x3fa13813,0x4fc9e5ae,0x0f9d3709 },
	ALTROM(69, 1, 1, 32768, ROMTYPE_EVEN|ROMTYPE_SCRAMBLED|ROMTYPE_8BIT, 0xdd207174,0xae67652d,0x64f5db20,0x0f4b2110,0xee59567f,0xfbd90a1b)
	ALTROM(69, 1, 2, 32768, ROMTYPE_ODD |ROMTYPE_SCRAMBLED|ROMTYPE_8BIT, 0x8f93d85d,0x73c62d21,0x40c0c092,0x6315b702,0xdd5d0f05,0x3dad7fab)
	{ _T("Freezer: Nordic Power v2.0"), 2, 0, 2, 0, _T("NPOWER\0"), 65536, 67, 0, 0, ROMTYPE_NORDIC, 0, 1, NULL,
	0xa4db2906, 0x0aec68f7,0x25470c89,0x6b699ff4,0x6623dec5,0xc777466e },
	ALTROM(67, 1, 1, 32768, ROMTYPE_EVEN|ROMTYPE_SCRAMBLED|ROMTYPE_8BIT, 0xb21be46c,0x50dc607c,0xce976bbd,0x3841eaf0,0x591ddc7e,0xa1939ad2)
	ALTROM(67, 1, 2, 32768, ROMTYPE_ODD |ROMTYPE_SCRAMBLED|ROMTYPE_8BIT, 0x96057aed,0xdd9209e2,0x1d5edfc1,0xcdb52abe,0x93de0f35,0xc43da696)
	{ _T("Freezer: Nordic Power v3.0"), 3, 0, 3, 0, _T("NPOWER\0"), 65536, 70, 0, 0, ROMTYPE_NORDIC, 0, 1, NULL,
	0x72850aef, 0x59c91d1f,0xa8f118f9,0x0bdba05a,0x9ae788d7,0x7a6cc7c9 },
	ALTROM(70, 1, 1, 32768, ROMTYPE_EVEN|ROMTYPE_SCRAMBLED|ROMTYPE_8BIT, 0xf3330e1f,0x3a597db2,0xb7d11b6c,0xb8e13496,0xc215f223,0x88c4ca3c)
	ALTROM(70, 1, 2, 32768, ROMTYPE_ODD |ROMTYPE_SCRAMBLED|ROMTYPE_8BIT, 0xee58e0f9,0x4148f4cb,0xb42cec33,0x8ca144de,0xd4f54118,0xe0f185dd)
	{ _T("Freezer: Nordic Power v3.2"), 3, 2, 3, 2, _T("NPOWER\0"), 65536, 115, 0, 0, ROMTYPE_NORDIC, 0, 1, NULL,
	0x46158b6e, 0xd8c3f5af,0x5f109c61,0x5f6acb38,0x68fe6c06,0x580041b5 },
	ALTROM(115, 1, 1, 32768, ROMTYPE_EVEN|ROMTYPE_SCRAMBLED|ROMTYPE_8BIT, 0x4bfc71de,0x51914de0,0xdc0f055a,0x29ca188d,0xa7f61914,0xfdecbd07)
	ALTROM(115, 1, 2, 32768, ROMTYPE_ODD |ROMTYPE_SCRAMBLED|ROMTYPE_8BIT, 0x923ec443,0x9f1e5334,0xaa620745,0xf4d0c50e,0x8736543b,0x6d4366c5)
	{ _T("Freezer: Pro Access v2.17"), 2, 17, 2, 17, _T("PROACCESS\0"), 65536, 116, 0, 0, ROMTYPE_AR, 0, 1, NULL,
	0xc4c265cd, 0x6a5c0d99,0x69a624dc,0x1b437aec,0x5dbcd4c7,0x2ce9064a },
	ALTROM(116, 1, 1, 32768, ROMTYPE_EVEN | ROMTYPE_8BIT, 0x1909f7e9, 0x5abe9b9d,0xaae328c8,0x134e2b62,0x7b33b698,0xe342afc2)
	ALTROM(116, 1, 2, 32768, ROMTYPE_ODD  | ROMTYPE_8BIT, 0xa3927c72, 0x7adc9352,0x2d112ae9,0x23b9a70d,0x951b1e7a,0xba800ea6)

	{ _T("Freezer: HRTMon v2.33 (built-in)"), 0, 0, 0, 0, _T("HRTMON\0"), 0, 63, 0, 0, ROMTYPE_HRTMON, 0, 1, NULL,
	0xffffffff, 0, 0, 0, 0, 0, _T("HRTMon") },

	{ _T("A2090a"), 0, 0, 0, 0, _T("A2090A\0"), 16384, 122, 0, 0, ROMTYPE_A2090, 0, 0, NULL,
	0x73fe45a7, 0x77ce9091,0x4be5a3bf,0x6f26b343,0x062b5bd8,0xc63c3754 },
	ALTROMPN(122, 1, 1, 8192, ROMTYPE_ODD  | ROMTYPE_8BIT, _T("315097-01"), 0x150b116c,0x8011d873,0x3be53db0,0x79dfe319,0x7ffb8634,0x1baa6dbd)
	ALTROMPN(122, 1, 2, 8192, ROMTYPE_EVEN | ROMTYPE_8BIT, _T("315098-01"), 0xbe422e3b,0x64ad1646,0x030db10f,0x54f13f64,0x7d449e4d,0x17f9ab5c)
	{ _T("A590/A2091 6.0"), 6, 0, 6, 0, _T("A590\0A2091\0"), 16384, 53, 0, 0, ROMTYPE_A2091, 0, 0, NULL,
	0x8396cf4e, 0x5E03BC61,0x8C862ABE,0x7BF79723,0xB4EEF4D2,0x1859A0F2 },
	ALTROMPN(53, 1, 1, 8192, ROMTYPE_ODD  | ROMTYPE_8BIT, _T("390389-03"), 0xb0b8cf24,0xfcf40175,0x05f4d441,0x814b45d5,0x59c19eab,0x43816b30)
	ALTROMPN(53, 1, 2, 8192, ROMTYPE_EVEN | ROMTYPE_8BIT, _T("390388-03"), 0x2e77bbff,0x8a098845,0x068f32cf,0xa4d34a27,0x8cd290f6,0x1d35a52c)
	{ _T("A590/A2091 6.6"), 6, 6, 6, 6, _T("A590\0A2091\0"), 16384, 54, 0, 0, ROMTYPE_A2091, 0, 0, NULL,
	0x33e00a7a, 0x739BB828,0xE874F064,0x9360F59D,0x26B5ED3F,0xBC99BB66 },
	ALTROMPN(54, 1, 1, 8192, ROMTYPE_ODD  | ROMTYPE_8BIT, _T("390722-02"), 0xe536bbb2,0xfd7f8a6d,0xa18c1b02,0xd07eb990,0xc2467a24,0x183ede12)
	ALTROMPN(54, 1, 2, 8192, ROMTYPE_EVEN | ROMTYPE_8BIT, _T("390721-02"), 0xc0871d25,0xe155f18a,0xbb90cf82,0x0589c15e,0x70559d3b,0x6b391af8)
	{ _T("A590/A2091 7.0"), 7, 0, 7, 0, _T("A590\0A2091\0"), 16384, 55, 0, 0, ROMTYPE_A2091, 0, 0, NULL,
	0x714a97a2, 0xE50F01BA,0xF2899892,0x85547863,0x72A82C33,0x3C91276E },
	ALTROMPN(55, 1, 1, 8192, ROMTYPE_ODD  | ROMTYPE_8BIT, _T("390722-03"), 0xa9ccffed,0x149f5bd5,0x2e2d2990,0x4e3de483,0xb9ad7724,0x48e9278e)
	ALTROMPN(55, 1, 2, 8192, ROMTYPE_EVEN | ROMTYPE_8BIT, _T("390721-03"), 0x2942747a,0xdbd7648e,0x79c75333,0x7ff3e4f4,0x91de224b,0xf05e6bb6)
	{ _T("A590/A2091 Guru ROM 6.14"), 6, 14, 6, 14, _T("A590\0A2091\0"), 32768, 56, 0, 0, ROMTYPE_A2091, 0, 0, NULL,
	0x04e52f93, 0x6DA21B6F,0x5E8F8837,0xD64507CD,0x8A4D5CDC,0xAC4F426B },
	{ _T("A4091 40.9"), 40, 9, 40, 9, _T("A4091\0"), 32768, 57, 0, 0, ROMTYPE_A4091, 0, 0, NULL,
	0x00000000, 0, 0, 0, 0, 0 },
	{ _T("A4091 40.13"), 40, 13, 40, 13, _T("A4091\0"), 32768, 58, 0, 0, ROMTYPE_A4091, 0, 0, _T("391592-02"),
	0x54cb9e85, 0x3CE66919,0xF6FD6797,0x4923A12D,0x91B730F1,0xFFB4A7BA },
	{ _T("SupraDrive AMAB6"), 3, 8, 3, 8, _T("SUPRA\0"), 16384, 121, 0, 0, ROMTYPE_SUPRA, 0, 0, _T("AMAB6"),
	0xf40bd349, 0x82168556,0x07525067,0xe9263431,0x1fb9c347,0xe737f247 },
	{ _T("SupraDrive AMAB5"), 3, 0, 3, 0, _T("SUPRA\0"), 16384, 134, 0, 0, ROMTYPE_SUPRA, 0, 0, _T("AMAB5"),
	0x75e1b343, 0xf15de74b,0x4e2a9c99,0xa6dc4f6c,0x33f64c76,0x8c043d1c },
	{ _T("SupraDrive AMAB4"), 0, 0, 0, 0, _T("SUPRA\0"), 8192, 135, 0, 0, ROMTYPE_SUPRA, 0, 0, _T("AMAB4"),
	0xde7f3f1c, 0xc0acbfc8,0x6641a6c1,0x024870cc,0x519f8c4c,0xbdfe8c64 },
	{ _T("SupraDrive AMAB3"), 0, 0, 0, 0, _T("SUPRA\0"), 8192, 136, 0, 0, ROMTYPE_SUPRA, 0, 0, _T("AMAB3"),
	0x3ead39aa, 0x02fe79ee,0xef423098,0xec6add8c,0xb92f849f,0xc64bcd41 },

	{ _T("Blizzard 1230-IV"), 0, 0, 0, 0, _T("B1230\0"), 32768, 89, 0, 0, ROMTYPE_CB_BLIZ1230, 0, 0, NULL,
	0x3078dbdc, 0x4d3e7fd0,0xa1a4c3ae,0xe17c5de3,0xcbe1af03,0x447aff92 },
	{ _T("Blizzard 1240/1260"), 0, 0, 0, 0, _T("B1240\0B1260\0"), 32768, 90, 0, 0, ROMTYPE_CB_BLIZ1260, 0, 0, NULL,
	0xf88ae0f1, 0xf69aca4b,0xb13e3389,0x04676f0c,0x8616f8db,0x074c313d },
	{ _T("Blizzard 2060"), 8, 5, 8, 5, _T("B2060\0"), 65536, 92, 0, 0, ROMTYPE_CB_BLIZ2060, 0, 0, NULL,
	0xce270bc0, 0xe043c1aa,0x3bb06e06,0xd4dabff3,0x0a8c6317,0xabfef2bb },
	ALTROMPN(92, 1, 1, 32768, ROMTYPE_ODD  | ROMTYPE_8BIT, NULL, 0xa6023f20, 0xdfb048d6, 0xbdc03587, 0x241e8121, 0x26aba603, 0xd69b0238)
	ALTROMPN(92, 1, 2, 32768, ROMTYPE_EVEN | ROMTYPE_8BIT, NULL, 0x9635a9cd, 0x47578b27, 0xc4ba6e54, 0x891930dd, 0xcb4b6a45, 0x5d6b31b2)
	{ _T("Blizzard SCSI Kit IV"), 8, 5, 8, 5, _T("BSCSIIV\0"), 32768, 94, 0, 0, ROMTYPE_CPUBOARDEXT, 0, 0, NULL,
	0xf53a0fca, 0xefe17ca5,0x88c44a7f,0x0f8c62be,0x20f23278,0xcfe06727, NULL, _T("blizzard_scsi_kit_iv.rom") },
	{ _T("Fastlane"), 8, 5, 8, 5, _T("FASTLANE\0"), 20788, 102, 0, 0, ROMTYPE_FASTLANE, 0, 0, NULL,
	0xe4f485a5, 0x20bf7de5,0x05e45d0a,0xc411cfd2,0x806d0fd8,0xe46276de, NULL, _T("fastlanez3.rom") },
	{ _T("Oktagon 2008"), 6, 12, 6, 12, _T("OKTAGON\0"), 32768, 103, 0, 0, ROMTYPE_OKTAGON, 0, 0, NULL,
	0xbb0d2f6a, 0x56c441fa,0x37d19339,0x3081b2e8,0xceae823b,0xc7e97e49, NULL, _T("oktagon2008.rom") },
	{ _T("Warp Engine A4000"), 0, 0, 0, 0, _T("WARPENGINE\0WARPENGINEA4000\0"), 32768, 93, 0, 0, ROMTYPE_CB_WENGINE, 0, 0, NULL,
	0x4deb574a, 0x6e6c95ff,0xe8448391,0xd36c5b68,0xc9065cb0,0x702a7d27 },
	{ _T("TekMagic 2040/2060"), 1, 0, 1, 0, _T("TEKMAGIC\0TEKMAGIC2040\0TEKMAGIC2060\0"), 65536, 104, 0, 0, ROMTYPE_CB_TEKMAGIC, 0, 0, NULL,
	0x9e9781d5, 0xf65b60d1,0x4300c50f,0x2ed17cf4,0x4dcfdef9,0x16697bc9, NULL,  _T("tekmagic2060.rom") },
	ALTROMPN(104, 1, 1, 32768, ROMTYPE_ODD  | ROMTYPE_8BIT, NULL, 0x888da4cf, 0x6ae85f3a, 0x65331ba4, 0xaaba67ae, 0x34763d70, 0x2bde0495)
	ALTROMPN(104, 1, 2, 32768, ROMTYPE_EVEN | ROMTYPE_8BIT, NULL, 0xaf1f47db, 0x28d5bed0, 0xbc517d46, 0x500e8159, 0x723e0b64, 0x4733c26a)
	{ _T("A2620/A2630 -07"), 0, 0, 0, 0, _T("A2620\0A2630\0"), 65536, 105, 0, 0, ROMTYPE_CB_A26x0, 0, 0, _T("390282-07/390283-07"),
	0x169d80e9, 0x41f518cb,0x41c1dc1f,0xcc636383,0x20676af5,0x4969010c, NULL, NULL },
	ALTROMPN(105, 1, 1, 32768, ROMTYPE_ODD  | ROMTYPE_8BIT, _T("390282-07"), 0xf2904058, 0x33695119, 0x5fdf5d56, 0x095a696b, 0x0ba2641d, 0x334845df)
	ALTROMPN(105, 1, 2, 32768, ROMTYPE_EVEN | ROMTYPE_8BIT, _T("390283-07"), 0xf697d458, 0x09fe260b, 0x03784e87, 0x3351dbec, 0x5146a455, 0x814383d1)
	{ _T("A2620/A2630 -06"), 0, 0, 0, 0, _T("A2620\0A2630\0"), 65536, 106, 0, 0, ROMTYPE_CB_A26x0, 0, 0, _T("390282-06/390283-06"),
	0xeb31fd9e, 0x2d6a5c68,0x1040f98d,0x7e63ad08,0x90da9e83,0x2b5c704d, NULL, NULL },
	ALTROMPN(106, 1, 1, 32768, ROMTYPE_ODD  | ROMTYPE_8BIT, _T("390282-06"), 0xd6ae582c, 0x47b3dea3, 0x31db76e6, 0x1380a3d6, 0x9f191657, 0xdd1cd4b3)
	ALTROMPN(106, 1, 2, 32768, ROMTYPE_EVEN | ROMTYPE_8BIT, _T("390283-06"), 0xcd379634, 0x65e251e2, 0xf6961c8e, 0x33a86c3d, 0x01248f70, 0xa159823b)
	{ _T("DKB 12x0"), 1, 23, 1, 23, _T("DKB\0"), 32768, 112, 0, 0, ROMTYPE_CB_DKB12x0, 0, 0, NULL,
	0xf3b2b0b3, 0x1d539593,0xb1d7514e,0xeb214ab3,0x433a97fc,0x8a010366, NULL, NULL },
	{ _T("Fusion Forty"), 0, 0, 0, 0, _T("FUSIONFORTY\0"), 131072, 113, 0, 0, ROMTYPE_CB_FUSION, 0, 0, NULL,
	0x48fcb5fd, 0x15674dac,0x90b6d8db,0xdda3a175,0x997184c2,0xa423d733, NULL, NULL },
	ALTROMPN(113, 1, 1, 32768, ROMTYPE_QUAD | ROMTYPE_EVEN | ROMTYPE_8BIT, _T("U28"), 0x434a21a8, 0x472c1623, 0x02babd00, 0x7c1a77ff, 0x40dd12ab, 0x39c97f82)
	ALTROMPN(113, 1, 2, 32768, ROMTYPE_QUAD | ROMTYPE_ODD  | ROMTYPE_8BIT, _T("U27"), 0x38373cf6, 0xfe8aa931, 0xada6b6f3, 0x6b48ca3c, 0x9b86677d, 0xbee4da59)
	ALTROMPN(113, 1, 3, 32768, ROMTYPE_QUAD | ROMTYPE_EVEN | ROMTYPE_8BIT, _T("U25"), 0xc9e990d3, 0xb251ef73, 0x1374e796, 0xa87cbc7e, 0x9263320a, 0x28a71d2b)
	ALTROMPN(113, 1, 4, 32768, ROMTYPE_QUAD | ROMTYPE_ODD  | ROMTYPE_8BIT, _T("U26"), 0x2e117fe0, 0xbb2de2da, 0x6db4e92c, 0x636fefe6, 0x13a32699, 0xcea31011)
	{ _T("Apollo 1240/1260"), 5, 60, 5, 60, _T("APOLLO12XX\0"), 131072, 119, 0, 0, ROMTYPE_CB_APOLLO, 0, 0, NULL,
	0xcd009ad9, 0x1c3b4851,0xc5a221e3,0xa7ca24fc,0xc1df4a5b,0x9f2343ad },
	{ _T("GVP A3001 Series I"), 3, 3, 3, 3, _T("A3001SI\0"), 8192, 114, 0, 0, ROMTYPE_CB_A3001S1, 0, 0, NULL,
	0xaaff7c65, 0x424cf3da,0xcc9da794,0x0ba74446,0x69dd1691,0x44ae87ee, NULL, NULL },
	{ _T("Kupke Golem 030"), 0, 0, 0, 0, _T("GOLEM030\0"), 8192, 126, 0, 0, ROMTYPE_CB_GOLEM030, 0, 0, NULL,
	0x05d473f4, 0x574ec567,0xcc67e06f,0x91dcecb9,0x8c204399,0x5fe2a09f, NULL, NULL },

	{ _T("Preferred Technologies Nexus"), 1, 0, 1, 0, _T("PTNEXUS\0"), 8192, 139, 0, 0, ROMTYPE_PTNEXUS, 0, 0, NULL,
	0xf495879a, 0xa3bd0202, 0xe14aa5b6, 0x49d3ce88, 0x22975950, 0x6500dbc2, NULL, NULL },
	{ _T("ICD AdSCSI 2000"), 1, 6, 1, 6, _T("ADSCSI\0"), 32768, 133, 0, 0, ROMTYPE_ADSCSI, 0, 0, NULL,
	0x7dba3e1f, 0x1e05f284, 0xd59a1e5d, 0x4e4de44e, 0x6f075175, 0x625cd6c0, NULL, NULL },
	{ _T("Archos ADD-500"), 1, 21, 1, 21, _T("ADD\0"), 16384, 132, 0, 0, ROMTYPE_ADD500, 0, 0, NULL,
	0x3f4e4a74, 0x9ed96fc0,0xd6381dc3,0x3192b0af,0xdfae4b74,0x576c3a69, NULL, NULL },
	{ _T("Protar A500HD"), 1, 193, 1, 193, _T("PROTAR\0"), 32768, 131, 0, 0, ROMTYPE_PROTAR, 0, 0, NULL,
	0x10c1b22c, 0x2b800cde,0x79fd559e,0xebd5e432,0xd711af3d,0x0b8ea7e9, NULL, NULL },
	{ _T("M-Tec AT500"), 1, 33, 1, 33, _T("MTECAT\0"), 32768, 130, 0, 0, ROMTYPE_MTEC, 0, 0, NULL,
	0x38b6b6b0, 0x8bb1093a,0xd592e7df,0x99c48f83,0xb9f842b2,0xb1a6e618, NULL, NULL },
	{ _T("AdIDE 40/44 v33"), 33, 0, 33, 0, _T("ADIDE\0"), 16384, 141, 0, 0, ROMTYPE_ADIDE, 0, 0, NULL,
	0x330254ce, 0xc91dd3b5,0x3f1986bd,0x94fba150,0xe753c2da,0x4dee78e7, NULL, NULL },
	{ _T("AdIDE 40/44 v34"), 34, 0, 34, 0, _T("ADIDE\0"), 16384, 129, 0, 0, ROMTYPE_ADIDE, 0, 0, NULL,
	0xedf84cbe, 0xabdbc01d,0xaa0b3aae,0xe4401ad7,0xe65525a9,0x6bfa2b27, NULL, NULL },
	{ _T("Vector Falcon 8000"), 7, 1, 7, 1, _T("VECTOR\0"), 32768, 128, 0, 0, ROMTYPE_VECTOR, 0, 0, NULL,
	0xa8120c55, 0x248935ab,0xf4d74036,0xefdafdbb,0x7817e232,0xfc13e0fa, NULL, NULL },
	{ _T("Kommos A500/A2000 v1.8"), 1, 8, 1, 8, _T("KOMMOS\0"), 32768, 127, 0, 0, ROMTYPE_KOMMOS, 0, 0, NULL,
	0xc1a5c848, 0x291310f1, 0x25455f2d, 0x8e114bcd, 0xe104ca4c, 0x9db51747, NULL, NULL },
	ALTROMPN(127, 1, 1, 16384, ROMTYPE_ODD  | ROMTYPE_8BIT, NULL, 0x1b86d2ed, 0x969995a1, 0x2ebf8c15, 0xab87e8d0, 0xddc837a1, 0xb90fbfa8)
	ALTROMPN(127, 1, 2, 16384, ROMTYPE_EVEN | ROMTYPE_8BIT, NULL, 0x65b4c3a0, 0x60e904d4, 0xe45bb6ba, 0x3e253ffa, 0xda4ee2e5, 0xc8548da1)
	{ _T("Kommos A500/A2000 v1.7"), 1, 7, 1, 7, _T("KOMMOS\0"), 32768, 140, 0, 0, ROMTYPE_KOMMOS, 0, 0, NULL,
	0x025d7835, 0x8205ad6e, 0x08dbf5a6, 0x9ba17956, 0xb5237ffe, 0x768de5a4, NULL, NULL },
	ALTROMPN(140, 1, 1, 16384, ROMTYPE_ODD  | ROMTYPE_8BIT, NULL, 0x3d982391, 0xbe26e141, 0x1e16cab4, 0xeab8af33, 0x1476ef8e, 0x21fcb2af)
	ALTROMPN(140, 1, 2, 16384, ROMTYPE_EVEN | ROMTYPE_8BIT, NULL, 0x68adc0ad, 0xb5eef088, 0xed8a9439, 0xb0302907, 0xcb3cb207, 0x62f28da7)
	{ _T("Kupke Golem v3.9"), 3, 9, 3, 9, _T("GOLEM\0"), 16384, 124, 0, 0, ROMTYPE_GOLEM, 0, 0, NULL,
	0x49157dd0, 0x03b615c9,0x2befa474,0xa37303ca,0xdc3e830d,0x3c0d9a9f, NULL, NULL },
	ALTROMPN(124, 1, 1, 8192, ROMTYPE_ODD  | ROMTYPE_8BIT, NULL, 0x88cf2ec5, 0x59680f8d, 0xae520893, 0xff9ada35, 0xac9a1146, 0x4f87453c)
	ALTROMPN(124, 1, 2, 8192, ROMTYPE_EVEN | ROMTYPE_8BIT, NULL, 0x713298ad, 0x2ad7d6f3, 0xd696fd2c, 0x51a256ee, 0xea185c47, 0x52906e04)
	{ _T("GVP Series I v1.0"), 1, 0, 1, 16, _T("GVPI\0"), 16384, 123, 0, 0, ROMTYPE_GVPS1, 0, 0, NULL,
	0x1a4c20aa, 0xb9a3377e,0x2d9b5163,0x28693c63,0x19ffb65b,0x40ae3618, NULL, NULL },
	ALTROMPN(123, 1, 1, 8192, ROMTYPE_ODD  | ROMTYPE_8BIT, NULL, 0x27f15785, 0x1a71a78d, 0xdd4e9559, 0x0f133bba, 0x4a71122f, 0x44caef78)
	ALTROMPN(123, 1, 2, 8192, ROMTYPE_EVEN | ROMTYPE_8BIT, NULL, 0xa723193e, 0x05b4a072, 0x785c7824, 0x54e003c3, 0x6d88bd9b, 0xf5f561b9)
	{ _T("GVP Series I/II v3.15"), 3, 15, 3, 15, _T("GVPII\0GVPI\0"), 16384, 111, 0, 0, ROMTYPE_GVPS12, 0, 0, NULL,
	0xf99c6f11, 0x77098a9e,0x35acaef2,0x11a546f0,0xc564cdac,0xf52836c4, NULL, NULL },
	{ _T("GVP Series II v4.15"), 4, 15, 4, 15, _T("GVPII\0"), 16384, 109, 0, 0, ROMTYPE_GVPS2, 0, 0, NULL,
	0xf89f44d6, 0xbf10c12c,0xc72dd040,0x549ea17c,0x24947633,0xe3773297, NULL, NULL },
	{ _T("GVP Series II Guru ROM"), 6, 14, 6, 14, _T("GVPII\0"), 32768, 110, 0, 0, ROMTYPE_GVPS2, 0, 0, NULL,
	0x756103b1, 0x7f1335ea,0xf5b7ce73,0xc5231173,0x261da5aa,0xe7249645, NULL, NULL },
	{ _T("AlfaPower v6.10"), 6, 10, 6, 10, _T("ALFAPOWER\0"), 32768, 117, 0, 0, ROMTYPE_ALFA, 0, 0, NULL,
	0x1cfb0a0b, 0xc7275eda,0x547d6664,0x5c4eb7a0,0x3b5cef37,0xa498365a, NULL, NULL },
	{ _T("AlfaPower v8.3"), 8, 3, 8, 3, _T("ALFAPOWERPLUS\0"), 32768, 118, 0, 0, ROMTYPE_ALFAPLUS, 0, 0, NULL,
	0xe8201bad, 0xdefea015,0x596fce32,0x11e84397,0x23046a31,0x5a7726dc, NULL, NULL },
	{ _T("Masoboshi MC-702"), 2, 201, 2, 201, _T("MASOBOSHI\0"), 32768, 120, 0, 0, ROMTYPE_MASOBOSHI, 0, 0, NULL,
	0xcd99b98a, 0x3897e46a,0x66d5833f,0x849b8e81,0x30acb3cb,0x319a2fa0, NULL, NULL },
	{ _T("Roctec RocHard RH800C"), 1, 0, 1, 0, _T("ROCHARD\0"), 16384, 138, 0, 0, ROMTYPE_ROCHARD, 0, 0, NULL,
	0x0e980aec, 0xbcafa14d,0xe80576cb,0xe3e0c638,0x1ca90379,0xe078a8bd, NULL, NULL },
	ALTROMPN(138, 1, 1, 8192, ROMTYPE_ODD  | ROMTYPE_8BIT, NULL, 0xde3a855b, 0xda2fe069, 0xd78c9ccc, 0xc221711f, 0x1e598298, 0x2bdabffd)
	ALTROMPN(138, 1, 2, 8192, ROMTYPE_EVEN | ROMTYPE_8BIT, NULL, 0xb0ed3006, 0x0a88d84e, 0x2094f9e5, 0x18d37f90, 0x34764f22, 0x9696c3d9)

	{ _T("CyberStorm MK I 68040"), 0, 0, 0, 0, _T("CSMKI\0"), 32768, 95, 0, 0, ROMTYPE_CB_CSMK1, 0, 0, NULL,
	  0, 0, 0, 0, 0, 0, NULL, _T("cyberstormmk1_040.rom") },
	{ _T("CyberStorm MK I 68060"), 0, 0, 0, 0, _T("CSMKI\0"), 65536, 101, 0, 0, ROMTYPE_CB_CSMK1, 0, 0, NULL,
	  0, 0, 0, 0, 0, 0, NULL, _T("cyberstormmk1_060.rom") },
	{ _T("CyberStorm MK II"), 0, 0, 0, 0, _T("CSMKII\0"), 131072, 96, 0, 0, ROMTYPE_CB_CSMK2, 0, 0, NULL,
	  0, 0, 0, 0, 0, 0, NULL, _T("cyberstormmk2.rom") },
	{ _T("CyberStorm MK III"), 0, 0, 0, 0, _T("CSMKIII\0"), 131072, 97, 0, 0, ROMTYPE_CB_CSMK3, 0, 0, NULL,
	  0, 0, 0, 0, 0, 0, NULL, _T("cyberstormmk3.rom") },
	{ _T("CyberStorm PPC"), 0, 0, 0, 0, _T("CSPPC\0"), 131072, 98, 0, 0, ROMTYPE_CB_CSPPC, 0, 0, NULL,
	  0, 0, 0, 0, 0, 0, NULL, _T("cyberstormppc.rom") },
	{ _T("Blizzard PPC 68040"), 0, 0, 0, 0, _T("BPPC\0"), 524288, 99, 0, 0, ROMTYPE_CB_BLIZPPC, 0, 0, NULL,
	  0, 0, 0, 0, 0, 0, NULL, _T("blizzardppc_040.rom") },
	{ _T("Blizzard PPC 68060"), 0, 0, 0, 0, _T("BPPC\0"), 524288, 100, 0, 0, ROMTYPE_CB_BLIZPPC, 0, 0, NULL,
	  0, 0, 0, 0, 0, 0, NULL, _T("blizzardppc_060.rom") },
	{ _T("ACA 500"), 0, 0, 0, 0, _T("ACA500\0"), 524288, 137, 0, 0, ROMTYPE_CB_ACA500, 0, 0, NULL,
	  0, 0, 0, 0, 0, 0, NULL, _T("menu500.aca") },

	{ _T("Picasso IV"), 7, 4, 7, 4, _T("PIV\0"), 131072, 91, 0, 0, ROMTYPE_PIV, 0, 0, NULL,
	0xa8133e7e, 0xcafafb91,0x6f16b9f3,0xec9b49aa,0x4b40eb4e,0xeceb5b5b },

	{ _T("Arcadia OnePlay 2.11"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 49, 0, 0, ROMTYPE_ARCADIABIOS, 0, 0 },
	{ _T("Arcadia TenPlay 2.11"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 50, 0, 0, ROMTYPE_ARCADIABIOS, 0, 0 },
	{ _T("Arcadia TenPlay 2.20"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 75, 0, 0, ROMTYPE_ARCADIABIOS, 0, 0 },
	{ _T("Arcadia OnePlay 3.00"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 51, 0, 0, ROMTYPE_ARCADIABIOS, 0, 0 },
	{ _T("Arcadia TenPlay 3.11"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 76, 0, 0, ROMTYPE_ARCADIABIOS, 0, 0 },
	{ _T("Arcadia TenPlay 4.00"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 77, 0, 0, ROMTYPE_ARCADIABIOS, 0, 0 },

	{ _T("Arcadia SportTime Table Hockey v2.1"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 33, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia SportTime Bowling v2.1"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 34, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia World Darts v2.1"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 35, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Magic Johnson's Fast Break v2.8"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 36, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Leader Board Golf v2.4"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 37, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Leader Board Golf"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 38, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Ninja Mission v2.5"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 39, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Road Wars v2.3"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 40, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Sidewinder v2.1"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 41, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Spot v2.0"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 42, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Space Ranger v2.0"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 43, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Xenon v2.3"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 44, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia World Trophy Soccer v3.0"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 45, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Blastaball v2.1"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 78, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Delta Command"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 79, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Pharaohs Match"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 80, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia SportTime Table Hockey"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 81, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia World Darts (bad)"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 82, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Magic Johnson's Fast Break v2.7"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 83, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Ninja Mission"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 84, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Sidewinder"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 85, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Leader Board Golf v2.5"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 86, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },
	{ _T("Arcadia Aaargh"), 0, 0, 0, 0, _T("ARCADIA\0"), 0, 88, 0, 0, ROMTYPE_ARCADIAGAME, 0, 2 },

	{ NULL }

};

void romlist_clear (void)
{
	int i;
	int mask = 0;
	struct romdata *parent;
	const TCHAR *pn;

	xfree (rl);
	rl = 0;
	romlist_cnt = 0;
	parent = 0;
	pn = NULL;
	for (i = 0; roms[i].name; i++) {
		struct romdata *rd = &roms[i];
		if (rd->group == 0) {
			parent = rd;
			mask = rd->type;
			pn = parent->partnumber;
		} else {
			rd->type &= ~ROMTYPE_MASK;
			rd->type |= mask & ROMTYPE_MASK;
			if (rd->partnumber && !pn) {
				TCHAR *newpn;
				if (parent->partnumber == NULL)
					parent->partnumber = my_strdup (_T(""));
				newpn = xcalloc (TCHAR, _tcslen (parent->partnumber) + 1 + _tcslen (rd->partnumber) + 1);
				if (_tcslen (parent->partnumber) > 0) {
					_tcscpy (newpn, parent->partnumber);
					_tcscat (newpn, _T("/"));
				}
				_tcscat (newpn, rd->partnumber);
				xfree ((char *) parent->partnumber);
				parent->partnumber = newpn;
			}
		}
	}
}

/* remove rom entries that need 2 or more roms but not everything required is present */
static void romlist_cleanup (void)
{
	int i = 0;
	while (roms[i].name) {
		struct romdata *rd = &roms[i];
		int grp = rd->group >> 16;
		int ok = 1;
		int j = i;
		int k = i;
		while (rd->name && (rd->group >> 16) == grp && grp > 0) {
			struct romlist *rl = romlist_getrl (rd);
			if (!rl)
				ok = 0;
			rd++;
			j++;
		}
		if (ok == 0) {
			while (i < j) {
				struct romlist *rl2 = romlist_getrl (&roms[i]);
				if (rl2) {
					int cnt = romlist_cnt - (rl2 - rl) - 1;
					write_log (_T("%s '%s' removed from romlist\n"), roms[k].name, rl2->path);
					xfree (rl2->path);
					if (cnt > 0)
						memmove (rl2, rl2 + 1, cnt * sizeof (struct romlist));
					romlist_cnt--;
				}
				i++;
			}
		}
		i++;
	}
#if 0
	for (i = 0; i < romlist_cnt; i++) {
		struct romlist *rll = &rl[i];
		write_log (_T("%d: %08x %s (%s)\n"), rll->rd->id, rll->rd->group, rll->rd->name, rll->path);
	}
#endif
}

struct romlist **getromlistbyident (int ver, int rev, int subver, int subrev, const TCHAR *model, int romflags, bool all)
{
	int i, j, ok, out, max;
	struct romdata *rd;
	struct romlist **rdout, *rltmp;
	void *buf;

	for (i = 0; roms[i].name; i++);
	if (all)
		max = i;
	else
		max = romlist_cnt;
	buf = xmalloc (uae_u8, (sizeof (struct romlist*) + sizeof (struct romlist)) * (i + 1));
	rdout = (struct romlist**)buf;
	rltmp = (struct romlist*)((uae_u8*)buf + (i + 1) * sizeof (struct romlist*));
	out = 0;
	for (i = 0; i < max; i++) {
		ok = 0;
		if (!all)
			rd = rl[i].rd;
		else
			rd = &roms[i];
		if (rd->group)
			continue;
		if (model && !_tcsicmp (model, rd->name))
			ok = 2;
		if ((ver < 0 || rd->ver == ver) && (rev < 0 || rd->rev == rev)) {
			if (subver >= 0) {
				if (rd->subver == subver && (subrev < 0 || rd->subrev == subrev) && rd->subver > 0)
					ok = 1;
			} else {
				ok = 1;
			}
		}
		if (!ok)
			continue;
		if (model && ok < 2) {
			const TCHAR *p = rd->model;
			ok = 0;
			while (p && *p) {
				if (!_tcscmp(rd->model, model)) {
					ok = 1;
					break;
				}
				p = p + _tcslen(p) + 1;
			}
		}
		if (romflags && (rd->type & romflags) == 0)
			ok = 0;
		if (ok) {
			if (all) {
				rdout[out++] = rltmp;
				rltmp->path = NULL;
				rltmp->rd = rd;
				rltmp++;
			} else {
				rdout[out++] = &rl[i];
			}
		}
	}
	if (out == 0) {
		xfree (rdout);
		return NULL;
	}
	for (i = 0; i < out; i++) {
		int v1 = rdout[i]->rd->subver * 1000 + rdout[i]->rd->subrev;
		for (j = i + 1; j < out; j++) {
			int v2 = rdout[j]->rd->subver * 1000 + rdout[j]->rd->subrev;
			if (v1 < v2) {
				struct romlist *rltmp = rdout[j];
				rdout[j] = rdout[i];
				rdout[i] = rltmp;
			}
		}
	}
	rdout[out] = NULL;
	return rdout;
}

struct romdata *getarcadiarombyname (const TCHAR *name)
{
	int i;
	for (i = 0; roms[i].name; i++) {
		if (roms[i].group == 0 && (roms[i].type == ROMTYPE_ARCADIAGAME || roms[i].type == ROMTYPE_ARCADIAGAME)) {
			const TCHAR *p = roms[i].name;
			p = p + _tcslen (p) + 1;
			if (_tcslen (name) >= _tcslen (p) + 4) {
				const TCHAR *p2 = name + _tcslen (name) - _tcslen (p) - 4;
				if (!memcmp (p, p2, _tcslen (p)) && !memcmp (p2 + _tcslen (p2) - 4, ".zip", 4))
					return &roms[i];
			}
		}
	}
	return NULL;
}

struct romlist **getarcadiaroms (void)
{
	int i, out, max;
	void *buf;
	struct romlist **rdout, *rltmp;

	max = 0;
	for (i = 0; roms[i].name; i++) {
		if (roms[i].group == 0 && (roms[i].type == ROMTYPE_ARCADIABIOS || roms[i].type == ROMTYPE_ARCADIAGAME))
			max++;
	}
	buf = xmalloc (uae_u8, (sizeof (struct romlist*) + sizeof (struct romlist)) * (max + 1));
	rdout = (struct romlist**)buf;
	rltmp = (struct romlist*)((uae_u8*)buf + (max + 1) * sizeof (struct romlist*));
	out = 0;
	for (i = 0; roms[i].name; i++) {
		if (roms[i].group == 0 && (roms[i].type == ROMTYPE_ARCADIABIOS || roms[i].type == ROMTYPE_ARCADIAGAME)) {
			rdout[out++] = rltmp;
			rltmp->path = NULL;
			rltmp->rd = &roms[i];
			rltmp++;
		}
	}
	rdout[out] = NULL;
	return rdout;
}


static int kickstart_checksum_do (uae_u8 *mem, int size)
{
	uae_u32 cksum = 0, prevck = 0;
	int i;
	for (i = 0; i < size; i+=4) {
		uae_u32 data = mem[i]*65536*256 + mem[i+1]*65536 + mem[i+2]*256 + mem[i+3];
		cksum += data;
		if (cksum < prevck)
			cksum++;
		prevck = cksum;
	}
	return cksum == 0xffffffff;
}

#define ROM_KEY_NUM 4
struct rom_key {
	uae_u8 *key;
	int size;
};

static struct rom_key keyring[ROM_KEY_NUM];

static void addkey (uae_u8 *key, int size, const TCHAR *name)
{
	int i;

	//write_log (_T("addkey(%08x,%d,'%s')\n"), key, size, name);
	if (key == NULL || size == 0) {
		xfree (key);
		return;
	}
	for (i = 0; i < ROM_KEY_NUM; i++) {
		if (keyring[i].key && keyring[i].size == size && !memcmp (keyring[i].key, key, size)) {
			xfree (key);
			//write_log (_T("key already in keyring\n"));
			return;
		}
	}
	for (i = 0; i < ROM_KEY_NUM; i++) {
		if (keyring[i].key == NULL)
			break;
	}
	if (i == ROM_KEY_NUM) {
		xfree (key);
		//write_log (_T("keyring full\n"));
		return;
	}
	keyring[i].key = key;
	keyring[i].size = size;
	write_log (_T("ROM KEY '%s' %d bytes loaded\n"), name, size);
}

void addkeyfile (const TCHAR *path)
{
	struct zfile *f;
	int keysize;
	uae_u8 *keybuf;

	f = zfile_fopen (path, _T("rb"), ZFD_NORMAL);
	if (!f)
		return;
	zfile_fseek (f, 0, SEEK_END);
	keysize = zfile_ftell (f);
	if (keysize > 0) {
		zfile_fseek (f, 0, SEEK_SET);
		keybuf = xmalloc (uae_u8, keysize);
		zfile_fread (keybuf, 1, keysize, f);
		addkey (keybuf, keysize, path);
	}
	zfile_fclose (f);
}

void addkeydir (const TCHAR *path)
{
	TCHAR tmp[MAX_DPATH];

	_tcscpy (tmp, path);
	if (zfile_exists (tmp)) {
		int i;
		for (i = _tcslen (tmp) - 1; i > 0; i--) {
			if (tmp[i] == '\\' || tmp[i] == '/')
				break;
		}
		tmp[i] = 0;
	}
	_tcscat (tmp, _T("/"));
	_tcscat (tmp, _T("rom.key"));
	addkeyfile (tmp);
}

int get_keyring (void)
{
	int i, num = 0;
	for (i = 0; i < ROM_KEY_NUM; i++) {
		if (keyring[i].key)
			num++;
	}
	return num;
}

int load_keyring (struct uae_prefs *p, const TCHAR *path)
{
	uae_u8 *keybuf;
	int keysize;
	TCHAR tmp[MAX_PATH], *d;
	int keyids[] = { 0, 48, 73, -1 };
	int cnt, i;

	free_keyring ();
	keybuf = target_load_keyfile (p, path, &keysize, tmp);
	addkey (keybuf, keysize, tmp);
	for (i = 0; keyids[i] >= 0; i++) {
		struct romdata *rd = getromdatabyid (keyids[i]);
		TCHAR *s;
		if (rd) {
			s = romlist_get (rd);
			if (s)
				addkeyfile (s);
		}
	}

	cnt = 0;
	for (;;) {
		keybuf = NULL;
		keysize = 0;
		tmp[0] = 0;
		switch (cnt)
		{
		case 0:
			if (path)
				_tcscpy (tmp, path);
			break;
		case 1:
			_tcscat (tmp, _T("rom.key"));
			break;
		case 2:
			if (p) {
				_tcscpy (tmp, p->path_rom.path[0]);
				_tcscat (tmp, _T("rom.key"));
			}
			break;
		case 3:
			_tcscpy (tmp, _T("roms/rom.key"));
			break;
		case 4:
			_tcscpy (tmp, start_path_data);
			_tcscat (tmp, _T("rom.key"));
			break;
		case 5:
			_stprintf (tmp, _T("%s../shared/rom/rom.key"), start_path_data);
			break;
		case 6:
			if (p) {
				for (i = 0; uae_archive_extensions[i]; i++) {
					if (_tcsstr (p->romfile, uae_archive_extensions[i]))
						break;
				}
				if (!uae_archive_extensions[i]) {
					_tcscpy (tmp, p->romfile);
					d = _tcsrchr (tmp, '/');
					if (!d)
						d = _tcsrchr (tmp, '\\');
					if (d)
						_tcscpy (d + 1, _T("rom.key"));
				}
			}
			break;
		case 7:
			return get_keyring ();
		}
		cnt++;
		if (!tmp[0])
			continue;
		addkeyfile (tmp);
	}
}
void free_keyring (void)
{
	int i;
	for (i = 0; i < ROM_KEY_NUM; i++)
		xfree (keyring[i].key);
	memset (keyring, 0, sizeof (struct rom_key) * ROM_KEY_NUM);
}

struct romdata *getromdatabyname (const TCHAR *name)
{
	TCHAR tmp[MAX_PATH];
	int i = 0;
	while (roms[i].name) {
		if (!roms[i].group) {
			getromname (&roms[i], tmp);
			if (!_tcscmp (tmp, name) || !_tcscmp (roms[i].name, name))
				return &roms[i];
		}
		i++;
	}
	return 0;
}

struct romdata *getromdatabyid (int id)
{
	int i = 0;
	while (roms[i].name) {
		if (id == roms[i].id && roms[i].group == 0)
			return &roms[i];
		i++;
	}
	return 0;
}

struct romdata *getromdatabyidgroup (int id, int group, int subitem)
{
	int i = 0;
	group = (group << 16) | subitem;
	while (roms[i].name) {
		if (id == roms[i].id && roms[i].group == group)
			return &roms[i];
		i++;
	}
	return 0;
}

STATIC_INLINE int notcrc32 (uae_u32 crc32)
{
	if (crc32 == 0xffffffff || crc32 == 0x00000000)
		return 1;
	return 0;
}

struct romdata *getromdatabycrc (uae_u32 crc32, bool allowgroup)
{
	int i = 0;
	while (roms[i].name) {
		if (roms[i].group == 0 && crc32 == roms[i].crc32 && !notcrc32(crc32))
			return &roms[i];
		i++;
	}
	if (allowgroup) {
		i = 0;
		while (roms[i].name) {
			if (roms[i].group && crc32 == roms[i].crc32 && !notcrc32(crc32))
				return &roms[i];
			i++;
		}
	}
	return 0;
}
struct romdata *getromdatabycrc (uae_u32 crc32)
{
	return getromdatabycrc (crc32, false);
}

static int cmpsha1 (const uae_u8 *s1, const struct romdata *rd)
{
	int i;

	for (i = 0; i < SHA1_SIZE / 4; i++) {
		uae_u32 v1 = (s1[0] << 24) | (s1[1] << 16) | (s1[2] << 8) | (s1[3] << 0);
		uae_u32 v2 = rd->sha1[i];
		if (v1 != v2)
			return -1;
		s1 += 4;
	}
	return 0;
}

struct romdata *getfrombydefaultname(const TCHAR *name, int size)
{
	int i = 0;
	while (roms[i].name) {
		if (notcrc32(roms[i].crc32) && size >= roms[i].size && roms[i].defaultfilename && !_tcsicmp(roms[i].defaultfilename, name)) {
			return &roms[i];
		}
		i++;
	}
	return NULL;
}

static struct romdata *checkromdata (const uae_u8 *sha1, int size, uae_u32 mask)
{
	int i = 0;
	while (roms[i].name) {
		if (!notcrc32(roms[i].crc32) && roms[i].size >= size) {
			if (roms[i].type & mask) {
				if (!cmpsha1 (sha1, &roms[i]))
					return &roms[i];
			}
		}
		i++;
	}
	return NULL;
}

int decode_cloanto_rom_do (uae_u8 *mem, int size, int real_size)
{
	int cnt, t, i;

	for (i = ROM_KEY_NUM - 1; i >= 0; i--) {
		uae_u8 sha1[SHA1_SIZE];
		struct romdata *rd;
		int keysize = keyring[i].size;
		uae_u8 *key = keyring[i].key;
		if (!key)
			continue;
		for (t = cnt = 0; cnt < size; cnt++, t = (t + 1) % keysize)  {
			mem[cnt] ^= key[t];
			if (real_size == cnt + 1)
				t = keysize - 1;
		}
		if ((mem[2] == 0x4e && mem[3] == 0xf9) || (mem[0] == 0x11 && (mem[1] == 0x11 || mem[1] == 0x14))) {
			cloanto_rom = 1;
			return 1;
		}
		get_sha1 (mem, size, sha1);
		rd = checkromdata (sha1, size, -1);
		if (rd) {
			if (rd->cloanto)
				cloanto_rom = 1;
			return 1;
		}
		if (i == 0)
			break;
		for (t = cnt = 0; cnt < size; cnt++, t = (t + 1) % keysize)  {
			mem[cnt] ^= key[t];
			if (real_size == cnt + 1)
				t = keysize - 1;
		}
	}
	return 0;
}

static int decode_rekick_rom_do (uae_u8 *mem, int size, int real_size)
{
	uae_u32 d1 = 0xdeadfeed, d0;
	int i;

	for (i = 0; i < size / 8; i++) {
		d0 = ((mem[i * 8 + 0] << 24) | (mem[i * 8 + 1] << 16) | (mem[i * 8 + 2] << 8) | mem[i * 8 + 3]);
		d1 = d1 ^ d0;
		mem[i * 8 + 0] = d1 >> 24;
		mem[i * 8 + 1] = d1 >> 16;
		mem[i * 8 + 2] = d1 >> 8;
		mem[i * 8 + 3] = d1;
		d1 = ((mem[i * 8 + 4] << 24) | (mem[i * 8 + 5] << 16) | (mem[i * 8 + 6] << 8) | mem[i * 8 + 7]);
		d0 = d0 ^ d1;
		mem[i * 8 + 4] = d0 >> 24;
		mem[i * 8 + 5] = d0 >> 16;
		mem[i * 8 + 6] = d0 >> 8;
		mem[i * 8 + 7] = d0;
	}
	return 1;
}

int decode_rom (uae_u8 *mem, int size, int mode, int real_size)
{
	if (mode == 1) {
		if (!decode_cloanto_rom_do (mem, size, real_size)) {
#ifndef SINGLEFILE
			notify_user (NUMSG_NOROMKEY);
#endif
			return 0;
		}
		return 1;
	} else if (mode == 2) {
		decode_rekick_rom_do (mem, size, real_size);
		return 1;
	}
	return 0;
}

struct romdata *getromdatabydata (uae_u8 *rom, int size)
{
	uae_u8 sha1[SHA1_SIZE];
	uae_u8 tmp[4];
	uae_u8 *tmpbuf = NULL;
	struct romdata *ret = NULL;

	if (size > 11 && !memcmp (rom, "AMIROMTYPE1", 11)) {
		uae_u8 *tmpbuf = xmalloc (uae_u8, size);
		int tmpsize = size - 11;
		memcpy (tmpbuf, rom + 11, tmpsize);
		decode_rom (tmpbuf, tmpsize, 1, tmpsize);
		rom = tmpbuf;
		size = tmpsize;
	}
#if 0
	if (size > 0x6c + 524288 && !memcmp (rom, "AMIG", 4)) {
		uae_u8 *tmpbuf = (uae_u8*)xmalloc (uae_u8, size);
		int tmpsize = size - 0x6c;
		memcpy (tmpbuf, rom + 0x6c, tmpsize);
		decode_rom (tmpbuf, tmpsize, 2, tmpsize);
		rom = tmpbuf;
		size = tmpsize;
	}
#endif
	get_sha1 (rom, size, sha1);
	ret = checkromdata(sha1, size, -1);
	if (!ret) {
		get_sha1 (rom, size / 2, sha1);
		ret = checkromdata (sha1, size / 2, -1);
		if (!ret) {
			/* ignore AR2/3 IO-port range until we have full dump */
			memcpy (tmp, rom, 4);
			memset (rom, 0, 4);
			get_sha1 (rom, size, sha1);
			ret = checkromdata (sha1, size, ROMTYPE_AR2);
			memcpy (rom, tmp, 4);
		}
	}//9 
	xfree (tmpbuf);
	return ret;
}

struct romdata *getromdatabyzfile (struct zfile *f)
{
	int pos, size;
	uae_u8 *p;
	struct romdata *rd;

	pos = zfile_ftell (f);
	zfile_fseek (f, 0, SEEK_END);
	size = zfile_ftell (f);
	if (size > 2048 * 1024)
		return NULL;
	p = xmalloc (uae_u8, size);
	if (!p)
		return NULL;
	memset (p, 0, size);
	zfile_fseek (f, 0, SEEK_SET);
	zfile_fread (p, 1, size, f);
	zfile_fseek (f, pos, SEEK_SET);
	rd = getromdatabydata (p, size);
	xfree (p);
	return rd;
}

void getromname	(const struct romdata *rd, TCHAR *name)
{
	name[0] = 0;
	if (!rd)
		return;
	while (rd->group)
		rd--;
	_tcscat (name, rd->name);
	if ((rd->subrev || rd->subver) && rd->subver != rd->ver)
		_stprintf (name + _tcslen (name), _T(" rev %d.%d"), rd->subver, rd->subrev);
	if (rd->size > 0)
		_stprintf (name + _tcslen (name), _T(" (%dk)"), (rd->size + 1023) / 1024);
	if (rd->partnumber && _tcslen (rd->partnumber) > 0)
		_stprintf (name + _tcslen (name), _T(" [%s]"), rd->partnumber);
}

struct romlist *getromlistbyromdata (const struct romdata *rd)
{
	int ids[2];

	ids[0] = rd->id;
	ids[1] = 0;
	return getromlistbyids(ids, NULL);
}

struct romlist *getromlistbyids (const int *ids, const TCHAR *romname)
{
	struct romdata *rd;
	int i, j;

	i = 0;
	if (romname) {
		while (ids[i] >= 0) {
			rd = getromdatabyid (ids[i]);
			if (rd) {
				for (j = 0; j < romlist_cnt; j++) {
					if (rl[j].rd->id == rd->id) {
						if (my_issamepath(rl[j].path, romname))
							return &rl[j];
					}
				}
			}
			i++;
		}
	}
	i = 0;
	while (ids[i] >= 0) {
		rd = getromdatabyid (ids[i]);
		if (rd) {
			for (j = 0; j < romlist_cnt; j++) {
				if (rl[j].rd->id == rd->id)
					return &rl[j];
			}
		}
		i++;
	}
	return NULL;
}

struct romdata *getromdatabyids (const int *ids)
{
	struct romdata *rd;
	int i;

	i = 0;
	while (ids[i] >= 0) {
		rd = getromdatabyid (ids[i]);
		if (rd)
			return rd;
		i++;
	}
	return NULL;
}

void romwarning (const int *ids)
{
	int i, exp;
	TCHAR tmp1[MAX_DPATH], tmp2[MAX_DPATH];
	TCHAR tmp3[MAX_DPATH];

	if (ids[0] == -1)
		return;
	exp = 0;
	tmp2[0] = 0;
	i = 0;
	while (ids[i] >= 0) {
		struct romdata *rd = getromdatabyid (ids[i]);
		if (!(rd->type & ROMTYPE_NONE)) {
			getromname (rd, tmp1);
			_tcscat (tmp2, _T("- "));
			_tcscat (tmp2, tmp1);
			_tcscat (tmp2, _T("\n"));
			if (rd->type & (ROMTYPE_SCSI | ROMTYPE_CPUBOARD | ROMTYPE_CD32CART))
				exp++;
		}
		i++;
	}
	translate_message (exp ? NUMSG_EXPROMNEED : NUMSG_ROMNEED, tmp3);
	gui_message (tmp3, tmp2);
}


static void byteswap (uae_u8 *buf, int size)
{
	int i;
	for (i = 0; i < size; i += 2) {
		uae_u8 t = buf[i];
		buf[i] = buf[i + 1];
		buf[i + 1] = t;
	}
}
static void wordbyteswap (uae_u8 *buf, int size)
{
	int i;
	for (i = 0; i < size; i += 4) {
		uae_u8 t;
		t = buf[i + 0];
		buf[i + 0] = buf[i + 2];
		buf[i + 2] = t;
		t = buf[i + 1];
		buf[i + 1] = buf[i + 3];
		buf[i + 3] = t;
	}
}

static void mergecd32 (uae_u8 *dst, uae_u8 *src, int size)
{
	int i, k;
	k = 0;
	for (i = 0; i < size / 2; i += 2) {
		int j = i + size / 2;
		dst[k + 1] = src[i + 0];
		dst[k + 0] = src[i + 1];
		dst[k + 3] = src[j + 0];
		dst[k + 2] = src[j + 1];
		k += 4;
	}
#if 0
	{
		struct zfile *f;
		f = zfile_fopen ("c:\\d\\1.rom","wb", ZFD_NORMAL);
		zfile_fwrite (dst, 1, size, f);
		zfile_fclose(f);
	}
#endif
}

static void descramble (const struct romdata *rd, uae_u8 *data, int size, int odd)
{
	int flags = rd->type;

	if (flags & (ROMTYPE_NORDIC | ROMTYPE_XPOWER))
		descramble_nordicpro (data, size, odd);
}

static int read_rom_file (uae_u8 *buf, const struct romdata *rd)
{
	struct zfile *zf;
	struct romlist *rl = romlist_getrl (rd);
	uae_char tmp[11];

	if (!rl || _tcslen (rl->path) == 0)
		return 0;
	zf = zfile_fopen (rl->path, _T("rb"), ZFD_NORMAL);
	if (!zf)
		return 0;
	addkeydir (rl->path);
	zfile_fread (tmp, sizeof tmp, 1, zf);
	if (!memcmp (tmp, "AMIROMTYPE1", sizeof tmp)) {
		zfile_fread (buf, rd->size, 1, zf);
		decode_cloanto_rom_do (buf, rd->size, rd->size);
	} else {
		memcpy (buf, tmp, sizeof tmp);
		zfile_fread (buf + sizeof tmp, rd->size - sizeof (tmp), 1, zf);
	}
#ifdef FSUAE
	romlist_patch_rom(buf, rd->size);
#endif
	zfile_fclose (zf);
	return 1;
}

#if SAVE_ROM
static void save_rom(uae_u8 *rom, int size)
{
	struct zfile *f;
	f = zfile_fopen (_T("c:\\temp\\1.rom"), _T("wb"));
	zfile_fwrite (rom, 1, size, f);
	zfile_fclose(f);
}
#endif

struct zfile *read_rom (struct romdata *prd)
{
	struct romdata *rd2 = prd;
	struct romdata *rd = prd;
	struct romdata *rdpair = NULL;
	const TCHAR *name;
	int id = rd->id;
	uae_u32 crc32;
	int size;
	uae_u8 *buf, *buf2;

	/* find parent node */
	for (;;) {
		if (rd2 == &roms[0])
			break;
		if (rd2[-1].id != id)
			break;
		rd2--;
	}
	
	size = rd2->size;
	crc32 = rd2->crc32;
	name = rd->name;
	buf = xmalloc (uae_u8, size * 2);
	memset (buf, 0xff, size * 2);
	if (!buf)
		return NULL;
	buf2 = buf + size;
	while (rd->id == id) {
		int i, j, add;
		int ok = 0;
		uae_u32 flags = rd->type;
		int odd = (flags & ROMTYPE_ODD) ? 1 : 0;

		add = 0;
		for (i = 0; i < 2; i++) {
			memset (buf, 0, size);
			if (!(flags & (ROMTYPE_EVEN | ROMTYPE_ODD))) {
				read_rom_file (buf, rd);
				if (flags & ROMTYPE_CD32) {
					memcpy (buf2, buf, size);
					mergecd32 (buf, buf2, size);
				}
				add = 1;
				i++;
			} else if (flags & ROMTYPE_QUAD) {
				if (i == 0) {
					for (int k = 0; k < 4; k++) {
						read_rom_file (buf2, rd2 + k + 1);
						for (j = 0; j < size; j += 4)
							buf[j + k] = buf2[j / 4];
					}
				} else {
					for (int kk = 0; kk < 2; kk++) {
						for (int k = 0; k < 2; k++) {
							read_rom_file (buf2, rd2 + k + kk * 2 + 1);
							for (j = 0; j < size / 2; j += 2) {
								buf[j + k + kk * (rd2->size / 2)] = buf2[j / 2];
							}
						}
					}
				}
				add = 4;
			} else {
				int romsize = size / 2;
				if (i)
					odd = !odd;
				if (rd->id == rd[1].id)
					rdpair = &rd[1];
				else if (rd != roms)
					rdpair = &rd[-1];
				else
					rdpair = rd;
				if (flags & ROMTYPE_8BIT) {
					read_rom_file (buf2, rd);
					if (flags & ROMTYPE_BYTESWAP)
						byteswap (buf2, romsize);
					if (flags & ROMTYPE_SCRAMBLED)
						descramble (rd, buf2, romsize, odd);
					for (j = 0; j < size; j += 2)
						buf[j + odd] = buf2[j / 2];
					read_rom_file (buf2, rdpair);
					if (flags & ROMTYPE_BYTESWAP)
						byteswap (buf2, romsize);
					if (flags & ROMTYPE_SCRAMBLED)
						descramble (rd + 1, buf2, romsize, !odd);
					for (j = 0; j < size; j += 2)
						buf[j + (1 - odd)] = buf2[j / 2];
				} else {
					read_rom_file (buf2, rd);
					if (flags & ROMTYPE_BYTESWAP)
						byteswap (buf2, romsize);
					if (flags & ROMTYPE_SCRAMBLED)
						descramble (rd, buf2, romsize, odd);
					for (j = 0; j < size; j += 4) {
						buf[j + 2 * odd + 0] = buf2[j / 2 + 0];
						buf[j + 2 * odd + 1] = buf2[j / 2 + 1];
					}
					read_rom_file (buf2, rdpair);
					if (flags & ROMTYPE_BYTESWAP)
						byteswap (buf2, romsize);
					if (flags & ROMTYPE_SCRAMBLED)
						descramble (rd + 1, buf2, romsize, !odd);
					for (j = 0; j < size; j += 4) {
						buf[j + 2 * (1 - odd) + 0] = buf2[j / 2 + 0];
						buf[j + 2 * (1 - odd) + 1] = buf2[j / 2 + 1];
					}
				}
				add = 2;
			}

#if SAVE_ROM
			save_rom(buf, size);
#endif

			if (notcrc32(crc32) || get_crc32(buf, size) == crc32) {
				ok = 1;
			}
			if (!ok && (rd->type & ROMTYPE_AR)) {
				uae_u8 tmp[2];
				tmp[0] = buf[0];
				tmp[1] = buf[1];
				buf[0] = buf[1] = 0;
				if (get_crc32 (buf, size) == crc32)
					ok = 1;
				buf[0] = tmp[0];
				buf[1] = tmp[1];
			}
			if (!ok) {
				/* perhaps it is byteswapped without byteswap entry? */
				byteswap (buf, size);
				if (get_crc32 (buf, size) == crc32)
					ok = 1;
				if (!ok)
					byteswap(buf, size);
			}
			if (ok) {
				struct zfile *zf = zfile_fopen_empty (NULL, name, size);
				if (zf) {
					zfile_fwrite (buf, size, 1, zf);
					zfile_fseek (zf, 0, SEEK_SET);
				}
				xfree (buf);
				return zf;
			}
		}
		rd += add;

	}
	xfree (buf);
	return NULL;
}

struct zfile *rom_fopen (const TCHAR *name, const TCHAR *mode, int mask)
{
	struct zfile *f;
	//write_log (_T("attempting to load '%s'\n"), name); 
	f = zfile_fopen (name, mode, mask);
	//write_log (_T("=%p\n"), f);
	return f;
}

struct zfile *read_rom_name (const TCHAR *filename)
{
	int i;
	struct zfile *f;

	for (i = 0; i < romlist_cnt; i++) {
#ifdef FSUAE
		// FIXME: implement my_issamepath
#endif
		if (my_issamepath(filename, rl[i].path)) {
			struct romdata *rd = rl[i].rd;
			f = read_rom (rd);
			if (f)
				return f;
		}
	}
	f = rom_fopen (filename, _T("rb"), ZFD_NORMAL);
	if (f) {
		uae_u8 tmp[11];
		zfile_fread (tmp, sizeof tmp, 1, f);
		if (!memcmp (tmp, "AMIROMTYPE1", sizeof tmp)) {
			struct zfile *df;
			int size;
			uae_u8 *buf;
			addkeydir (filename);
			zfile_fseek (f, 0, SEEK_END);
			size = zfile_ftell (f) - sizeof tmp;
			zfile_fseek (f, sizeof tmp, SEEK_SET);
			buf = xmalloc (uae_u8, size);
			zfile_fread (buf, size, 1, f);
			df = zfile_fopen_empty (f, _T("tmp.rom"), size);
			decode_cloanto_rom_do (buf, size, size);
#ifdef FSUAE
			romlist_patch_rom(buf, size);
#endif
			zfile_fwrite (buf, size, 1, df);
			zfile_fclose (f);
			xfree (buf);
			zfile_fseek (df, 0, SEEK_SET);
			f = df;
		} else {
			zfile_fseek (f, -((int)sizeof tmp), SEEK_CUR);
		}
	}
	return f;
}

struct zfile *read_rom_name_guess (const TCHAR *filename)
{
	int i, j;
	struct zfile *f;
	const TCHAR *name;

	for (i = _tcslen (filename) - 1; i >= 0; i--) {
		if (filename[i] == '/' || filename[i] == '\\')
			break;
	}
	if (i < 0)
		return NULL;
	name = &filename[i];

	for (i = 0; i < romlist_cnt; i++) {
		TCHAR *n = rl[i].path;
		for (j = _tcslen (n) - 1; j >= 0; j--) {
			if (n[j] == '/' || n[j] == '\\')
				break;
		}
		if (j < 0)
			continue;
		if (!_tcsicmp (name, n + j)) {
			struct romdata *rd = rl[i].rd;
			f = read_rom (rd);
			if (f) {
				write_log (_T("ROM %s not found, using %s\n"), filename, rl[i].path);
				return f;
			}
		}
	}
	return NULL;
}

void kickstart_fix_checksum (uae_u8 *mem, int size)
{
	uae_u32 cksum = 0, prevck = 0;
	int i, ch = size == 524288 ? 0x7ffe8 : (size == 262144 ? 0x3ffe8 : 0x3e);

	mem[ch] = 0;
	mem[ch + 1] = 0;
	mem[ch + 2] = 0;
	mem[ch + 3] = 0;
	for (i = 0; i < size; i+=4) {
		uae_u32 data = (mem[i] << 24) | (mem[i + 1] << 16) | (mem[i + 2] << 8) | mem[i + 3];
		cksum += data;
		if (cksum < prevck)
			cksum++;
		prevck = cksum;
	}
	cksum ^= 0xffffffff;
	mem[ch++] = cksum >> 24;
	mem[ch++] = cksum >> 16;
	mem[ch++] = cksum >> 8;
	mem[ch++] = cksum >> 0;
}

int kickstart_checksum (uae_u8 *mem, int size)
{
	if (!kickstart_checksum_do (mem, size)) {
#ifndef	SINGLEFILE
		notify_user (NUMSG_KSROMCRCERROR);
#endif
		return 0;
	}
	return 1;
}

int configure_rom (struct uae_prefs *p, const int *rom, int msg)
{
	struct romdata *rd;
	TCHAR *path = 0;
	int i;

	if (rom[0] < 0)
		return 1;
	i = 0;
	while (rom[i] >= 0) {
		rd = getromdatabyid (rom[i]);
		if (!rd) {
			i++;
			continue;
		}
		path = romlist_get (rd);
		if (path)
			break;
		i++;
	}
	if (!path) {
		if (msg)
			romwarning(rom);
		return 0;
	}
	if (rd->type & (ROMTYPE_KICK | ROMTYPE_KICKCD32))
		_tcscpy (p->romfile, path);
	if (rd->type & (ROMTYPE_EXTCD32 | ROMTYPE_EXTCDTV | ROMTYPE_ARCADIABIOS))
		_tcscpy (p->romextfile, path);
	if (rd->type & (ROMTYPE_CD32CART | ROMTYPE_ARCADIAGAME) ||
		rd->type == ROMTYPE_HRTMON || rd->type == ROMTYPE_XPOWER || rd->type ==  ROMTYPE_NORDIC || rd->type == ROMTYPE_AR || rd->type ==  ROMTYPE_SUPERIV)
		_tcscpy (p->cartfile, path);
	if (rd->type & ROMTYPE_CPUBOARD)
		set_device_rom(p, path, ROMTYPE_CPUBOARD, 0);
	if (rd->type & ROMTYPE_CPUBOARDEXT)
		set_device_rom(p, path, ROMTYPE_CPUBOARDEXT, 0);
	return 1;
}

void set_device_rom(struct uae_prefs *p, const TCHAR *path, int romtype, int devnum)
{
	int idx;
	const struct expansionromtype *ert = get_device_expansion_rom(romtype);
	if (path == NULL) {
		struct boardromconfig *brc = get_device_rom(p, romtype, devnum, &idx);
		if (brc) {
			brc->roms[idx].romfile[0] = 0;
			brc->roms[idx].romident[0] = 0;
		}
	} else {
		struct boardromconfig *brc = get_device_rom_new(p, romtype, devnum, &idx);
		_tcscpy(brc->roms[idx].romfile, path);
	}
}

const struct expansionromtype *get_unit_expansion_rom(int hdunit)
{
	if (hdunit >= HD_CONTROLLER_TYPE_SCSI_EXPANSION_FIRST && hdunit <= HD_CONTROLLER_TYPE_SCSI_LAST)
		return &expansionroms[hdunit - HD_CONTROLLER_TYPE_SCSI_EXPANSION_FIRST];
	if (hdunit >= HD_CONTROLLER_TYPE_IDE_EXPANSION_FIRST && hdunit <= HD_CONTROLLER_TYPE_IDE_LAST)
		return &expansionroms[hdunit - HD_CONTROLLER_TYPE_IDE_EXPANSION_FIRST];
	return NULL;
}

const struct expansionromtype *get_device_expansion_rom(int romtype)
{
	for (int i = 0; expansionroms[i].name; i++) {
		const struct expansionromtype *ert = &expansionroms[i];
		if ((ert->romtype & ROMTYPE_MASK) == (romtype & ROMTYPE_MASK))
			return ert;
	}
	return NULL;
}

static void device_rom_defaults(struct boardromconfig *brc, int romtype, int devnum)
{
	memset(brc, 0, sizeof(boardromconfig));
	brc->device_type = romtype;
	brc->device_num = devnum;
	for (int i = 0; i < MAX_BOARD_ROMS; i++) {
		brc->roms[i].device_id = 7;	
	}
}

struct boardromconfig *get_device_rom_new(struct uae_prefs *p, int romtype, int devnum, int *index)
{
	int idx2;
	static struct boardromconfig fake;
	const struct expansionromtype *ert = get_device_expansion_rom(romtype);
	if (!ert) {
		*index = 0;
		return &fake;
	}
	*index = ert->parentromtype ? 1 : 0;
	struct boardromconfig *brc = get_device_rom(p, ert->parentromtype ? ert->parentromtype : romtype, devnum, &idx2);
	if (!brc) {
		for (int i = 0; i < MAX_EXPANSION_BOARDS; i++) {
			brc = &p->expansionboard[i];
			if (brc->device_type == 0)
				continue;
			int ok = 0;
			for (int j = 0; j < MAX_BOARD_ROMS; j++) {
				if (!brc->roms[j].romfile[0] && !brc->roms[j].romident[0] && !brc->roms[j].board_ram_size)
					ok++;
			}
			if (ok == MAX_BOARD_ROMS)
				memset(brc, 0, sizeof(boardromconfig));
		}
		for (int i = 0; i < MAX_EXPANSION_BOARDS; i++) {
			brc = &p->expansionboard[i];
			if (brc->device_type == 0) {
				device_rom_defaults(brc, romtype, devnum);
				return brc;
			}
		}
		return &fake;
	}
	return brc;
}

void clear_device_rom(struct uae_prefs *p, int romtype, int devnum)
{
	int index;
	struct boardromconfig *brc = get_device_rom(p, romtype, devnum, &index);
	if (!brc)
		return;
	memset(&brc->roms[index], 0, sizeof(struct romconfig));
}

struct boardromconfig *get_device_rom(struct uae_prefs *p, int romtype, int devnum, int *index)
{
	const struct expansionromtype *ert = get_device_expansion_rom(romtype);
	if (!ert) {
		*index = 0;
		return NULL;
	}
	int parentrom = ert->parentromtype ? ert->parentromtype : romtype;
	*index = ert->parentromtype ? 1 : 0;
	for (int i = 0; i < MAX_EXPANSION_BOARDS; i++) {
		struct boardromconfig *brc = &p->expansionboard[i];
		if (!brc->device_type)
			continue;
		if ((brc->device_type & ROMTYPE_MASK) == (parentrom & ROMTYPE_MASK) && brc->device_num == devnum)
			return brc;
	}
	return NULL;
}

struct romconfig *get_device_romconfig(struct uae_prefs *p, int romtype, int devnum)
{
	int idx;
	struct boardromconfig *brc = get_device_rom(p, romtype, devnum, &idx);
	if (brc)
		return &brc->roms[idx];
	return NULL;
}

struct zfile *read_device_from_romconfig(struct romconfig *rc, const int *roms)
{
	if (!_tcsicmp(rc->romfile, _T(":NOROM")))
		return NULL;
	struct zfile *z = read_rom_name (rc->romfile);
	if (!z) {
		struct romlist *rl = getromlistbyids(roms, rc->romfile);
		if (rl) {
			struct romdata *rd = rl->rd;
			z = read_rom (rd);
		}
		if (!z) {
			romwarning (roms);
		}
	}
	return z;
}

struct zfile *read_device_rom(struct uae_prefs *p, int romtype, int devnum, int *roms)
{
	int idx;
	struct boardromconfig *brc = get_device_rom(p, romtype, devnum, &idx);
	if (brc) {
		const TCHAR *romname = brc->roms[idx].romfile;
		if (!_tcsicmp(romname, _T(":NOROM")))
			return NULL;
		struct zfile *z = read_rom_name (romname);
		if (!z) {
			struct romlist *rl = getromlistbyids(roms, romname);
			if (rl) {
				struct romdata *rd = rl->rd;
				z = read_rom (rd);
			}
		}
		return z;
	}
	return NULL;
}

int is_device_rom(struct uae_prefs *p, int romtype, int devnum)
{
	int idx;
	struct boardromconfig *brc = get_device_rom(p, romtype, devnum, &idx);
	if (brc) {
		const TCHAR *romname = brc->roms[idx].romfile;
		if (_tcslen(romname) == 0)
			return -1;
		if (!_tcsicmp(romname, _T(":NOROM")))
			return 0;
		return 1;
	}
	return -1;
}

struct boardromconfig *get_boardromconfig(struct uae_prefs *p, int romtype, int *index)
{
	for (int i = 0; i < MAX_EXPANSION_BOARDS; i++) {
		struct boardromconfig *brc = &p->expansionboard[i];
		if (!brc->device_type)
			continue;
		if ((brc->device_type & ROMTYPE_MASK) == (romtype & ROMTYPE_MASK)) {
			for (int j = 0; j < MAX_BOARD_ROMS; j++) {
				if (brc->roms[j].romfile[0]) {
					if (index)
						*index = j;
					return brc;
				}
			}
		}
	}
	return NULL;
}

bool load_rom_rc(struct romconfig *rc, const int *roms, int maxfilesize, int fileoffset, uae_u8 *rom, int maxromsize, int flags)
{
	if (flags & LOADROM_ONEFILL)
		memset(rom, 0xff, maxromsize);
	if (flags & LOADROM_ZEROFILL)
		memset(rom, 0x00, maxromsize);
	struct zfile *f = read_device_from_romconfig(rc, roms);
	if (!f)
		return false;
	zfile_fseek(f, fileoffset, SEEK_SET);
	int cnt = 0;
	int pos = 0;
	bool eof = false;
	while (cnt < maxromsize && cnt < maxfilesize && pos < maxromsize) {
		uae_u8 b = 0xff;
		if (!eof) {
			if (!zfile_fread(&b, 1, 1, f))
				eof = true;
		}
		if (eof) {
			int bitcnt = 0;
			for (int i = 1; i < maxromsize; i <<= 1) {
				if (cnt & i)
					bitcnt++;
			}
			if (bitcnt == 1)
				break;
		}

		rom[pos] = b;
		if (flags & LOADROM_EVENONLY) {
			rom[pos + 1] = (flags >> 16) & 0xff;
			pos += 2;
		} else {
			pos += 1;
		}
		cnt++;
	}
	zfile_fclose(f);
	int posend = pos;
	if (!(flags & LOADROM_FILL))
		return true;
	int oldpos = 0;
	while (pos < maxromsize) {
		rom[pos] = rom[oldpos];
		oldpos++;
		pos++;
		if (oldpos >= posend)
			oldpos = 0;
	}
	return true;
}
