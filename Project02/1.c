#include<stdio.h>
int main() {
  float fa[64], d = 0.9;
  int n = 64, i;
  for ( i=0; i<64; i++ ){
    fa[i] = fa[i] * d + 0.5;
    printf("fa[%d]: %f\n",i,fa[i]);
  }
  return 0;
}