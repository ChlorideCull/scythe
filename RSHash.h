#ifndef RSHASH_H
#define RSHASH_H

void BlockHash_Init();
void BlockHash_DeInit();
bool BlockHash_1(unsigned char *p512bytes, unsigned char* final_hash);

#endif
