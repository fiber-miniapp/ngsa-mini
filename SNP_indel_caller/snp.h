/*
	File:	snp.h
	Copyright(C) 2012-2013 RIKEN, Japan.
*/
#define _POSIX_C_SOURCE 2
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

extern int total;//行数
extern char chr[];

extern FILE *SNP_fp;
extern FILE *INDEL_fp;

// intput and output files
extern char  *REALIGNFile; // output realign file

// global variable and default value
extern int    LOWER_COV               ;    // minimum depth filter
extern int    UPPER_COV               ;   // maximum depth filter
extern double CUT_OFF_FRQ             ; // unused
extern int    CUT_OFF_Q               ;   // base quality cut off
extern int    INDEL_RANGE             ;    // neighbor indel range
extern int    SNP_RANGE               ;    // unused
extern int    INDEL_LOWER_LIMIT       ;    // indel minimum depth
extern int    NEIGHBOR_SNP_RANGE      ;    // neighbor snp range
extern int    NEIGHBOR_SNP_NUM        ;    // neighbor snp count
extern int    CUT_OFF_NUM             ;    // unused
extern int    MQ_CUTOFF               ;   // mapping quality cuf off
extern int    CUT_OFF_Q2              ;   // unused
extern int    CUT_OFF_NUM2            ;    // unused
extern double INDEL_ALLELE_FRQ_CUTOFF ;  // indel allele frequency cut off
extern double LOSS                    ; // plausibility threshold
extern double CONSENSUS_Q             ;   // consensus quality cut off
extern double SNP_Q                   ;   // snp quality cut off
extern int    INDEL_Q_SCORE           ;   // indel quality score cut off
extern int    INDEL_WINDOW            ;   // indel window size
extern int    ROWS_FOR_TEST;
extern int    REQUIRED_BQ             ;   // base quality required for each allele at least 1 read
