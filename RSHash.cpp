#include "Global.h"
#include "RSHash.h"
#include "Blake512.h"
#include "SHA256.h"
#include <stdint.h>
#include <iostream>
using std::cout;
using std::endl;

#define PHI 0x9e3779b9
#define BLOCKHASH_1_PADSIZE (1024*1024*4)

typedef unsigned int uint32;
typedef unsigned long long int uint64;

static uint32 BlockHash_1_Q[4096], BlockHash_1_c, BlockHash_1_i;
unsigned char *BlockHash_1_MemoryPAD8;
uint32 *BlockHash_1_MemoryPAD32_to_8;
uint32 *BlockHash_1_MemoryPAD32;

uint32 BlockHash_1_rand(void)
{
	uint32 & pos = BlockHash_1_Q[BlockHash_1_i & 0xFFF];
	uint64 t = 0x495ELL * pos + BlockHash_1_c;
	BlockHash_1_c = (t >> 32);
	uint32 x = (t + BlockHash_1_c) & 0xFFFFFFFF;
	if (x < BlockHash_1_c) {
		x++;
		BlockHash_1_c++;
	}
	++BlockHash_1_i;
	return (pos = 0xffffffff + (~x));
}

#include <cstdio>
#include <iomanip>

void BlockHash_Init()
{
	try {
		static unsigned char SomeArrogantText1[] =
		    "Back when I was born the world was different. As a kid I could run around the streets, build things in the forest, go to the beach and generally live a care free life. Sure I had video games and played them a fair amount but they didn't get in the way of living an adventurous life. The games back then were different too. They didn't require 40 hours of your life to finish. Oh the good old days, will you ever come back?";
		static unsigned char SomeArrogantText2[] =
		    "Why do most humans not understand their shortcomings? The funny thing with the human brain is it makes everyone arrogant at their core. Sure some may fight it more than others but in every brain there is something telling them, HEY YOU ARE THE MOST IMPORTANT PERSON IN THE WORLD. THE CENTER OF THE UNIVERSE. But we can't all be that, can we? Well perhaps we can, introducing GODria, take 2 pills of this daily and you can be like RealSolid, lord of the universe.";
		static unsigned char SomeArrogantText3[] =
		    "What's up with kids like artforz that think it's good to attack other's work? He spent a year in the bitcoin scene riding on the fact he took some other guys SHA256 opencl code and made a miner out of it. Bravo artforz, meanwhile all the false praise goes to his head and he thinks he actually is a programmer. Real programmers innovate and create new work, they win through being better coders with better ideas. You're not real artforz, and I hear you like furries? What's up with that? You shouldn't go on IRC when you're drunk, people remember the weird stuff.";
		BlockHash_1_MemoryPAD8 = new unsigned char[BLOCKHASH_1_PADSIZE + 8];	//need the +8 for memory overwrites
		BlockHash_1_MemoryPAD32_to_8 =
		    (uint32 *) BlockHash_1_MemoryPAD8;

		BlockHash_1_Q[0] = 0x6970F271;
		BlockHash_1_Q[1] = uint32(0x6970F271ULL + PHI);
		BlockHash_1_Q[2] = uint32(0x6970F271ULL + PHI + PHI);
		for (int i = 3; i < 4096; i++)
			BlockHash_1_Q[i] =
			    BlockHash_1_Q[i - 3] ^ BlockHash_1_Q[i -
								 2] ^ PHI ^ i;
		BlockHash_1_c = 362436;
		BlockHash_1_i = 0;

		int count1 = 0, count2 = 0, count3 = 0;
		for (int x = 0; x < (BLOCKHASH_1_PADSIZE / 4) + 2; x++)
			BlockHash_1_MemoryPAD32_to_8[x] = BlockHash_1_rand();
		for (int x = 0; x < BLOCKHASH_1_PADSIZE + 8; x++) {
			switch (BlockHash_1_MemoryPAD8[x] & 3) {
			case 0:
				BlockHash_1_MemoryPAD8[x] ^=
				    SomeArrogantText1[count1++];
				if (count1 >= sizeof(SomeArrogantText1))
					count1 = 0;
				break;
			case 1:
				BlockHash_1_MemoryPAD8[x] ^=
				    SomeArrogantText2[count2++];
				if (count2 >= sizeof(SomeArrogantText2))
					count2 = 0;
				break;
			case 2:
				BlockHash_1_MemoryPAD8[x] ^=
				    SomeArrogantText3[count3++];
				if (count3 >= sizeof(SomeArrogantText3))
					count3 = 0;
				break;
			case 3:
				BlockHash_1_MemoryPAD8[x] ^= 0xAA;
				break;
			}
		}
		BlockHash_1_MemoryPAD32 = new uint32[BLOCKHASH_1_PADSIZE];
		for (uint32 i = 0; i < BLOCKHASH_1_PADSIZE; ++i)
			BlockHash_1_MemoryPAD32[i] =
			    *(uint32 *) (BlockHash_1_MemoryPAD8 + i);
	}
	catch(std::exception s) {
		cout << "(3) Error: " << s.what() << endl;
	}
}

