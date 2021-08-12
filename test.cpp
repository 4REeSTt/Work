#include <iostream>

void merge(int* A, int p, int q, int r){
    int A_tmp[r]{0}; 
    int index = 0;
    int left = 0;
    int right = q;

    while(left < q && right < r){
        if(A[left] < A[right])
            A_tmp[index++] = A[left++];
        else
            A_tmp[index++] = A[right++];
    }
    
    if(left == q)
        for(int i = right; i < r; i++)
            A_tmp[index++] = A[i]; 
    else
        for(int i = left; i < q; i++)
            A_tmp[index++] = A[i];

    for(int j = 0; j < r; j++)
        A[j] = A_tmp[j];
}

void sort(int* A, int p, int r){
    if(p < r){
        int q = (p+r)/2;

        sort(A,p,q);
        sort(A,q+1,r);
        merge(A,p,q,r);
    }
}

int main(){
    int length = 8;
    int A[length]{5,2,4,6,1,3,2,6};
    sort(A,1,length);
    
    for(int i = 0; i < length; i++)
        std::cout << A[i] << " ";


    return 0;
}
