#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAXLENGTH 32
#define MAXWORD 220000

void sort(char a[][MAXLENGTH], int N)
{
    int i, j;
    char next[MAXLENGTH];

    for ( i = 1; i < N; ++i) {
        strcpy(next, a[i]);

        for ( j = i-1; j >= 0 && (strcmp(a[j],next) > 0); --j){
            strcpy(a[j+1], a[j]);
        }
        strcpy(a[j+1], next);
    } 
}

int readDictionary(char fname[], char dictionary[][MAXLENGTH], int max)
{
    int count = 0;
    FILE* infile;

    if(!( infile = fopen( fname, "r"))){
        return 0;
    }

    while (count < max && fscanf(infile,"%s", dictionary[count++]) == 1);

    return count;
}

int binarySearch( char a[][MAXLENGTH], int N, char W[] ) {
    int idxOfX = -1, mid, low, high;
    int strCmpResult;

    low = 0;
    high = N - 1;
    while ( (low <= high) && (idxOfX == -1) ) {
        mid = (low + high) / 2;

        strCmpResult = strcmp( a[mid], W);

        if ( strCmpResult == 0 ) {
            idxOfX = mid;
        } else if ( strCmpResult < 0 ){
            low = mid + 1;
        } else {
            high = mid - 1;    
        }
    }

    return idxOfX;
}

int queryWordList(char fname[], int* nQuery, 
                char dictionary[][MAXLENGTH], int size)
{
    FILE* infile;
    int count = 0, found = 0;
    char word[MAXLENGTH];

    if(!( infile = fopen( fname, "r"))){
        return 0;
    }

    while (count < *nQuery && fscanf(infile,"%s", word) == 1) {
        count++;
        if (binarySearch(dictionary, size, word) != -1){
            found++;
        }
    }

    //Update the number query if we cannot perform all
    if (count != *nQuery){
        *nQuery = count;
    }

    return found;
}


int main(int argc, char**argv)
{
    char dictionary[MAXWORD][MAXLENGTH];
    int dSize = 0, N, success, nQuery = 10000;

    clock_t start, mid, end;
    double sortTime, searchTime;

    //Read the "data size" as N
    printf("Data Size = ");
    scanf("%d", &N);

    dSize = readDictionary("words_random.txt", dictionary, N);
    printf("Read %d words\n", dSize);

    //Take note of the starting time
    start = clock();

    sort(dictionary, dSize);

    //Capture time needed for sort()
    mid = clock();

    success = queryWordList("words_query.txt", &nQuery, dictionary, dSize);

    //Capture the ending time
    end = clock();

    //Print the statistic
    sortTime = ((double)mid - start) / CLOCKS_PER_SEC;
    searchTime = ((double)end - mid) / CLOCKS_PER_SEC;

    printf("##Time Elapsed = Sort{%.3f s} + Search{%.3f s]} = %.3f s\n", 
        sortTime, searchTime, sortTime+searchTime);

    printf("Found %d out of %d queries in dictionary of %d words\n", 
        success, nQuery, dSize);

    //TODO: You may want to use a simplified output format to aid excel
    //      plotting

    return 0;
}