void BlockHash_DeInit()
{
	delete[]BlockHash_1_MemoryPAD8;
	delete[]BlockHash_1_MemoryPAD32;
}

const uint32 PAD_MASK = BLOCKHASH_1_PADSIZE - 1;
typedef unsigned char uchar;

#define READ_PAD8(offset) BlockHash_1_MemoryPAD8[(offset)&PAD_MASK]
#define READ_PAD32(offset) (*((uint32*)&BlockHash_1_MemoryPAD8[(offset)&PAD_MASK]))

typedef unsigned int uint;

void BlockHash_1(unsigned char *p512bytes, unsigned char *final_hash)
{
	//0->127   is the block header      (128)
	//128->191 is blake(blockheader)    (64)
	//192->511 is scratch work area     (320)

	unsigned char *work1 = p512bytes;
	unsigned char *work2 = work1 + 128;
	unsigned char *work3 = work1 + 192;

	blake512_hash(work2, work1);

	//setup the 320 scratch with some base values
#define WORKINIT(a,b,c)   work3[a] ^= work2[c]; \
        if(work3[a]&0x80) work3[b]=work1[(b+work3[a])&127]; \
        else              work3[b]=work2[(b+work3[a])&63];

	work3[0] = work2[15];
	for (int x = 1; x < 320; x++) {
		WORKINIT(x - 1, x, x & 63);
	}

	uint64 qCount = *((uint64 *) & work3[310]);
	int nExtra = (READ_PAD8(qCount + work3[300]) >> 3) + 512;
	for (int x = 1; x < nExtra; x++) {
		qCount += READ_PAD32(qCount);
		if (qCount & 0x87878700)
			work3[qCount % 320]++;

		qCount -= READ_PAD8(qCount + work3[qCount % 160]);
		if (qCount & 0x80000000) {
			qCount += READ_PAD8(qCount & 0x8080FFFF);
		} else {
			qCount += READ_PAD32(qCount & 0x7F60FAFB);
		}

		qCount += READ_PAD32(qCount + work3[qCount % 160]);
		if (qCount & 0xF0000000)
			work3[qCount % 320]++;

		qCount += READ_PAD32(*((uint32 *) & work3[qCount & 0xFF]));
		work3[x % 320] = work2[x & 63] ^ uchar(qCount);

		qCount += READ_PAD32((qCount >> 32) + work3[x % 200]);
		*((uint32 *) & work3[qCount % 316]) ^=
		    (qCount >> 24) & 0xFFFFFFFF;
		if ((qCount & 0x07) == 0x03)
			x++;
		qCount -= READ_PAD8((x * x));
		if ((qCount & 0x07) == 0x01)
			x++;
	}

	Sha256(work1, final_hash);
}

