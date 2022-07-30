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

int main()
{
    FILE* inf;
    clock_t start, end;
    double seconds;
    int i;

    if (!(inf = fopen("MAGIC.txt", "r"))){
        printf("Cannot read from MAGIC.txt. Exiting.\n");
        return 1;
    }

    //Read the MAGIC value to setup standard unit work
    fscanf(inf, "%u", &__MAGIC);
    printf("Using Magic = %u\n", __MAGIC);

    for (i = 0; i < 3; i++){
        printf("\n**Rout %d Work Starts\n", i);
        start = clock();

        //Put the actual workload function here
        unitWork();        
        unitWork();        
        unitWork();        
        unitWork();        
        unitWork();        

        end = clock();
        printf("**Work Ended\n");

        //Calculate elapsed time
        seconds = ((double)end - start) / CLOCKS_PER_SEC;
        printf("##Time Elapsed = %u us [%.3f s]\n", end-start, seconds);
    }
    printf("Each round above should take about 0.05 second\n");

    return 0;
}
