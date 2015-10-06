#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"
#include <sys/stat.h>
#include <limits.h>

void
usage(char *prog) 
{
    fprintf(stderr, "usage: %s <-i inputfile> <-o outputfile> <-l lowvalue> <-h highvalue>\n", prog);
    exit(1);
}

int compare (const void * a, const void * b)
{

  rec_t *elemA = (rec_t *)a;
  rec_t *elemB = (rec_t *)b;

  return ( elemB->key - elemA->key );
}

int
main(int argc, char *argv[])
{
    // program assumes a 4-byte key in a 100-byte record
    assert(sizeof(rec_t) == 100);

    long long int lowvalue, highvalue;
    char *inFile   = "/no/such/file";
    char *outFile = "/no/such/file";
    // input params
    int c;
    opterr = 0;
  
  while ((c = getopt(argc, argv, "i:o:l:h:")) != -1) {
  switch (c) {
  case 'i':
      inFile = strdup(optarg);
      break;
  case 'o':
      outFile  = strdup(optarg);
      break;
  case 'l':
      lowvalue = atoll(optarg);
      if (lowvalue > UINT_MAX || lowvalue < 0){
        fperror(stderr, "Error: Invalid range value");
        exit(1);
      }
      else if(strlen(optarg) > lowvalue+1){
        fprintf(stderr, "Error: Invalid range value");
        exit(1);
      }
      break;
  case 'h':
      highvalue = atoll(optarg);
      if (highvalue > UINT_MAX || highvalue <0){
        fprintf(stderr, "Error: Invalid range value");
        exit(1);
      }
      else if(strlen(optarg) > highvalue+1){
        fprintf(stderr, "Error: Invalid range value");
        exit(1);
      }
      break;
  default:
      usage(argv[0]);
    }
  }
  
  printf("#########\n");
  printf("%lld \n",lowvalue);
  printf("%lld \n",highvalue);
  printf("##########\n");

  if (lowvalue > highvalue){
    fprintf(stderr, "Error: Invalid range value");
    exit(1);
  }

  // open and create output file
  int fd = open(inFile, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "File Open Error");
    exit(1);
  }

  struct stat buf;          

  stat(inFile, &buf);
  int size = buf.st_size;

  rec_t * inputData;

  inputData = (rec_t *) malloc(size);
  
  int numElements = 0;

  while (1) { 
    rec_t r;
    int rc;
    rc = read(fd, &r, sizeof(rec_t));
    if (rc == 0) // 0 indicates EOF
        break;
    if (rc < 0) {
        perror("read");
        exit(1);
    }

    if (r.key >= lowvalue && r.key <= highvalue) {
      inputData[numElements] = r;
      numElements++;
    }

    }

    //Apply quick sort here ....

    qsort(inputData, numElements, sizeof(rec_t), compare);

    int k;
    for(k = (numElements - 1);k>=0;k--) {
      printf("%u ", inputData[k].key);
      printf("\n");
    
    }

    //Write in output file

    int fp = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
    if (fp < 0) {
      perror("open");
      exit(1);
    }

    int w;
    for (w=(numElements-1);w>=0;w--){

    int x = write(fp, &inputData[w], sizeof(rec_t));
    if (x != sizeof(rec_t)) {
      perror("write");
      exit(1);
    }
  }

  (void) close(fd);
  (void) close(fp);
  free(inputData);

  return 0;
}
