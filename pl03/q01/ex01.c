#include <stdio.h>
#include<unistd.h>

int main () {

    long i,j;
    double soma;
    double arraysoma [5000];
    double *ponteiro;
    ponteiro=arraysoma;

    for (j=0; j<10000; j+=2){
        soma=0;
        for (i=0;i<20000;i++)
            soma +=1.0/(i+j);
        *ponteiro=soma;
        ponteiro++;
    }

    for ( j =0; j <=5000; j ++)
        printf ( "%f \n", arraysoma [j]);

    sleep(60);

    printf("I am ready to terminate , press [ ENTER ] to exit .\n");

    getchar();

    return 0;
}