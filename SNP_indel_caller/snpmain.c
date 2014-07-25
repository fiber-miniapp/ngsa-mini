/*
	File:	snpmain.c
	Copyright(C) 2012-2013 RIKEN, Japan.
*/
#define _POSIX_C_SOURCE 2
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <math.h>
#include <stdarg.h>

const int PILE_BUFSIZE = 1024;
const int SA_INITSIZE  = 1024;
const int SA_INITSIZE2 = 25600;

#define KAIJYOU_MAX 1000
#define CHR_BUFSIZE 1024

enum SNP_INDEL { SNP_T, INDEL_T };
int total = 0;
char chr[CHR_BUFSIZE] = {0};

FILE *SNP_fp   = NULL;
FILE *SUM_fp   = NULL;
FILE *INDEL_fp = NULL;

// intput and output files
char  *INFILE                  = NULL; // input pileup file
char  *INDELFile               = NULL; // output indel file
char  *SNPFile                 = NULL; // output snp file
char  *SUMFile                 = NULL; // output summary file
char  *REALIGNFile             = NULL; // output realign file

// global variable and default value
int    LOWER_COV               = 5;    // minimum depth filter
int    UPPER_COV               = 75;   // maximum depth filter
double CUT_OFF_FRQ             = 0.15; // unused
int    CUT_OFF_Q               = 10;   // base quality cut off
int    INDEL_RANGE             = 5;    // neighbor indel range
int    SNP_RANGE               = 3;    // unused
int    INDEL_LOWER_LIMIT       = 4;    // indel minimum depth
int    NEIGHBOR_SNP_RANGE      = 5;    // neighbor snp range
int    NEIGHBOR_SNP_NUM        = 3;    // neighbor snp count
int    CUT_OFF_NUM             = 0;    // unused
int    MQ_CUTOFF               = 20;   // mapping quality cuf off
int    CUT_OFF_Q2              = 30;   // unused
int    CUT_OFF_NUM2            = 1;    // unused
double INDEL_ALLELE_FRQ_CUTOFF = 0.1;  // indel allele frequency cut off
double LOSS                    = 5000; // plausibility threshold
double CONSENSUS_Q             = 60;   // consensus quality cut off
double SNP_Q                   = 60;   // snp quality cut off
int    INDEL_Q_SCORE           = 10;   // indel quality score cut off
int    INDEL_WINDOW            = 10;   // indel window size
int    ROWS_FOR_TEST;
int    REQUIRED_BQ             = 30;   // base quality required for each allele at least 1 read

struct getoption {
  char *option;
  void *ptr;
};

struct getoption GetOptions[] = {
  { "Infile=s",  &INFILE },
  { "INDEL=s",   &INDELFile },
  { "SNP=s",     &SNPFile },
  { "SUM=s",     &SUMFile },
};

// SNP, indel
typedef struct _TypeSNP {
    char   *gicode;
    int    position;
    char   kchar;
    char   *sstring;
    int    key1;
    int    key2;
    int    key3;
    int    key4; /* indel not use */
    int    key5; /* indel not use */
    double d1;
    double d2;
} TypeSNP;

double kaijyou(int n)
{
  static double bank[KAIJYOU_MAX] = {1,1};
  static int nmax=1;
  int i;
  double a;
  if (n < 0) {
    return 1.0;
  }
  else if (n <= nmax) {
    return bank[n];
  }
  a = bank[nmax];
  for (i=nmax+1;i<=n;i++) {
    a *= i;
    bank[i] = a;
  }
  nmax = n;
  return a;
}

double roundc(double a, int digit)
{
  double b = 1.0;
  int i;
  double c;
  char s[10];
  for(i=1;i<digit;i++) {
    b *= 10.0;
  }
  c = a*b+0.5;
  sprintf(s,"%5.1f\n",c); // hack for intel 80 bit problem
  return floor(c)/b;
}

struct indel_frq {
  enum SNP_INDEL snp_indel;
  char *geno;
  int cov;
  int ref_allele_num;
  int indel_allele_num;
  double ref_allele_frq;
  double indel_allele_frq;
};

int indel_frq_Q(struct indel_frq *p,
                char *indel_allele, int ref_allele_num,
                int indel_allele_num,
                double INDEL_ALLELE_FRQ_CUTOFF, int INDEL_LOWER_LIMIT)
{
  int ret = 0;
  char s[256];
  p->snp_indel = INDEL_T;
  p->cov = indel_allele_num + ref_allele_num;
  p->ref_allele_num   = ref_allele_num;
  p->indel_allele_num = indel_allele_num;
  p->ref_allele_frq   = roundc((double)ref_allele_num/p->cov, 3);
  p->indel_allele_frq = roundc((double)indel_allele_num/p->cov, 3);
  sprintf( s, "%g", p->indel_allele_frq ); // hack for intel 80 bit problem
  if ( p->indel_allele_frq > INDEL_ALLELE_FRQ_CUTOFF &&
       indel_allele_num >= INDEL_LOWER_LIMIT ) {
    p->geno = indel_allele;
  }
  else {
    p->geno = NULL;
    ret = -1;
  }
  return ret;
}

