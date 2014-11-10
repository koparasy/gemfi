#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mtwist.h>
#include <randistrs.h>
#include <math.h>
#include <time.h>

typedef struct fault_type{
  char faulttype[40];
  char cpu[4];
  char thread[4];
  char when[6];
  char what[6];
  long int time;
  int tcontext;
  int occurence;
  int reg;
  int bit;
  char regdec[15];
}fault_t;

const float e = 0.01;
const float c=0.99;
const int fault_types=1; //Flip Bit
const int Fetchplaces= 1;
const int DecodePlaces = 1;
const int Executeplaces = 1;
const int LoadStoreplaces = 1;
const int fault_places = 6; //PC Reg File Fetch Stage Decode stage Execute stage Load Store Unit
const char *when="Inst:";
const char *what="Flip:";

mt_state time_r, reg_r, dec_r,bit_r,choice_r;


void create_random_time(fault_t *faults,long int range, int num_of_faults){
    int i;
    for(i = 0 ;i < num_of_faults ; i++)
      faults[i].time=rds_liuniform(&time_r,0,range);
}


void print_faults(fault_t *faults, int size, char *name){
  int i;
  FILE *f = fopen(name,"w");
  for(i = 0 ; i<size; i++){
    fprintf(f,"%s %s%ld %s%d %s %s %d %d",faults[i].faulttype,faults[i].when,faults[i].time,
	   faults[i].what,faults[i].bit+1,faults[i].cpu,faults[i].thread,
	    faults[i].occurence,faults[i].tcontext);
    if(strcmp(faults[i].faulttype,"RegisterInjectedFault")==0){
      if(faults[i].reg>=8)
	fprintf(f," float %d",faults[i].reg-8);
      else
	fprintf(f," int %d",faults[i].reg);
    }
    if(strcmp(faults[i].faulttype,"RegisterDecodingInjectedFault")==0){
      fprintf(f," %s",faults[i].regdec);
    }
    fprintf(f,"\n");
  }
}

void fill_faults(fault_t *faults,int size,char *name,int range){
  int i;
  for(i = 0 ; i < size; i++ ){
    strcpy(faults[i].faulttype,name);
    strcpy(faults[i].cpu,"all\0");
    strcpy(faults[i].thread,"all\0");
    strcpy(faults[i].what,what);
    strcpy(faults[i].when,when);
    faults[i].bit = rds_liuniform(&bit_r,0,range) +1;
    faults[i].tcontext = 0;
    faults[i].occurence=1;
  }
}

void fill_register_faults(fault_t *faults, int size){
  int i;
  for( i = 0 ; i < size; i++){
    faults[i].reg = rds_liuniform(&reg_r,0,64);
  }
}

void fill_regdec_faults(fault_t *faults, int size){
  int i;
  int reg,choice;
  for( i = 0 ; i < size ; i++){
    choice=rds_liuniform(&choice_r,0,3);
    reg = rds_liuniform(&dec_r,0,8); 
    if(choice==2){
      sprintf(faults[i].regdec,"Dst0:%d",reg);
    }
    else
      sprintf(faults[i].regdec,"Src%d:%d",choice,reg);
    
  }
}

int compare(const void *a ,const void *b){
  fault_t *aa = (fault_t*) a;
  fault_t *bb = (fault_t*) b;
  
  return   bb->time - aa->time;
}


