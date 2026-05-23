#include <stdio.h>
#include "../../include/gf257.h"

int lagrange_l0(int k, int *vx, int i){
    int ans = 1;
    int aux = 0;
    for(int j=0; j<k; j++){
        if(j!=i){
            if(!gf257_div(gf257_sub(0,vx[j]),gf257_sub(vx[i],vx[j]),&aux)){
                return -1;
            }
            ans = gf257_mul(ans, aux);
        }
    }
    return ans;
}

int lagrange_p0(int k, int *vx, int *vy){
    int ans = 0;
    int aux;
    for(int i = 0; i < k; i++){
        aux = lagrange_l0(k, vx, i);
        if(aux == -1){
            return -1;
        }
        ans = gf257_add(gf257_mul(aux,vy[i]),ans);
    }
    return ans;
}

int decypher_lagrange(int k, int* vx, int* vy){
    if(k<=1){
        return vy[0];
    }
    int s1 = lagrange_p0(k, vx, vy);
    if(s1==-1){
        return -1;
    }
    for(int i=0; i<k-1; i++){
        if(!gf257_div(gf257_sub(vy[i],s1),vx[i], &(vy[i]))){
            return -1;
        }
    }
    return decypher_lagrange(k-1, vx, vy);
}