struct strarray {
  int length;   //要素の数 scalar
  char *data;   //データ入っている箇所
  long datasize; //データのサイズ(allocsz)
  long datatop;  //領域の使用済みの次の先頭
  char *current_ptr; //アクセスする場合に現在の場所を覚えている
  int current_idx; //アクセスしている要素番号
};

void sa_init(struct strarray *p, int datasize)
{
  p->length = 0;
  if (datasize > 0) {
    p->data = (char *)malloc(datasize);
  }
  else {
    p->data = NULL;
  }
  p->datasize = datasize;
  p->datatop = 0;
  p->current_ptr = p->data;
  p->current_idx = 0;
}

void sa_clear(struct strarray *p)
{
  p->length = 0;
  p->datatop = 0;
  p->current_ptr = p->data;
  p->current_idx = 0;
}

void sa_free(struct strarray *p)
{
  free(p->data);
  sa_init(p, 0);
}

void sa_extend(struct strarray *p, int add)
{
  if (p->datatop + add > p->datasize) {
    long nc = p->current_ptr - p->data;
    if (p->datasize == 0) {
      sa_init(p, add*10000);
    }
    else {
      p->datasize = p->datasize * 3 / 2;
      p->data = (char *)realloc(p->data, p->datasize);
    }
    p->current_ptr = p->data + nc;
  }
}

void sa_push(struct strarray *p, char *str)
{
  int add = strlen(str) + 1;
  sa_extend(p, add);
  memcpy(p->data+p->datatop, str, add);
  p->datatop += add;
  p->length++;
}

void sa_push_c(struct strarray *p, int c)
{
   char s[2] = { c, '\0' };
   sa_push(p, s);
}

void sa_putc(struct strarray *p, int c)
{
  sa_extend(p, 1);
  p->data[p->datatop-1] = c;
  p->data[p->datatop]   = '\0';
  p->datatop++;
}

int sa_scalar(struct strarray *p)
{
  return p->length;
}

const char *sa_get(struct strarray *p, int idx)
{
  int i;
  if (p == NULL || idx < 0 || idx >= p->length) {
    return NULL;
  }
  if (idx < p->current_idx) {
    i = 0;
    p->current_ptr = p->data;
  }
  else {
    i = p->current_idx;
  }
  for (;i<idx;i++) {
    char *q;
    for (q=p->current_ptr;q<p->data+p->datasize;q++) {
      if (*q == '\0') break;
    }
    p->current_ptr = q + 1;
  }
  p->current_idx = idx;
  return p->current_ptr;
}

int sa_maxfrq(struct strarray *p, int *idx_max)
{
  int n = sa_scalar( p );
  if (n == 0) return 0;
  {
    int i,j;
    int maxfrq;
    int frq[n];
    for (i=0;i<n;i++) {
      frq[i] = 0;
    }
    for (i=0;i<n;i++) {
      const char *si = sa_get( p, i );
      if ( frq[i] < 0 ) continue;
      frq[i]++;
      for (j=i+1;j<n;j++) {
        const char *sj = sa_get( p, j );
        if ( frq[j] < 0 ) continue;
        if ( strcmp( si, sj ) == 0 ) {
          frq[i]++;
          frq[j]--;
        }
      }
    }
    maxfrq = frq[0];
    *idx_max = 0;
    for (i=1;i<n;i++) {
      if ( frq[i] > maxfrq ) {
        maxfrq = frq[i];
        *idx_max = i;
      }
    }
    return maxfrq;
  }
}

void diec(const char *fmt, ...)
{
  va_list ap;
  va_start( ap, fmt );
  vfprintf( stderr, fmt, ap );
  va_end( ap );
  exit( 1 );
}

