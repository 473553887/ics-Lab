//ID:5130379017 Name:﻿李晟 
#include "cachelab.h"
#include<getopt.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
int verbose=0;
typedef struct
{
	int *unusetime;
	int *tag;
	char* notempty;
}Set;

void readarg(FILE *fp ,char* op,unsigned long long* address)
{
	*op=0;
	*address=0;
	fscanf(fp,"%s",op);
	char buffer[100];
	fscanf(fp,"%s",buffer);
	*address=strtoll(buffer,NULL,16);
	if(verbose&&(*op=='L'||*op=='S'||*op=='M'))
		printf("%s %s ",op,buffer);
}

void cachesim(FILE *fp,int setindex,int linenum,int blockbits,int* hits,int* miss,int* evictions)
{
	char op; 
	unsigned long long address;
	Set *sets;
	int setnum=1<<setindex;
	int setmask=0;
	for(int i=0;i<setindex;i++)
	{
		setmask|=1<<i;
	}
//inital sets
	sets=(Set*)malloc(sizeof(Set)*setnum);
	for(int i=0;i<setnum;i++)
	{
		sets[i].unusetime=(int *)malloc(sizeof(int)*linenum);
		sets[i].tag=(int*)malloc(sizeof(int)*linenum);
		sets[i].notempty=(char*)malloc(linenum);
		memset(sets[i].unusetime,0,sizeof(int)*linenum);
		memset(sets[i].tag,0,sizeof(int)*linenum);
		memset(sets[i].notempty,0,linenum);
	}
//simulate cache
	while(!feof(fp))
	{
		int line=0;
		int set=0;
		int tag=0;
		int ifhit=0;
		readarg(fp,&op,&address);
		switch(op)
		{
		case'L':case'S':
			line=0;
			ifhit=0;
			set=(address>>blockbits)&setmask;
			tag=address>>(blockbits+setindex);
			for(int i=0;i<linenum;i++)
			{
				if(sets[set].notempty[i])
				{
					if(sets[set].tag[i]==tag)
					{
						ifhit=1;
						(*hits)++;
						sets[set].unusetime[i]=0;
						if(verbose)
							printf("hit \n");
					}
					else
						sets[set].unusetime[i]++;
				}
			}
			if(!ifhit)
			{
				if(verbose)
					printf("miss ");
				(*miss)++;
				for(int i=0;i<linenum;i++)
				{
					if(!sets[set].notempty[i])
					{
						line=i;
						break;
					}
					if(sets[set].unusetime[line]<sets[set].unusetime[i])
						line=i;
				}
				if(sets[set].notempty[line])
				{
					(*evictions)++;
					if(verbose)
						printf("eviction ");
				}
				sets[set].tag[line]=tag;
				sets[set].unusetime[line]=0;
				sets[set].notempty[line]=1;
				if(verbose)
					printf("\n");
			}
			break;
		case 'M':
			line=0;
			ifhit=0;
			set=(address>>blockbits)&setmask;
			tag=address>>(blockbits+setindex);
			for(int i=0;i<linenum;i++)
			{
				if(sets[set].notempty[i])
				{
					if(sets[set].tag[i]==tag)
					{
						ifhit=1;
						(*hits)+=2;
						sets[set].unusetime[i]=0;
						if(verbose)
							printf("hit hit \n");
					}
					else
						sets[set].unusetime[i]++;
				}
				
			}
			if(!ifhit)
			{
				(*miss)++;
				if(verbose)
					printf("miss ");
				for(int i=0;i<linenum;i++)
				{
					if(!sets[set].notempty[i])
					{
						line=i;
						break;
					}
					if(sets[set].unusetime[line]<sets[set].unusetime[i])
						line=i;
				}
				if(sets[set].notempty[line])
				{
					(*evictions)++;
					if(verbose)
						printf("eviction ");
				}
				(*hits)++;
				if(verbose)
					printf("hit \n");
				sets[set].tag[line]=tag;
				sets[set].unusetime[line]=0;
				sets[set].notempty[line]=1;
			}
			break;
		}
			
	}
//free sets	
	for(int i=0;i<setnum;i++)
	{
		free(sets[i].unusetime);
		free(sets[i].tag);
		free(sets[i].notempty);
	}
	free(sets);
	
}
void printmessage()
{
	printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\nOptions:\n  -h         Print this help message.\n  -v         Optional verbose flag.\n  -s <num>   Number of set index bits.\n  -E <num>   Number of lines per set.\n  -b <num>   Number of block offset bits.\n  -t <file>  Trace file.\n\nExamples:\n  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}
int main(int argc,char** argv)
{
	int setindex=0;
	int linenum=0;
	int blockbits=0;
	int hits=0;
	int miss=0;
	int evictions=0;
	char fname[100];
	FILE *fp;
	int ch;
	int argnum=0;
	while(argnum<4)
	{
		if((ch = getopt(argc,argv,"hvs:E:b:t:"))==-1)
			break;
		switch(ch)
		{
		case'h':
			printmessage();
			return 0;
		case'v':
			verbose=1;
			break;
		case's':
			setindex=atoi(optarg);
			argnum++;
			break;
		case'b':
			blockbits=atoi(optarg);
			argnum++;
			break;
		case'E':
			linenum=atoi(optarg);
			argnum++;
			break;
		case't':
			strcpy(fname,optarg);
			argnum++;
			break;
		}
	
	}
	if(optopt!=0)
	{
		printmessage();
		return 0;
	}
	else if(argnum<4)
	{
		printf("Missing required command line argument\n");
		printmessage();
		return 0;
	}
	fp=fopen(fname,"r");
	if(!fp)
	{
		printf("failed to open the file %s\n",fname);
		return 0;
	}
	cachesim(fp,setindex,linenum,blockbits,&hits,&miss,&evictions);	
    	printSummary(hits, miss, evictions);
    	return 0;
}
