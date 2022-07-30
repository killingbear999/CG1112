#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAXLENGTH 32
#define MAXWORD 220000

void M(char a[][MAXLENGTH], int low, int mid, int high) 
//Purposely modified to a obsure fashion for CG1112 :-)
// Dont write like this for actual assignment / work
{
    int n = high-low+1, k;

    char **b;
    int left=low, right=mid+1, bIdx=0, i;

    b = (char**) malloc(sizeof(char*) * n);

    for (i = 0; i < n; i++){
        b[i] = (char*) malloc(sizeof(char)* MAXLENGTH);
    }

    while (left <= mid && right <= high) {
        if (strcmp(a[left], a[right]) < 0) {
            strcpy(b[bIdx++], a[left++]);
        } else {
            strcpy(b[bIdx++], a[right++]);
        }
    } 

    while (left<=mid) strcpy(b[bIdx++], a[left++]);
    while (right<=high) strcpy(b[bIdx++], a[right++]);

    for (k = 0; k < n; ++k) 
        strcpy(a[low+k], b[k]);

    for (i = 0; i < n; i++){
        free(b[i]);
    }
    free(b);

}

void MSort (char a[][MAXLENGTH], int low, int high)
{
    if (low < high) {
        int mid = (low+high)/2;	

        MSort(a, low, mid);	
        MSort(a, mid+1, high);

        M(a, low, mid, high);	
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
    double sortTime, searchTime, totalTime;

    //Read the "data size" as N
    printf("Data Size = ");
    scanf("%d", &N);

    dSize = readDictionary("words_random.txt", dictionary, N);
    printf("Read %d words\n", dSize);

    //Take note of the starting time
    start = clock();

    //Use mysterious sorting function
    MSort(dictionary, 0, dSize-1);

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