int parse_line1(const char *str, const int *_bq, int bqnum, int ref,
                struct strarray *_indel, struct strarray *base2)
{
  const char *BASES = "ATGCN*";
  int indel_character = 0;
  int str_length = strlen( str );
  int indel_q_sum = 0;
  int k = -1;
  while ( ++k < str_length ) {
     int b = str[k];
     int ub = toupper( b );
     if ( b == '.' || b == ',' ) {
       sa_push_c( base2, ref );
     } else if ( strchr( BASES, ub ) != NULL) {
       sa_push_c( base2, ub );
     } else if ( b == '^' ) {
       indel_character += 2;
       k++;
       continue;
     } else if ( b == '$' ) {
       indel_character++;
       continue;
     } else if ( b == '+' || b == '-' ) {
       if ( isdigit( str[k+1] ) ) {
         char *endptr = NULL;
         int indel_len_num = strtol( &str[k+1], &endptr, 10 );
         int indel_len_num_len = endptr - &str[k+1];
         int i;
         sa_push_c( _indel, b );
         for (i=0;i<indel_len_num;i++,endptr++) {
           sa_putc( _indel, toupper( *endptr ) );
         }
         if (k - 1 - indel_character < bqnum && k - 1 - indel_character >= 0) {
           indel_q_sum += _bq[k - 1 - indel_character];
         }
         else {
           diec( "_bq over line:%d %s\n", total, str );
         }
         indel_character += indel_len_num_len + indel_len_num + 1;
         k += indel_len_num_len + indel_len_num;
         continue;
       } else {
         diec( "?? line:%d %s\n", total, str );
       }
     } else if ( isdigit(b) ) {
       diec( "Base call !! line:%d %s\n", total, str );
     } else {
       diec( "Base call !! line %d %s %c\n", total, str, b );
     }
  }
  return indel_q_sum;
}

struct snp_bayse {
  enum SNP_INDEL snp_indel;
  char geno[3];
  int cov;
  int call_num[4];
  double P_allele;
  double P_error;
};

void get_max_indexes(int max_indexes[2], const int call_num[4])
{
  int i;
  int imax = 2; // "ATGC" G first
  int imax2;
  for (i=3;i>=0;i--) {
    if (call_num[i] > call_num[imax]) {
      imax = i;
    }
  }
  if (imax == 2) {
    imax2 = 3;
  }
  else {
    imax2 = 2;
  }
  for (i=3;i>=0;i--) {
    if (i != imax && call_num[i] > call_num[imax2]) {
      imax2 = i;
    }
  }
  max_indexes[0] = imax;
  max_indexes[1] = imax2;
}

int snp_bayse_q( struct snp_bayse *p, struct strarray *call_ref,
                 const int *_bq, int bqnum, const int *_mq, int mqnum,
                 int ref, int CUT_OFF_Q, int MQ_CUTOFF, int REQUIRED_BQ,
                 int LOWER_COV, double LOSS)
{
   int fcall_num[4] = { 0, 0, 0, 0 };
   int call_num[4]  = { 0, 0, 0, 0 };
   const char BASES[5] = "ATGC";
   int j = sa_scalar(call_ref);
   {
     int lbq[j];
     int lcall[j];
     int i = -1;
     int c;
     int nlcall = 0;
     int major_allele;
     int allele_2nd;
     int major_allele_idx;
     int allele_2nd_idx;
     int max_indexes[2];
     int n;
     double kaijyou_major;
     double kaijyou_2nd;
     double kaijyou_n;
     double major_error_rate;
     double minor_error_rate;
     double P_error_1;
     char *ptr_idx;
     int base_idx;

     while ( ++i < j ) {
       if ( _bq[i] < CUT_OFF_Q || _mq[i] < MQ_CUTOFF ) continue;
       c = sa_get( call_ref, i )[0];
       ptr_idx = strchr( BASES, c );
       if ( ptr_idx == NULL ) continue;
       base_idx = ptr_idx - BASES;
       call_num[ base_idx ]++;
       lcall[ nlcall ] = c;
       lbq[ nlcall ] = ( _bq[i] == 2 )?10:_bq[i];
       nlcall++;
       if ( _bq[i] >= REQUIRED_BQ ) {
         fcall_num[ base_idx ] = 1;
       }
     }
     if ( nlcall < LOWER_COV ) return -1;
     p->snp_indel = SNP_T;
     p->cov = nlcall;

     get_max_indexes( max_indexes, call_num );
     major_allele_idx = max_indexes[0];
     allele_2nd_idx   = max_indexes[1];
     major_allele = BASES[ major_allele_idx ];
     allele_2nd   = BASES[ allele_2nd_idx ];

     // TODO : BQ > 30 filter
     if ( fcall_num[ major_allele_idx ] ) {
       if ( fcall_num[ allele_2nd_idx ] ) {
          // do nothing;
       } else {
         call_num[ allele_2nd_idx ] = 0;
       }
     } else if ( ! fcall_num[ major_allele_idx ] ) {
       if ( fcall_num[ allele_2nd_idx ] ) {
         int t;
         call_num[ major_allele_idx ] = 0;
         t = major_allele;
         major_allele = allele_2nd;
         allele_2nd = t;
         t = major_allele_idx;
         major_allele_idx = allele_2nd_idx;
         allele_2nd_idx = t;
       } else {
         call_num[ allele_2nd_idx ] = 0;
       }
     }

     // !! NEW !!
     if ( major_allele == ref && call_num[ allele_2nd_idx ] == 0) {
       return -1;
     }

     n = call_num[ major_allele_idx ] + call_num[ allele_2nd_idx ];
     kaijyou_major = kaijyou( call_num[ major_allele_idx ] );
     kaijyou_2nd   = kaijyou( call_num[ allele_2nd_idx ] );
     kaijyou_n     = kaijyou( n );

     major_error_rate = 1.0;
     minor_error_rate = 1.0;
     i = -1;
     while ( ++i < nlcall ) {
       if ( lcall[i] == major_allele ) {
         major_error_rate *= 1.0 - pow( 10.0, (-1) * ( lbq[i] / 10.0) );
       } else if ( lcall[i] == allele_2nd ) {
         minor_error_rate *= pow( 10.0, (-1) * ( lbq[i] / 10.0) );
       }
     }

     P_error_1   = kaijyou_n / ( kaijyou_major *  kaijyou_2nd );
     p->P_allele = P_error_1 * pow( 0.5, n );
     p->P_error  = P_error_1 * ( major_error_rate * minor_error_rate );

     p->geno[2] = '\0';
     if ( call_num[ allele_2nd_idx ] == 0 ) {
       p->geno[0] = major_allele;
       p->geno[1] = major_allele;
     } else if ( p->P_allele / p->P_error > LOSS ) {
       if ( major_allele < allele_2nd ) {
         p->geno[0] = major_allele;
         p->geno[1] = allele_2nd;
       } else {
         p->geno[0] = allele_2nd;
         p->geno[1] = major_allele;
       }
     } else if ( major_allele == ref ) {
       return -1;
     } else {
       p->geno[0] = major_allele;
       p->geno[1] = major_allele;
     }
     for (i=0;i<4;i++) {
       p->call_num[i] = call_num[i];
     }
   }

   return 0;
}

