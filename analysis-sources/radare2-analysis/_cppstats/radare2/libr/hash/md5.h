void MD5_Init(MD5_CTX *);
void MD5_Update(MD5_CTX *, const ut8*, unsigned int);
void MD5_Final(ut8 [16], MD5_CTX *);