int main(int argc,char **argv){

 
  long int fetchInstr;
  long int decodeInstr;
  long int executeInstr;
  long int loadstoreInstr;
  int c;
  int total_places;
  long int total_fetchplaces;
  long int total_decodeplaces;
  long int total_executeplaces;
  long int total_loadstoreplaces;
  int reg_faults;
  int pc_faults;
  int fetch_faults;
  int decode_faults;
  int execute_faults;
  int loadstore_faults;
  long int num_of_experiments;
  char *cvalue = NULL;
  char *endptr;
  
  mts_seed32new(&time_r,time(NULL));
//  mts_seed32new(&reg_r,time(NULL));
  mts_seed32new(&dec_r,time(NULL));
  mts_seed32new(&bit_r,time(NULL));
  mts_seed32new(&choice_r,time(NULL));
  fault_t *faults;
  
  
  opterr = 0;
  
  while ((c = getopt (argc, argv, "f:d:e:l:")) != -1)
  {
    switch (c){
      case 'f':
	cvalue = optarg;
	if(cvalue)
	  fetchInstr = strtol(cvalue, &endptr, 10);
	else
	  fetchInstr= 0;	
	break;
      case 'd':
	cvalue = optarg;
	if(cvalue)
	  decodeInstr = strtol(cvalue, &endptr, 10);
	else
	  decodeInstr= 0;	
      break;
      case 'e':
	cvalue = optarg;
	if(cvalue)
	  executeInstr = strtol(cvalue, &endptr, 10);
	else
	  executeInstr= 0;
      break;
      case 'l':
	cvalue = optarg;
	if(cvalue)
	  loadstoreInstr = strtol(cvalue, &endptr, 10);
	else
	  loadstoreInstr= 0;
      break;
    }
    if ( *endptr != '\0' ) {
	  printf("Invalid input. Argument must be an integer in base 10.\n");
	  return 1;
    }
  }
  
  
  printf("\t\tFetch Instructions: %ld\n \
	    \tDecode Instructions %ld\n \
	    \tExecute Instructions %ld\n \
	    \tLoadStore Instructions %ld\n ",fetchInstr,decodeInstr,executeInstr,loadstoreInstr);
  
  total_fetchplaces = fetchInstr * Fetchplaces;
  total_decodeplaces = decodeInstr * DecodePlaces;
  total_executeplaces = executeInstr * Executeplaces;
  total_loadstoreplaces = loadstoreInstr * LoadStoreplaces;
  total_places = total_fetchplaces +total_decodeplaces + total_executeplaces + total_loadstoreplaces;
  
  printf("\n\n\t\tProbability for Fetch PC fault: \t%f\n \
	    \tProbability for Decode fault: \t\t%f\n \
	    \tProbability for Execute fault: \t\t%f\n \
	    \tProbability for Load/Store fault: \t%f\n ",
	    total_fetchplaces/(float)total_places,
	    total_decodeplaces/(float)total_places,
	    total_executeplaces/(float)total_places,
	    total_loadstoreplaces/(float)total_places);
  
  num_of_experiments = (total_places) / (1 + ((e*e)* ((total_places-1)/((c*c)*(0.5*0.5)))));
  printf("\n\n\t\tTotal Number Of Experiments : \t%ld \n",num_of_experiments);
  
  fetch_faults = ceil(((total_fetchplaces )/(float)total_places) *num_of_experiments);
  pc_faults = 0;//ceil(((total_fetchplaces/3.0 + 0.5)/(float)total_places) *num_of_experiments);
  reg_faults = 0;//ceil(((total_fetchplaces/3.0 + 0.5)/(float)total_places) *num_of_experiments);
  
  decode_faults = ceil((total_decodeplaces/(float)total_places) *num_of_experiments);
  execute_faults = ceil((total_executeplaces/(float)total_places) *num_of_experiments);
  loadstore_faults = ceil((total_loadstoreplaces/(float)total_places) *num_of_experiments);
  num_of_experiments = fetch_faults+decode_faults+execute_faults+loadstore_faults;
  printf("\n\n\t\tTotal Number Of Upper Bounded Experiments : \t%ld \n",num_of_experiments);
  
  printf("\t\tFetch faults: \t\t%d\n\
	    \tReg faults: \t\t%d\n\
	    \tPC faults: \t\t%d\n \
	    \tDecode faults: \t\t%d\n \
	    \tExecute faults: \t%d\n \
	    \tLoad/Store faults: \t%d\n",
	    fetch_faults,
	    reg_faults,
	    pc_faults,
	    decode_faults,
	    execute_faults,
	    loadstore_faults);
  
  faults = (fault_t*) malloc (sizeof(fault_t)*num_of_experiments);
  create_random_time(faults,fetchInstr,fetch_faults);
  
  fill_faults(faults,fetch_faults,"GeneralFetchInjectedFault",32);
//  fill_faults(&faults[fetch_faults],reg_faults,"RegisterInjectedFault",64);
 // fill_register_faults(&faults[fetch_faults],reg_faults);
  //fill_faults(&faults[fetch_faults+reg_faults],pc_faults,"PCInjectedFault",64);
  
  create_random_time(&faults[fetch_faults],decodeInstr,decode_faults);
  fill_faults(&faults[fetch_faults],decode_faults,"RegisterDecodingInjectedFault",64);
  fill_regdec_faults(&faults[fetch_faults],decode_faults);
  
  create_random_time(&faults[fetch_faults+decode_faults],executeInstr,execute_faults);
  fill_faults(&faults[fetch_faults+decode_faults],execute_faults,"IEWStageInjectedFault",64);
  
  create_random_time(&faults[fetch_faults+decode_faults+execute_faults],loadstoreInstr,loadstore_faults);
  fill_faults(&faults[fetch_faults+decode_faults+execute_faults],loadstore_faults,"LoadStoreInjectedFault",64);
  
  
  qsort(&faults[0], fetch_faults ,sizeof(fault_t),compare);
  print_faults(&faults[0],fetch_faults,"Fetch.txt");

 
  qsort(&faults[fetch_faults], decode_faults ,sizeof(fault_t),compare);
  print_faults(&faults[fetch_faults],decode_faults,"Decode.txt");

 
  qsort(&faults[decode_faults+fetch_faults], execute_faults ,sizeof(fault_t),compare);
  print_faults(&faults[decode_faults+fetch_faults],execute_faults,"IEW.txt");

 
  qsort(&faults[decode_faults+fetch_faults + execute_faults], loadstore_faults ,sizeof(fault_t),compare);
  print_faults(&faults[decode_faults+fetch_faults + execute_faults],loadstore_faults,"LDS.txt");




  return 0;
}