struct snp {
  enum SNP_INDEL snp_indel;
  char *chr;
  char *pos;
  char *ref;
  struct snp_bayse *bayse_q;
  struct indel_frq *indel_q;
};

void sa_push_snp(struct strarray *p, const struct snp *snp)
{
  int add = 0;
  char *ptr;
  if (snp->snp_indel == SNP_T) {
    add += sizeof(struct snp_bayse)+1;
  } else {
    add += sizeof(struct indel_frq)+1;
    add += strlen(snp->indel_q->geno)+1;
  }
  add += strlen(snp->chr)+1;
  add += strlen(snp->pos)+1;
  add += strlen(snp->ref)+1;
  add = ((add+16-1)/16)*16;
  sa_extend(p, add);
  ptr = p->data+p->datatop;
  if (snp->snp_indel == SNP_T) {
    memcpy(ptr, snp->bayse_q, sizeof(struct snp_bayse));
    ptr += sizeof(struct snp_bayse)+1;
    ptr[-1] = '\0';
  } else {
    memcpy(ptr, snp->indel_q, sizeof(struct indel_frq));
    ptr += sizeof(struct indel_frq)+1;
    ptr[-1] = '\0';
    strcpy(ptr, snp->indel_q->geno);
    ptr += strlen(snp->indel_q->geno)+1;
  }
  strcpy(ptr, snp->chr);
  ptr += strlen(snp->chr)+1;
  strcpy(ptr, snp->pos);
  ptr += strlen(snp->pos)+1;
  strcpy(ptr, snp->ref);
  ptr += strlen(snp->ref)+1;
  p->datatop += add;
  p->length++;
}

void sa_get_snp(struct strarray *p, int idx, struct snp *snp)
{
  int i,k;
  char *pstr[3];
  char *q;
  int knum;
  if (p == NULL || idx < 0 || idx >= p->length) {
    snp = NULL;
    return;
  }
  if (idx < p->current_idx) {
    i = 0;
    p->current_ptr = p->data;
  } else {
    i = p->current_idx;
  }
  for (;i<idx;i++) {
    q = p->current_ptr;
    if (*((enum SNP_INDEL *)q) == SNP_T) {
      q += sizeof(struct snp_bayse)+1;
      knum = 3;
    } else {
      q += sizeof(struct indel_frq)+1;
      knum = 4;
    }
    for (k=0;k<knum;k++) {
      for (;q<p->data+p->datasize;q++) {
        if (*q == '\0') break;
      }
      q++;
    }
    p->current_ptr += ((q-p->current_ptr+15)/16)*16;
  }
  if (*((enum SNP_INDEL *)p->current_ptr) == SNP_T) {
    snp->bayse_q = (struct snp_bayse *)p->current_ptr;
    snp->indel_q = NULL;
    q = p->current_ptr+sizeof(struct snp_bayse)+1;
    snp->snp_indel = SNP_T;
  } else {
    snp->indel_q = (struct indel_frq *)p->current_ptr;
    snp->bayse_q = NULL;
    q = p->current_ptr+sizeof(struct indel_frq)+1;
    snp->indel_q->geno = q;
    q += strlen(snp->indel_q->geno)+1;
    snp->snp_indel = INDEL_T;
  }
  for (k=0;k<3;k++) {
    pstr[k] = q;
    for (;q<p->data+p->datasize;q++) {
      if (*q == '\0') break;
    }
    q++;
  }
  snp->chr = pstr[0];
  snp->pos    = pstr[1];
  snp->ref    = pstr[2];
  p->current_idx = idx;
}