#ifdef REAPER_BUILD_64BIT
#define remu320(x) ((x)%320)
#define remu316(x) ((x)%316)
#define remu160(x) ((x)%160)
#else
//256
uint remu320(ullint x)
{
	uint n =
	    (((x >> 38UL) + ((x >> 6) & 0xFFFFFFFF)) >> 32) + ((x >> 38UL) +
							       ((x >> 6) &
								0xFFFFFFFF) &
							       0xFFFFFFFF);
	return ((n % 5) << 6) + (x & 63U);
}

//96
uint remu160(ullint x)
{
	uint n =
	    (((x >> 37UL) + ((x >> 5) & 0xFFFFFFFF)) >> 32) + ((x >> 37UL) +
							       ((x >> 5) &
								0xFFFFFFFF) &
							       0xFFFFFFFF);
	return ((n % 5) << 5) + (x & 31U);
}

//208
uint remu316(ullint x)
{
	x = (x >> 32) * 208 + (uint) x;
	x = (x >> 32) * 208 + (uint) x;
	return ((uint) x + (uint) (x >> 32) * 208U) % 316;
}
#endif

void BlockHash_1_mine_V3(unsigned char *p512bytes, unsigned char *final_hash,
			 unsigned char *results)
{
	//0->127   is the block header      (128)
	//128->191 is blake(blockheader)    (64)
	//192->511 is scratch work area     (320)

	uchar *work1 = p512bytes;
	uchar *xork1 = p512bytes + 512;
	uchar *york1 = p512bytes + 1024;
	uchar *work2 = work1 + 128;
	uchar *xork2 = xork1 + 128;
	uchar *york2 = york1 + 128;
	uchar *work3 = work1 + 192;
	uchar *xork3 = xork1 + 192;
	uchar *york3 = york1 + 192;

	blake512_hash(work2, work1);
	blake512_hash(xork2, xork1);
	blake512_hash(york2, york1);

#define ORKINIT(a,b,c) \
        work3[a] ^= work2[c]; \
        xork3[a] ^= xork2[c]; \
        york3[a] ^= york2[c]; \
        if(work3[a]&0x80) work3[b]=work1[(b+work3[a])&127]; \
        else              work3[b]=work2[(b+work3[a])&63]; \
        if(xork3[a]&0x80) xork3[b]=xork1[(b+xork3[a])&127]; \
        else              xork3[b]=xork2[(b+xork3[a])&63]; \
        if(york3[a]&0x80) york3[b]=york1[(b+york3[a])&127]; \
        else              york3[b]=york2[(b+york3[a])&63];

	work3[0] = work2[15];
	xork3[0] = xork2[15];
	york3[0] = york2[15];
	ORKINIT(0, 1, 1);
	for (int x = 2; x < 320; x += 2) {
		ORKINIT(x - 1, x, x & 63);
		ORKINIT(x, x + 1, (x + 1) & 63);
	}
#undef ORKINIT

	uint64 counts[3];
#define qCount counts[0]
#define rCount counts[1]
#define sCount counts[2]
	qCount = *((uint64 *) & work3[310]);
	rCount = *((uint64 *) & xork3[310]);
	sCount = *((uint64 *) & york3[310]);
	int nExtra = (READ_PAD8(qCount + work3[300]) >> 3) + 512;
	int oExtra = (READ_PAD8(rCount + xork3[300]) >> 3) + 512;
	int pExtra = (READ_PAD8(sCount + york3[300]) >> 3) + 512;
	int x, y, z;
	for (x = 1, y = 1, z = 1; x < nExtra && y < oExtra && z < pExtra;
	     x++, y++, z++) {
		qCount += READ_PAD32(qCount);
		rCount += READ_PAD32(rCount);
		sCount += READ_PAD32(sCount);

		if (qCount & 0x87878700)
			++work3[remu320(qCount)];
		if (rCount & 0x87878700)
			++xork3[remu320(rCount)];
		if (sCount & 0x87878700)
			++york3[remu320(sCount)];

		qCount -= READ_PAD8(qCount + work3[remu160(qCount)]);
		rCount -= READ_PAD8(rCount + xork3[remu160(rCount)]);
		sCount -= READ_PAD8(sCount + york3[remu160(sCount)]);

		if (qCount & 0x80000000) {
			qCount += READ_PAD8(qCount & 0x8080FFFF);
		} else {
			qCount += READ_PAD32(qCount & 0x7F60FAFB);
		}
		if (rCount & 0x80000000) {
			rCount += READ_PAD8(rCount & 0x8080FFFF);
		} else {
			rCount += READ_PAD32(rCount & 0x7F60FAFB);
		}
		if (sCount & 0x80000000) {
			sCount += READ_PAD8(sCount & 0x8080FFFF);
		} else {
			sCount += READ_PAD32(sCount & 0x7F60FAFB);
		}

		qCount += READ_PAD32(qCount + work3[remu160(qCount)]);
		rCount += READ_PAD32(rCount + xork3[remu160(rCount)]);
		sCount += READ_PAD32(sCount + york3[remu160(sCount)]);

		if (qCount & 0xF0000000)
			++work3[remu320(qCount)];
		if (rCount & 0xF0000000)
			++xork3[remu320(rCount)];
		if (sCount & 0xF0000000)
			++york3[remu320(sCount)];

		qCount += READ_PAD32(*((uint32 *) & work3[qCount & 0xFF]));
		rCount += READ_PAD32(*((uint32 *) & xork3[rCount & 0xFF]));
		sCount += READ_PAD32(*((uint32 *) & york3[sCount & 0xFF]));

		work3[x % 320] = work2[x & 63] ^ uchar(qCount);
		xork3[y % 320] = xork2[y & 63] ^ uchar(rCount);
		york3[z % 320] = york2[z & 63] ^ uchar(sCount);

		qCount += READ_PAD32((qCount >> 32) + work3[x % 200]);
		rCount += READ_PAD32((rCount >> 32) + xork3[y % 200]);
		sCount += READ_PAD32((sCount >> 32) + york3[z % 200]);

		*((uint32 *) & work3[remu316(qCount)]) ^=
		    (qCount >> 24) & 0xFFFFFFFF;
		*((uint32 *) & xork3[remu316(rCount)]) ^=
		    (rCount >> 24) & 0xFFFFFFFF;
		*((uint32 *) & york3[remu316(sCount)]) ^=
		    (sCount >> 24) & 0xFFFFFFFF;

		if ((qCount & 0x07) == 0x03)
			x++;
		if ((rCount & 0x07) == 0x03)
			y++;
		if ((sCount & 0x07) == 0x03)
			z++;

		qCount -= READ_PAD8((x * x));
		rCount -= READ_PAD8((y * y));
		sCount -= READ_PAD8((z * z));

		if ((qCount & 0x07) == 0x01)
			x++;
		if ((rCount & 0x07) == 0x01)
			y++;
		if ((sCount & 0x07) == 0x01)
			z++;
	}
	while (x < nExtra) {
		qCount += READ_PAD32(qCount);
		if (qCount & 0x87878700)
			++work3[remu320(qCount)];

		qCount -= READ_PAD8(qCount + work3[remu160(qCount)]);
		if (qCount & 0x80000000) {
			qCount += READ_PAD8(qCount & 0x8080FFFF);
		} else {
			qCount += READ_PAD32(qCount & 0x7F60FAFB);
		}

		qCount += READ_PAD32(qCount + work3[remu160(qCount)]);

		if (qCount & 0xF0000000)
			++work3[remu320(qCount)];

		qCount += READ_PAD32(*((uint32 *) & work3[qCount & 0xFF]));

		work3[x % 320] = work2[x & 63] ^ uchar(qCount);

		qCount += READ_PAD32((qCount >> 32) + work3[x % 200]);
		*((uint32 *) & work3[remu316(qCount)]) ^=
		    (qCount >> 24) & 0xFFFFFFFF;
		if ((qCount & 0x07) == 0x03)
			x++;
		qCount -= READ_PAD8((x * x));
		if ((qCount & 0x07) == 0x01)
			x++;
		x++;
	}
	while (y < oExtra) {
		rCount += READ_PAD32(rCount);
		if (rCount & 0x87878700)
			++xork3[remu320(rCount)];

		rCount -= READ_PAD8(rCount + xork3[remu160(rCount)]);
		if (rCount & 0x80000000) {
			rCount += READ_PAD8(rCount & 0x8080FFFF);
		} else {
			rCount += READ_PAD32(rCount & 0x7F60FAFB);
		}

		rCount += READ_PAD32(rCount + xork3[remu160(rCount)]);

		if (rCount & 0xF0000000)
			++xork3[remu320(rCount)];

		rCount += READ_PAD32(*((uint32 *) & xork3[rCount & 0xFF]));

		xork3[y % 320] = xork2[y & 63] ^ uchar(rCount);

		rCount += READ_PAD32((rCount >> 32) + xork3[y % 200]);
		*((uint32 *) & xork3[remu316(rCount)]) ^=
		    (rCount >> 24) & 0xFFFFFFFF;
		if ((rCount & 0x07) == 0x03)
			y++;
		rCount -= READ_PAD8((y * y));
		if ((rCount & 0x07) == 0x01)
			y++;
		y++;
	}
	while (z < pExtra) {
		sCount += READ_PAD32(sCount);
		if (sCount & 0x87878700)
			++york3[remu320(sCount)];

		sCount -= READ_PAD8(sCount + york3[remu160(sCount)]);
		if (sCount & 0x80000000) {
			sCount += READ_PAD8(sCount & 0x8080FFFF);
		} else {
			sCount += READ_PAD32(sCount & 0x7F60FAFB);
		}

		sCount += READ_PAD32(sCount + york3[remu160(sCount)]);

		if (sCount & 0xF0000000)
			++york3[remu320(sCount)];

		sCount += READ_PAD32(*((uint32 *) & york3[sCount & 0xFF]));

		york3[z % 320] = york2[z & 63] ^ uchar(sCount);

		sCount += READ_PAD32((sCount >> 32) + york3[z % 200]);
		*((uint32 *) & york3[remu316(sCount)]) ^=
		    (sCount >> 24) & 0xFFFFFFFF;
		if ((sCount & 0x07) == 0x03)
			z++;
		sCount -= READ_PAD8((z * z));
		if ((sCount & 0x07) == 0x01)
			z++;
		z++;
	}

	Sha256(work1, final_hash);
	results[0] = (final_hash[30] == 0 && final_hash[31] == 0
		      && final_hash[29] < 0x80);
	Sha256(xork1, final_hash);
	results[1] = (final_hash[30] == 0 && final_hash[31] == 0
		      && final_hash[29] < 0x80);
	Sha256(york1, final_hash);
	results[2] = (final_hash[30] == 0 && final_hash[31] == 0
		      && final_hash[29] < 0x80);
}

