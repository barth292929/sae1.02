
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 10
#define RANDOM_MAX 100
typedef int tableau[N];

int partition(tableau t,int debut,int fin,int pivot, int *nbPermut);
void triRapide(tableau t, int debut, int fin);
void initRandom(tableau t);
void affiche(tableau t);
void permuter(int *a, int *b);

int main()
{
    tableau Tab;
    int debut, fin, pivot, Permutation;
    Permutation = 0;
    debut=0;
    fin=N-1;
    pivot = (debut+fin)/2;
    initRandom(Tab);
    affiche(Tab);
    triRapide(Tab, debut, fin);
    affiche(Tab);
    return 0;
}


int partition(tableau t,int debut,int fin,int pivot, int *nbPermut)
{
    int j;
    permuter(&t[pivot], &t[fin]);
    *nbPermut = *nbPermut+1;
    j= debut;
    for (int i=debut; i<fin ; i++)
    {
        if (t[i] <= t[fin])
        {
            permuter(&t[i], &t[j]);
            *nbPermut = *nbPermut+1;
            j = j+1;
        }
    }
    permuter(&t[fin], &t[j]);
    *nbPermut = *nbPermut+1;
    return j;
}


void triRapide(tableau t, int debut, int fin){
int pivot;
if (debut < fin)
{
    pivot = (debut+fin)/2;
    pivot = partition(t, debut, fin, pivot, Permut);
    triRapide(t, debut, pivot-1);
    triRapide(t, pivot+1, fin);
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
        t[i]= rand() % RANDOM_MAX;
    }
}

void permuter(int *a, int *b)
{
    int temp;
    temp=*a;
    *a=*b;
    *b=temp;
}