void sa_get_TypeSNP(struct strarray *p, int idx, TypeSNP *typeSNP)
{
  struct snp geno;
  sa_get_snp(p, idx, &geno);
  typeSNP->gicode   = geno.chr;
  typeSNP->position = strtol(geno.pos, NULL, 10);
  typeSNP->kchar    = geno.ref[0];
  switch (geno.snp_indel) {
  case SNP_T:
    typeSNP->sstring = geno.bayse_q->geno;
    typeSNP->key1    = geno.bayse_q->cov;
    typeSNP->key2    = geno.bayse_q->call_num[0];
    typeSNP->key3    = geno.bayse_q->call_num[1];
    typeSNP->key4    = geno.bayse_q->call_num[2];
    typeSNP->key5    = geno.bayse_q->call_num[3];
    typeSNP->d1      = geno.bayse_q->P_allele;
    typeSNP->d2      = geno.bayse_q->P_error;
    break;
  case INDEL_T:
    typeSNP->sstring = geno.indel_q->geno;
    typeSNP->key1    = geno.indel_q->cov;
    typeSNP->key2    = geno.indel_q->ref_allele_num;
    typeSNP->key3    = geno.indel_q->indel_allele_num;
    typeSNP->key4    = 0;
    typeSNP->key5    = 0;
    typeSNP->d1      = geno.indel_q->ref_allele_frq;
    typeSNP->d2      = geno.indel_q->indel_allele_frq;
    break;
  }
}

void chomp(char *buf)
{
  int n = strlen(buf);
  int i;
  for (i=n-1;i>=0;i--) {
    if ( buf[i] == '\n' || buf[i] == '\r') {
      buf[i] = '\0';
    } else {
      break;
    }
  }
}

void readtoEOL(char **pbuf, int *pbufsize, FILE *fp)
{
  char *buf = *pbuf;
  int bufsize = *pbufsize;
  int n=strlen(buf);
  while ( n == bufsize-1 && buf[n-1] != '\n' ) {
    bufsize *= 2;
    buf = realloc(buf, bufsize);
    if (fgets(&buf[n], bufsize-n, fp) == NULL) break;
    n = strlen(buf);
  }
  *pbuf = buf;
  *pbufsize = bufsize;
}

void split(char *buf, int c, char *ptrs[], int nsplit)
{
  char *p;
  int isplit = 0;
  ptrs[0] = buf;
  for (p=buf;*p!='\0';p++) {
    if ( *p == c ) {
      *p = '\0';
      isplit++;
      if (isplit >= nsplit) break;
      ptrs[isplit] = p + 1;
    }
  }
  if (isplit < nsplit-1) {
    int i;
    for (i=isplit+1;i<nsplit;i++) {
      ptrs[i] = p;
    }
  }
}

void join(char *buf, int length, int c)
{
  int i;
  for (i=0;i<length;i++) {
    if (buf[i] == '\0') {
      buf[i] = c;
    }
  }
}

