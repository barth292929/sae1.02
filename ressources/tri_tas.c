
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 300000
typedef int tableau[N];

void initRandom(tableau t);
void affiche(tableau t);
void permuter(int *a, int *b);
void tamiser(tableau t, int noeud, int n, int *nb);
void triParTas(tableau t, int *nombre);

int main()
{
    tableau Tab;
    int nbPermut;
    nbPermut =0;
    clock_t begin = clock();
    initRandom(Tab);
    triParTas(Tab, &nbPermut);
    affiche(Tab);
    printf("\n");
    printf("il y a %d permutations\n", nbPermut);
    clock_t end = clock();
    double tmpsCPU = ((end - begin)*1.0) / CLOCKS_PER_SEC;
    printf( "Temps CPU = %.3f secondes\n",tmpsCPU);
    return 0;
}


void tamiser(tableau t, int noeud, int n, int *nb)
{
    int fils;
    fils = 2*noeud + 1;
    if ( fils < n && t[fils+1] > t[fils] )
    {
        fils++;
    }
    if ( fils <= n && t[noeud] < t[fils] )
    {
    permuter(&t[noeud], &t[fils]);
    *nb=*nb+1;
    tamiser(t,fils,n, nb);
    }
}



void triParTas(tableau t, int *nombre)
{
    for (int i  = N/2 - 1; i>0; i--)
    {
        tamiser(t,i,N-1, nombre);
    }
    for (int i=N-1; i>0; i--)
    {
        permuter(&t[0], &t[i]);
        *nombre=*nombre+1;
        tamiser(t, 0, i-1, nombre);
    }
}


void affiche(tableau t)
{
    for (int i=0; i<N; i++)
    {
        printf("%d\t", t[i]);
    }
    printf("\n");
}


void initRandom(tableau t)
{
    int i;
    srand(time(NULL));
    for(i=0; i<N-1; i++)
    {
        t[i]= rand();
    }
}


void permuter(int *a, int *b)
{
    int temp;
    temp=*a;
    *a=*b;
    *b=temp;
}