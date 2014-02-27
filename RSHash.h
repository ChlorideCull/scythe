#ifndef RSHASH_H
#define RSHASH_H

void BlockHash_Init();
void BlockHash_DeInit();
void BlockHash_1(unsigned char* p512bytes, unsigned char* final_hash);
void BlockHash_1_mine_V1(unsigned char *p512bytes, unsigned char* final_hash, unsigned char* results);
void BlockHash_1_mine_V2(unsigned char *p512bytes, unsigned char* final_hash, unsigned char* results);
void BlockHash_1_mine_V3(unsigned char *p512bytes, unsigned char* final_hash, unsigned char* results);

#endif