void call_indelc(const char *infile, struct strarray *SNP,
                 struct strarray *neighbor_indel,
                 struct strarray *neighbor_low_cq,
                 int CUT_OFF_Q, int MQ_CUTOFF, int REQUIRED_BQ,
                 int LOWER_COV, int UPPER_COV, double LOSS,
                 double INDEL_Q_SCORE,
                 double INDEL_ALLELE_FRQ_CUTOFF, int INDEL_LOWER_LIMIT,
                 double CONSENSUS_Q, double SNP_Q)
{
  char *LINE[11];
  char *buf = malloc(PILE_BUFSIZE);
  int bufsize = PILE_BUFSIZE;
  FILE *IN_PILE;
  int i;
  int ref;
  struct strarray indel;
  struct strarray base2;
  double indel_q = 0.0;
  int indel_q_sum;
  int cov;
  int line4;
  struct snp_bayse geno_snp;
  struct indel_frq geno_indel;
  struct snp geno;
  char *genotype;
  char refref[3];

  IN_PILE = fopen( infile, "r" );
  if ( IN_PILE == NULL ) {
    diec( "%s !! at %s line %d.\n", infile, __FILE__, __LINE__ );
  }

  sa_init( &indel, SA_INITSIZE );
  sa_init( &base2, SA_INITSIZE );

  while ( fgets( buf, bufsize, IN_PILE ) != NULL ) {
    int linelength;
    readtoEOL( &buf, &bufsize, IN_PILE );
    if ( buf[0] == '#' ) continue;
    chomp( buf );
    linelength = strlen( buf );
    total++;

    split( buf, '\t', LINE, 11);
    ref = LINE[2][0];
    if ( chr[0] == '\0' ) {
      strncpy( chr, LINE[0], sizeof(chr) );
    }
    if ( ref != '*' ) {
      indel_q = 0;
    }
    cov = strtol(LINE[7], NULL, 10);
    if ( cov < LOWER_COV || cov > UPPER_COV ) continue;

    line4 = strtol(LINE[4], NULL, 10);
    if ( line4 < 20 ) {
      sa_push( neighbor_low_cq, LINE[1] );
    }

    if ( ref != '*' ) {
      int bqnum = strlen(LINE[9]);
      int mqnum = strlen(LINE[10]);
      int max_mq;
      int BQ[bqnum];
      int MQ[mqnum];
      int ret;
      for (i=0;i<bqnum;i++) {
        BQ[i] = LINE[9][i] - 33;
      }
      sa_clear( &base2 );
      indel_q_sum = parse_line1(LINE[8], BQ, bqnum, ref, &indel, &base2);
      if (indel_q_sum) {
        indel_q = (double)indel_q_sum / sa_scalar(&indel);
      }
      else {
        indel_q = 0;
      }

      if ( bqnum != sa_scalar(&base2) ) {
        join( buf, linelength, '\t');
        diec( "%s\t diffrent number of #BQ, #base, #MQ :\t%d\t%d\t%d\n",
              buf, bqnum-1, sa_scalar(&base2), mqnum );
      }
      if ( line4 < 20 ) continue;
      max_mq = 0;
      for (i=0;i<bqnum;i++) {
        MQ[i] = LINE[10][i] - 33;
        if (MQ[i] > max_mq) max_mq = MQ[i];
      }

      if ( max_mq < 40 ) continue;
      ret = snp_bayse_q( &geno_snp, &base2,
                         BQ, bqnum, MQ, mqnum,
                         ref, CUT_OFF_Q, MQ_CUTOFF, REQUIRED_BQ,
                         LOWER_COV, LOSS);
      if (ret < 0) continue;

      geno.snp_indel = SNP_T;
      geno.bayse_q = &geno_snp;
      geno.indel_q = NULL;
      genotype = geno_snp.geno;
    } else {
      int ref_allele_num;
      int indel_allele_num;
      int indel_allele_idx;
      // $ref eq '*'
      sa_push( neighbor_indel, LINE[1] );

      // count indel allele 120119 on previous line
      if ( sa_scalar( &indel ) == 0 ) {
        diec( "indel allele not found !! %s %s\n", LINE[0], LINE[1] );
      }
      // cut low q-score indel 1201199
      if ( indel_q <= INDEL_Q_SCORE ) {
        indel_q = 0;
        sa_clear( &indel );
        continue;
      }

      ref_allele_num = strtol( LINE[7], NULL, 10 ) - sa_scalar( &indel );
      indel_allele_num = sa_maxfrq( &indel, &indel_allele_idx );
      sa_clear( &indel );

      if (indel_frq_Q(&geno_indel, LINE[3], ref_allele_num, indel_allele_num,
            INDEL_ALLELE_FRQ_CUTOFF, INDEL_LOWER_LIMIT) < 0) continue;
      if (geno_indel.indel_allele_num <= 3 && line4 < CONSENSUS_Q &&
            strtol( LINE[5], NULL, 10 ) < SNP_Q) continue;

      geno.snp_indel = INDEL_T;
      geno.bayse_q = NULL;
      geno.indel_q = &geno_indel;
      genotype = geno_indel.geno;
    }

    refref[0] = ref;
    refref[1] = ref;
    refref[2] = '\0';
    if ( strcmp( refref, genotype ) != 0 && strcmp( genotype, "--" ) != 0 ) {
      char ref_s[2] = { ref, '\0' };
      geno.chr = LINE[0];
      geno.pos = LINE[1];
      geno.ref = ref_s;
      sa_push_snp( SNP, &geno );
    }
  }

  sa_free(&indel);
  sa_free(&base2);
  free(buf);
}

char *strtolower(char *dest, const char *src)
{
  const char *p;
  char *q;
  for (p=src,q=dest;*p!='\0';p++,q++) {
    *q = tolower(*p);
  }
  *q = '\0';
  return dest;
}

