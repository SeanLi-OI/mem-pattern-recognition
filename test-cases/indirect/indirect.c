#include<stdlib.h>
const int N=10000;
int main(){
    int A[N],B[N];
    for(int i=0;i<N;i++){
        A[i]=rand()%N;
        B[i]=rand()%N;
    }
    int t;
    for(int i=0;i<N;i++){
        t=A[B[i]]; // indirect
    }
    return 0;
}