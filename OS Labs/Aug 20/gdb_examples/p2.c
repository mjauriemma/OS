#include<stdio.h>  
 
int main(int argc, char *argv[])  
{  
        int a[10]; 
        
        int *a1;  
         
 
        a1 = a;  
 
        //a1 = 0;  
 
        a1[0] = 1;  
        printf("a1[0] = %d\n", a1[0]);
 
        return 0;  
}