int getoptions(int argc, char *argv[])
{
  int i,j;
  int nopt = sizeof(GetOptions)/sizeof(GetOptions[0]);
  for (i=1;i<argc;i++) {
    if ( argv[i][0] == '-' ) {
      int match[nopt];
      int nmatch = 0;
      int lastmatch = -1;
      int ptype[nopt];
      char lstr[strlen(argv[i])];
      strtolower( lstr, &argv[i][1] );
      for (j=0;j<nopt;j++) {
        char *p;
        char sopt[strlen(GetOptions[j].option)+1];
        strtolower( sopt, GetOptions[j].option );
        p = strchr( sopt, '=' );
        if ( p != NULL ) {
          *p = '\0';
          ptype[j] = p[1];
        }
        if ( strcmp( lstr, sopt ) == 0 ) {
          nmatch = 1;
          lastmatch = j;
          break;
        }
        if ( strncmp( lstr, sopt, strlen(lstr) ) == 0 ) {
          match[j] = 1;
          lastmatch = j;
          nmatch++;
        } else {
          match[j] = 0;
        }
      }
      if ( nmatch == 0 ) {
        fprintf( stderr, "Unknown option: %s\n", lstr );
        return -1;
      } else if ( nmatch > 1 ) {
        fprintf( stderr, "Option %s is ambiguos (", lstr );
        for (j=0;j<nopt;j++) {
          if ( match[j] ) {
            char *p;
            char lopt[strlen(GetOptions[j].option)+1];
            strtolower( lopt, GetOptions[j].option );
            p = strchr( lopt, '=' );
            if ( p != NULL ) {
              *p = '\0';
            }
            if ( j == lastmatch ) {
              fprintf( stderr, "%s)\n", lopt );
            } else {
              fprintf( stderr, "%s, ", lopt );
            }
          }
        }
        return -1;
      } else if (i+1 >= argc) {
        char *p;
        char lopt[strlen(GetOptions[lastmatch].option)+1];
        strtolower( lopt, GetOptions[lastmatch].option );
        p = strchr( lopt, '=' );
        if ( p != NULL ) {
          *p = '\0';
        }
        fprintf( stderr, "Option %s is requires an argument\n", lopt );
        return -1;
      } else {
        switch ( ptype[lastmatch] ) {
        case 'i':
          *((int *)GetOptions[lastmatch].ptr) = strtol( argv[i+1], NULL, 10 );
          break;
        case 'f':
          *((double *)GetOptions[lastmatch].ptr) = strtod( argv[i+1], NULL );
          break;
        case 's':
          *((char **)GetOptions[lastmatch].ptr) = argv[i+1];
          break;
        }
      }
    }
  }
  return 0;
}

extern int  indel_SNP_callc(int numSNP, TypeSNP *tSNP,
                    int numneighbor_indel, int *neighbor_indel,
                    int numneighbor_low_cq, int *neighbor_low_cq,
                    int *t_SNPs, int *t_ins, int *t_del);
