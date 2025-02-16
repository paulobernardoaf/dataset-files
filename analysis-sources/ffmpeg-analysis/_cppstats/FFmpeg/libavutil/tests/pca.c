#include "libavutil/pca.c"
#include "libavutil/lfg.h"
#undef printf
#include <stdio.h>
#include <stdlib.h>
int main(void){
PCA *pca;
int i, j, k;
#define LEN 8
double eigenvector[LEN*LEN];
double eigenvalue[LEN];
AVLFG prng;
av_lfg_init(&prng, 1);
pca= ff_pca_init(LEN);
for(i=0; i<9000000; i++){
double v[2*LEN+100];
double sum=0;
int pos = av_lfg_get(&prng) % LEN;
int v2 = av_lfg_get(&prng) % 101 - 50;
v[0] = av_lfg_get(&prng) % 101 - 50;
for(j=1; j<8; j++){
if(j<=pos) v[j]= v[0];
else v[j]= v2;
sum += v[j];
}
ff_pca_add(pca, v);
}
ff_pca(pca, eigenvector, eigenvalue);
for(i=0; i<LEN; i++){
pca->count= 1;
pca->mean[i]= 0;
for(j=i; j<LEN; j++){
printf("%f ", pca->covariance[i + j*LEN]);
}
printf("\n");
}
for(i=0; i<LEN; i++){
double v[LEN];
double error=0;
memset(v, 0, sizeof(v));
for(j=0; j<LEN; j++){
for(k=0; k<LEN; k++){
v[j] += pca->covariance[FFMIN(k,j) + FFMAX(k,j)*LEN] * eigenvector[i + k*LEN];
}
v[j] /= eigenvalue[i];
error += fabs(v[j] - eigenvector[i + j*LEN]);
}
printf("%f ", error);
}
printf("\n");
for(i=0; i<LEN; i++){
for(j=0; j<LEN; j++){
printf("%9.6f ", eigenvector[i + j*LEN]);
}
printf(" %9.1f %f\n", eigenvalue[i], eigenvalue[i]/eigenvalue[0]);
}
return 0;
}
