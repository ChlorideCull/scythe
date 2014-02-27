#ifndef OMASHA256_H
#define OMASHA256_H

void Sha256(unsigned char* in, unsigned char* out);
void Sha256_round(uint* s, unsigned char* data);
void Sha256_round_padding(uint* s);
void SWeird(uint* s, const uint* const pad);

#endif
