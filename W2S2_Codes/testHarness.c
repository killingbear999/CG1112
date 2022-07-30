#include <stdio.h>
#include <time.h>
#include <math.h>

//Note: Avoid using global variable in general.
// In this case, we are using it to setup a shared values to be used by
//  various function (without recompilation).
unsigned int __MAGIC;
void unitWork()
{
    int i;
    double dummy =3.14159;

    for (i = 0; i < __MAGIC; i++){
        //just do some work, dont think too much :-)
        dummy /= 2.71828;
        dummy *= 2.71828;
    }

}

void workA(int N)
{
    int i;

    for (i = 0; i < 567; i++){
        unitWork();
    }
}

void workB(int N)
{
    int i;

    for (i = 0; i < N; i++){
        unitWork();
    }
}

void workC(int N)
{
    int i, j;

    for (i = 0; i < N; i++){
        for (j = 0; j < N; j++){
            unitWork();
        }
    }
}

void workD(int N)
{
    int i;

    if (N == 0){
        unitWork();
        return;
    }

    workD( N / 2 );
    workD( N / 2 );
    for (i = 0; i < N; i++){
        unitWork();
    }
}

void workE(int N)
{
    if (N == 0){
        unitWork();
        return;
    }

    workE( N / 2 );
    unitWork();
}

int main()
{
    FILE* inf;
    clock_t start, end;
    double seconds;
    int N;

    if (!(inf = fopen("MAGIC.txt", "r"))){
        printf("Cannot read from MAGIC.txt. Exiting.\n");
        return 1;
    }

    //Read the MAGIC value to setup standard unit of work
    fscanf(inf, "%u", &__MAGIC);
    printf("Using Magic = %u\n", __MAGIC);

    //Read the "data size" as N
    printf("Data Size = ");
    scanf("%d", &N);

    printf("\n**Work Starts.... ");
    start = clock();

    //**Put actual workload here to measure time taken**

    end = clock();
    printf("Done\n");

    //Calculate elapsed time
    seconds = ((double)end - start) / CLOCKS_PER_SEC;
    printf("##Time Elapsed = %u us [%.3f s]\n", end-start, seconds);

    //Uncomment for more succint output message to help with excel plotting
    //printf("%d, %u\n", N, end-start);

    return 0;
}