void BlockHash_1_mine_V2(unsigned char *p512bytes, unsigned char *final_hash,
			 unsigned char *results)
{
	//0->127   is the block header      (128)
	//128->191 is blake(blockheader)    (64)
	//192->511 is scratch work area     (320)

	uchar *work1 = p512bytes;
	uchar *xork1 = p512bytes + 512;
	uchar *work2 = work1 + 128;
	uchar *xork2 = xork1 + 128;
	uchar *work3 = work1 + 192;
	uchar *xork3 = xork1 + 192;

	blake512_hash(work2, work1);
	blake512_hash(xork2, xork1);

#define ORKINIT(a,b,c) \
        work3[a] ^= work2[c]; \
        xork3[a] ^= xork2[c]; \
        if(work3[a]&0x80) work3[b]=work1[(b+work3[a])&127]; \
        else              work3[b]=work2[(b+work3[a])&63]; \
        if(xork3[a]&0x80) xork3[b]=xork1[(b+xork3[a])&127]; \
        else              xork3[b]=xork2[(b+xork3[a])&63]; \


	work3[0] = work2[15];
	xork3[0] = xork2[15];
	ORKINIT(0, 1, 1);
	for (int x = 2; x < 320; x += 2) {
		ORKINIT(x - 1, x, x & 63);
		ORKINIT(x, x + 1, (x + 1) & 63);
	}
#undef ORKINIT

	uint64 counts[2];
#define qCount counts[0]
#define rCount counts[1]
	qCount = *((uint64 *) & work3[310]);
	rCount = *((uint64 *) & xork3[310]);
	int nExtra = (READ_PAD8(qCount + work3[300]) >> 3) + 512;
	int oExtra = (READ_PAD8(rCount + xork3[300]) >> 3) + 512;
	int x, y;
	for (x = 1, y = 1; x < nExtra && y < oExtra; x++, y++) {
		qCount += READ_PAD32(qCount);
		rCount += READ_PAD32(rCount);

		if (qCount & 0x87878700)
			++work3[remu320(qCount)];
		if (rCount & 0x87878700)
			++xork3[remu320(rCount)];

		qCount -= READ_PAD8(qCount + work3[remu160(qCount)]);
		rCount -= READ_PAD8(rCount + xork3[remu160(rCount)]);

		if (qCount & 0x80000000) {
			qCount += READ_PAD8(qCount & 0x8080FFFF);
		} else {
			qCount += READ_PAD32(qCount & 0x7F60FAFB);
		}
		if (rCount & 0x80000000) {
			rCount += READ_PAD8(rCount & 0x8080FFFF);
		} else {
			rCount += READ_PAD32(rCount & 0x7F60FAFB);
		}

		qCount += READ_PAD32(qCount + work3[remu160(qCount)]);
		rCount += READ_PAD32(rCount + xork3[remu160(rCount)]);

		if (qCount & 0xF0000000)
			++work3[remu320(qCount)];
		if (rCount & 0xF0000000)
			++xork3[remu320(rCount)];

		qCount += READ_PAD32(*((uint32 *) & work3[qCount & 0xFF]));
		rCount += READ_PAD32(*((uint32 *) & xork3[rCount & 0xFF]));

		work3[x % 320] = work2[x & 63] ^ uchar(qCount);
		xork3[y % 320] = xork2[y & 63] ^ uchar(rCount);

		qCount += READ_PAD32((qCount >> 32) + work3[x % 200]);
		rCount += READ_PAD32((rCount >> 32) + xork3[y % 200]);

		*((uint32 *) & work3[remu316(qCount)]) ^=
		    (qCount >> 24) & 0xFFFFFFFF;
		*((uint32 *) & xork3[remu316(rCount)]) ^=
		    (rCount >> 24) & 0xFFFFFFFF;

		if ((qCount & 0x07) == 0x03)
			x++;
		if ((rCount & 0x07) == 0x03)
			y++;

		qCount -= READ_PAD8((x * x));
		rCount -= READ_PAD8((y * y));

		if ((qCount & 0x07) == 0x01)
			x++;
		if ((rCount & 0x07) == 0x01)
			y++;
	}
	while (x < nExtra) {
		qCount += READ_PAD32(qCount);
		if (qCount & 0x87878700)
			++work3[remu320(qCount)];

		qCount -= READ_PAD8(qCount + work3[remu160(qCount)]);
		if (qCount & 0x80000000) {
			qCount += READ_PAD8(qCount & 0x8080FFFF);
		} else {
			qCount += READ_PAD32(qCount & 0x7F60FAFB);
		}

		qCount += READ_PAD32(qCount + work3[remu160(qCount)]);

		if (qCount & 0xF0000000)
			++work3[remu320(qCount)];

		qCount += READ_PAD32(*((uint32 *) & work3[qCount & 0xFF]));

		work3[x % 320] = work2[x & 63] ^ uchar(qCount);

		qCount += READ_PAD32((qCount >> 32) + work3[x % 200]);
		*((uint32 *) & work3[remu316(qCount)]) ^=
		    (qCount >> 24) & 0xFFFFFFFF;
		if ((qCount & 0x07) == 0x03)
			x++;
		qCount -= READ_PAD8((x * x));
		if ((qCount & 0x07) == 0x01)
			x++;
		x++;
	}
	while (y < oExtra) {
		rCount += READ_PAD32(rCount);
		if (rCount & 0x87878700)
			++xork3[remu320(rCount)];

		rCount -= READ_PAD8(rCount + xork3[remu160(rCount)]);
		if (rCount & 0x80000000) {
			rCount += READ_PAD8(rCount & 0x8080FFFF);
		} else {
			rCount += READ_PAD32(rCount & 0x7F60FAFB);
		}

		rCount += READ_PAD32(rCount + xork3[remu160(rCount)]);

		if (rCount & 0xF0000000)
			++xork3[remu320(rCount)];

		rCount += READ_PAD32(*((uint32 *) & xork3[rCount & 0xFF]));

		xork3[y % 320] = xork2[y & 63] ^ uchar(rCount);

		rCount += READ_PAD32((rCount >> 32) + xork3[y % 200]);
		*((uint32 *) & xork3[remu316(rCount)]) ^=
		    (rCount >> 24) & 0xFFFFFFFF;
		if ((rCount & 0x07) == 0x03)
			y++;
		rCount -= READ_PAD8((y * y));
		if ((rCount & 0x07) == 0x01)
			y++;
		y++;
	}
	Sha256(work1, final_hash);
	results[0] = (final_hash[30] == 0 && final_hash[31] == 0
		      && final_hash[29] < 0x80);
	Sha256(xork1, final_hash);
	results[1] = (final_hash[30] == 0 && final_hash[31] == 0
		      && final_hash[29] < 0x80);
}