int main(int argc, char *argv[])
{
  struct strarray SNP;
  struct strarray neighbor_indel;
  struct strarray neighbor_low_cq;

  getoptions( argc, argv );

  if ( INFILE == NULL ) {
    printf( "-INF Input file!!!\n" );
    return -1;
  }
  if ( INDELFile == NULL ) {
    printf( "-INDEL Indel file!!!\n" );
    return -1;
  }
  if ( SNPFile == NULL ) {
    printf( "-SNP SNP file!!!\n" );
    return -1;
  }
  if ( SUMFile == NULL ) {
    printf( "-SUM Summary file!!!\n" );
    return -1;
  }

  SUM_fp = fopen( SUMFile, "w" );
  if ( SUM_fp == NULL ) {
    diec( "Cannot open %s!!\n", SUMFile );
  }

  sa_init( &SNP, SA_INITSIZE2 );
  sa_init( &neighbor_indel, SA_INITSIZE2 );
  sa_init( &neighbor_low_cq, SA_INITSIZE2 );
  call_indelc( INFILE, &SNP, &neighbor_indel, &neighbor_low_cq,
               CUT_OFF_Q, MQ_CUTOFF, REQUIRED_BQ,
               LOWER_COV, UPPER_COV, LOSS, INDEL_Q_SCORE,
               INDEL_ALLELE_FRQ_CUTOFF, INDEL_LOWER_LIMIT,
               CONSENSUS_Q, SNP_Q);

  SNP_fp = fopen( SNPFile, "w" );
  if ( SNP_fp == NULL ) {
    diec( "Died at %s line %d.\n", __FILE__, __LINE__ );
  }
  INDEL_fp = fopen( INDELFile, "w" );
  if ( INDEL_fp == NULL ) {
    diec( "Died at %s line %d.\n", __FILE__, __LINE__ );
  }

  fprintf( SNP_fp,"#chr\tpos\tref\tgenotype\tdepth\tA\tT\tG\tC\tA\tT\tG\tC\n" );
  fprintf( INDEL_fp,"#chr\tpos\tref\tgenotype\tdepth\tnon-ins\tins\n" );

  ////// indel_SNP_call etc
  {
    int c;
    int numSNP;
    TypeSNP *tSNP;
    int *tneighbor_indel;
    int numneighbor_indel;
    int *tneighbor_low_cq;
    int numneighbor_low_cq;
    int total_SNPs = 0;
    int total_ins = 0;
    int total_del = 0;
    int i;

    numSNP = sa_scalar(&SNP);
    tSNP = (TypeSNP *)malloc( numSNP * sizeof(TypeSNP) );
    for (i=0;i<numSNP;i++) {
      sa_get_TypeSNP( &SNP, i, &tSNP[i] );
    }

    numneighbor_indel = sa_scalar( &neighbor_indel );
    tneighbor_indel = (int *)malloc( numneighbor_indel * sizeof(int) );
    for (i=0;i<numneighbor_indel;i++) {
      tneighbor_indel[i] = strtol( sa_get( &neighbor_indel, i ), NULL, 10 );
    }

    numneighbor_low_cq = sa_scalar( &neighbor_low_cq );
    tneighbor_low_cq = (int *)malloc( numneighbor_low_cq * sizeof(int) );
    for (i=0;i<numneighbor_low_cq;i++) {
      tneighbor_low_cq[i] = strtol( sa_get( &neighbor_low_cq, i ), NULL, 10 );
    }

    c = indel_SNP_callc(numSNP, tSNP,
                    numneighbor_indel, tneighbor_indel,
                    numneighbor_low_cq, tneighbor_low_cq,
                    &total_SNPs, &total_ins, &total_del);

    // Summary Output
    fprintf(SUM_fp, "\n"                          );
    fprintf(SUM_fp, "# Files\n"                   );
    fprintf(SUM_fp, "input_file\t%s\n", INFILE    );
    fprintf(SUM_fp, "INDEL\t%s\n",      INDELFile );
    fprintf(SUM_fp, "SNP\t%s\n",        SNPFile   );
    fprintf(SUM_fp, "SUM\t%s\n",        SUMFile   );
    fprintf(SUM_fp, "\n"                          );
    fprintf(SUM_fp, "# Threshold\n"               );
    fprintf(SUM_fp, "lower_cov\t%d\n",  LOWER_COV );
    fprintf(SUM_fp, "cut_off_frq\t%g\n",CUT_OFF_FRQ);
    fprintf(SUM_fp, "cut_off_Q\t%d\n",  CUT_OFF_Q );
    fprintf(SUM_fp, "cut_off_num\t%d\n",CUT_OFF_NUM);
    fprintf(SUM_fp, "cut_off_Q2\t%d\n",CUT_OFF_Q2);
    fprintf(SUM_fp, "cut_off_num2\t%d\n",CUT_OFF_NUM2);
    fprintf(SUM_fp, "indel_range\t%d\n",INDEL_RANGE);
    fprintf(SUM_fp, "SNP_range\t%d\n",  SNP_RANGE );
    fprintf(SUM_fp, "ins_lower_limit\t%d\n", INDEL_LOWER_LIMIT);
    fprintf(SUM_fp, "neighbor_SNP_range\t%d\n", NEIGHBOR_SNP_RANGE);
    fprintf(SUM_fp, "neighbor_SNP_num\t%d\n", NEIGHBOR_SNP_NUM);
    fprintf(SUM_fp, "mq_cutoff\t%d\n",  MQ_CUTOFF );
    fprintf(SUM_fp, "inndel_allele_frq_cutoff\t%g\n", INDEL_ALLELE_FRQ_CUTOFF);
    fprintf(SUM_fp, "minimum_base_quality\t%d\n", REQUIRED_BQ);
    fprintf(SUM_fp, "\n"                          );
    fprintf(SUM_fp, "#Analysis summary\n"         );
    fprintf(SUM_fp, "chr\t%s\n", chr              );
    fprintf(SUM_fp, "analyzed\t%d\n", total       );
    fprintf(SUM_fp, "SNPs\t%d\n", total_SNPs      );
    fprintf(SUM_fp, "Insertion\t%d\n", total_ins  );
    fprintf(SUM_fp, "Deletion\t%d\n", total_del   );

    free(tSNP);
    free(tneighbor_indel);
    free(tneighbor_low_cq);
  }

  sa_free( &SNP );
  sa_free( &neighbor_indel );
  sa_free( &neighbor_low_cq );
  fclose(SNP_fp);
  fclose(INDEL_fp);
  fclose(SUM_fp);
  return 0;
}
