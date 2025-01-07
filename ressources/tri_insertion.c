#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 300000
typedef int tablo[N];

void tri_insertion(tablo tableau);
void affiche(tablo tableau);
void initRandom(tablo tableau);

int main()
{
    tablo T;
    clock_t begin = clock();
    initRandom(T);
    tri_insertion(T);
    affiche(T);
    clock_t end = clock();
    double tmpsCPU = ((end - begin)*1.0) / CLOCKS_PER_SEC;
    printf( "Temps CPU = %.3f secondes\n",tmpsCPU);
    return 0;
}



void tri_insertion(tablo tableau)
{
    int i, x, j;
for (i=1; i<N-1; i++)
{
    x=tableau[i];
    j=i;
    while(j>0 && tableau[j-1]>x)
    {
        tableau[j]=tableau[j-1];
        j=j-1;
    }
    tableau[j]=x;
}
}


void affiche(tablo tableau)
{
    for (int i=0; i<N; i++)
    {
        printf("%d\t", tableau[i]);
    }
}

void initRandom(tablo tableau)
{
    int i;
    srand(time(NULL)); // Initialise le générateur de nombres aléatoires
    for(i=0; i<N-1; i++)
    {
        tableau[i]= rand();
    }
}