void BlockHash_1_mine_V1(unsigned char *p512bytes, unsigned char *final_hash,
			 unsigned char *results)
{
	//0->127   is the block header      (128)
	//128->191 is blake(blockheader)    (64)
	//192->511 is scratch work area     (320)

	uchar *work1 = p512bytes;
	uchar *work2 = work1 + 128;
	uchar *work3 = work1 + 192;

	blake512_hash(work2, work1);

#define ORKINIT(a,b,c) \
        work3[a] ^= work2[c]; \
        if(work3[a]&0x80) work3[b]=work1[(b+work3[a])&127]; \
        else              work3[b]=work2[(b+work3[a])&63]; \


	work3[0] = work2[15];
	ORKINIT(0, 1, 1);
	for (int x = 2; x < 320; x += 2) {
		ORKINIT(x - 1, x, x & 63);
		ORKINIT(x, x + 1, (x + 1) & 63);
	}
#undef ORKINIT

	uint64 counts[1];
#define qCount counts[0]
	qCount = *((uint64 *) & work3[310]);
	int nExtra = (READ_PAD8(qCount + work3[300]) >> 3) + 512;
	int x;
	for (x = 1; x < nExtra; x++) {
		qCount += READ_PAD32(qCount);

		if (qCount & 0x87878700)
			++work3[remu320(qCount)];

		qCount -= READ_PAD8(qCount + work3[remu160(qCount)]);

		if (qCount & 0x80000000) {
			qCount += READ_PAD8(qCount & 0x8080FFFF);
		} else {
			qCount += READ_PAD32(qCount & 0x7F60FAFB);
		}

		qCount += READ_PAD32(qCount + work3[remu160(qCount)]);

		if (qCount & 0xF0000000)
			++work3[remu320(qCount)];

		qCount += READ_PAD32(*((uint32 *) & work3[qCount & 0xFF]));

		work3[x % 320] = work2[x & 63] ^ uchar(qCount);

		qCount += READ_PAD32((qCount >> 32) + work3[x % 200]);

		*((uint32 *) & work3[remu316(qCount)]) ^=
		    (qCount >> 24) & 0xFFFFFFFF;

		if ((qCount & 0x07) == 0x03)
			x++;

		qCount -= READ_PAD8((x * x));

		if ((qCount & 0x07) == 0x01)
			x++;
	}
	Sha256(work1, final_hash);
	results[0] = (final_hash[30] == 0 && final_hash[31] == 0
		      && final_hash[29] < 0x80);
}
