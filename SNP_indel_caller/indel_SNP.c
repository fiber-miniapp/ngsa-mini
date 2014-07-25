/*
	File:	indel_SNP.c
	Copyright(C) 2012-2013 RIKEN, Japan.
*/
#include "snp.h"

// SNP, indel
typedef struct _TepySNP {
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

// SNP list
typedef struct _TepySNP_list {
    int     ival;        /* use mutation */
    struct _TepySNP       *snp;
    struct _TepySNP_list *next;
} TypeSNP_list;

// low index + hight index
typedef struct _Range {
    int low;
    int hight;
} Range;

// PostionとそのCounter
// Rangeと同型
typedef struct _PosCount {
    int position; /* genom posistion */
    int counter;  /* counter         */
} PosCount;

#define MALLOC(x)  (x *)malloc(sizeof(x))
#define ARRAY_SIZE 1024

//Parameter parameter={5, 5, 3, 10, NULL};
static int total_del = 0;
static int total_ins = 0;
static int total_SNPs = 0;

#define getValue(x, idx) ((idx == 0) ? x.low : x.hight)

void myfree(void *ptr)
{
    if(ptr != NULL){
	free(ptr);
    }
}

void myfreeList(TypeSNP_list *tlist)
{
    int i;
    TypeSNP_list *tmp;
    TypeSNP_list *save;
    if(tlist != NULL){
	for(i = 0, tmp = tlist; tmp != NULL;i++){
	    save = tmp->next;
	    myfree(tmp);
	    tmp = save;
	}
    }
}

/* 配列の要素を交換する Range */
void SwapRange(Range x[ ], int i, int j)
{
    Range temp;
    temp = x[i];
    x[i] = x[j];
    x[j] = temp;
}

/* 配列の要素を交換する int */
void SwapInt(int x[ ], int i, int j)
{
    int temp;
    temp = x[i];
    x[i] = x[j];
    x[j] = temp;
}

void mergeRange(Range a[], int na, Range b[], int nb, Range ab[], int idx)
{
  int ia=0,ib=0,i,n=na+nb;
  for (i=0;i<n;i++) {
    if ( ib >= nb ||
         ( ia < na && getValue( a[ia], idx ) <= getValue( b[ib], idx ) ) ) {
      ab[i] = a[ia];
      ia++;
    } else {
      ab[i] = b[ib];
      ib++;
    }
  }
}

void mergeSortRange(Range a[], int na, int idx)
{
  if ( na > 2 ) {
    int nb = na/2;
    int nc = na-nb;
    Range b[nb];
    Range c[nc];
    memcpy( b, a, nb*sizeof(Range) );
    memcpy( c, a+nb, nc*sizeof(Range) );
    mergeSortRange( b, nb, idx );
    mergeSortRange( c, nc, idx );
    mergeRange( b, nb, c, nc, a, idx );
  } else if ( na == 2 ) {
    if ( getValue( a[0], idx ) > getValue( a[1], idx ) ) {
      SwapRange( a, 0, 1 );
    }
  }
}

void mergeInt(int a[], int na, int b[], int nb, int ab[])
{
  int ia=0,ib=0,i,n=na+nb;
  for (i=0;i<n;i++) {
    if ( ib >= nb ||
         ( ia < na && a[ia] <= b[ib] ) ) {
      ab[i] = a[ia];
      ia++;
    } else {
      ab[i] = b[ib];
      ib++;
    }
  }
}

void mergeSortInt(int a[], int na)
{
  if ( na > 2 ) {
    int nb = na/2;
    int nc = na-nb;
    int b[nb];
    int c[nc];
    memcpy( b, a, nb*sizeof(int) );
    memcpy( c, a+nb, nc*sizeof(int) );
    mergeSortInt( b, nb );
    mergeSortInt( c, nc );
    mergeInt( b, nb, c, nc, a );
  } else if ( na == 2 ) {
    if ( a[0] > a[1] ) {
      SwapInt( a, 0, 1 );
    }
  }
}

/* ソートを行う int */
void SortInt(int x[ ], int xsize)
{
  mergeSortInt( x, xsize );
}

/* ソートを行う Range */
void SortRange(Range x[ ], int xsize, int idx)
{
  mergeSortRange( x, xsize, idx );
}

/* リストデータをsortコピーする */
TypeSNP_list *copyListSort(TypeSNP_list *mptr, TypeSNP_list *src)
{
    TypeSNP_list *tmp, *stmp, *prev;
    TypeSNP_list *cur;

    if(src == NULL){
	return mptr;
    }
    for(stmp = src; stmp != NULL; stmp = stmp->next){
	tmp = MALLOC(TypeSNP_list);
	tmp->ival = stmp->ival;
        tmp->snp  = stmp->snp;
        tmp->next = NULL;

	if(mptr != NULL){
	    prev = NULL;
	    for(cur = mptr; cur != NULL; cur = cur->next){
	        if(cur->snp->position > tmp->snp->position){
		    tmp->next = cur;
		    if(prev == NULL){
			mptr = tmp;
		    }
		    else {
			prev->next = tmp;
		    }
		    break;
	        }
		else {
		    prev = cur;
		}
	    }
	    if(prev != NULL && prev->next == NULL){
	        prev->next = tmp;
	    }
	}
	else {
	    mptr = tmp;
	}
    }
    return mptr;
}

int search_index(int *filtered_indel_pos, int  num, int pos)
{
    int i;
    int ret = 0;
    for(i=0; i < num; i++){
	if(filtered_indel_pos[i] == pos){
	    ret = 1;
	    break;
	}
    }
    return ret;
}

int search_index_PosCount(PosCount *window, int  num, int pos)
{
    int i;
    int ret = 0;
    for(i=0; i < num; i++){
	if(window[i].position == pos){
	    ret = window[i].counter;
	    break;
	}
    }
    return ret;
}

/* get_largest_indel_size のC言語版 */
int c_get_largest_indel_size( char *str )
{
    int i, cnt, ret;
    int len = strlen(str);

    for(i = 0, cnt = 0, ret = 0; i < len; i++){
	if(str[i] == '/'){
	    if(ret < cnt){
		ret = cnt;
	    }
	    cnt = 0;
	}
        else {
	    cnt++;
        }
    }
    if(ret < cnt){
	ret = cnt;
    }

    return ret - 1;
}

void pushSNP(TypeSNP_list **top, TypeSNP *snp, int ival)
{
    TypeSNP_list *tmp;
    TypeSNP_list *cur;

    tmp = MALLOC(TypeSNP_list);
    tmp->ival = ival;
    tmp->snp  = snp;
    tmp->next = NULL;
    if(*top == NULL){
	*top      = tmp;
    }
    else {
	for(cur = *top; cur->next != NULL; cur = cur->next);

	cur->next = tmp;
    }
}

void pushRange(Range **top, int low, int hight, int *num, int *allocsz)
{
    if(*num >= *allocsz){
	*allocsz += ARRAY_SIZE;
        *top = (Range *)realloc(*top, sizeof(Range) * (*allocsz));
    }
    (*top)[*num].low   = low;
    (*top)[*num].hight = hight;
    (*num)++;
}

void pushInt(int **top, int *cur, int numcur, int *num, int *allocsz)
{
    int i;

    for(i = 0; i < numcur; i++){
	if(*num >= *allocsz){
	    *allocsz += ARRAY_SIZE;
	    *top = (int *)realloc(*top, sizeof(int )*(*allocsz));
	}
	(*top)[*num] = cur[i];
	(*num)++;
    }
}

void push_window(int pos1, PosCount **top, int *num, int *allocsz){
    int i;
    PosCount *idlw = *top;
    for(i=0; i < *num; i++){
	if(idlw[i].position == pos1){
	    idlw[i].counter++;
	    return;
	}
    }
    if(*num >= *allocsz){
	*allocsz += ARRAY_SIZE;
        *top = (PosCount *)realloc(*top, sizeof(PosCount) * (*allocsz));
    }
    (*top)[*num].position = pos1;
    (*top)[*num].counter = 1;
    (*num)++;
    return;
}

int *neighbor_SNPs(int SNP[], int num_SNP, int nrange, int srange, int *asize,
int *allocsz)
{
    int i, j;
    int examined_SNPs = 0;
    int neighbor_SNP_tmp[ARRAY_SIZE];
    int nindex = 0;
    int *all_neighbor_SNP = (int *)calloc(sizeof(int), ARRAY_SIZE);
    *asize    = 0;
    *allocsz  = ARRAY_SIZE;

    for(i=0; i < num_SNP; i++){
        int SNP1 = SNP[i];
        nindex = 0;
	neighbor_SNP_tmp[nindex] = SNP1;
        nindex++;
	for(j = examined_SNPs; j < num_SNP; j++){
	    int SNP2 = SNP[j];
	    if((SNP2 - SNP1) < (nrange * -1)){
		examined_SNPs++;
		continue;
	    }
	    if(abs(SNP2 - SNP1) <= nrange && abs(SNP2 - SNP1) != 0){
		neighbor_SNP_tmp[nindex] = SNP2;
		nindex++;
	    }
	    else if( (SNP2 - SNP1) > nrange ) {
		break;
	    }
	    else {
		;
	    }
        }
	if( nindex >= srange ){
	    pushInt(&all_neighbor_SNP,neighbor_SNP_tmp, nindex, asize, allocsz);
	}
    }
    SortInt(all_neighbor_SNP, *asize);
    return all_neighbor_SNP;
}

/* Mutation Listの最後の保存領域 */
static TypeSNP_list *mlast=NULL;
TypeSNP_list *pushMutation(TypeSNP_list *mutation, TypeSNP *snp)
{
    TypeSNP_list *ret=NULL;
    TypeSNP_list *cur = MALLOC(TypeSNP_list);
    cur->ival = 2;
    cur->snp  = snp;
    cur->next = NULL;
    ret       = mutation;
    if(mlast != NULL){
	mlast->next = cur;
	mlast       = cur;
    }
    else {
	if(mutation == NULL){
	    mlast = cur;
	    ret = cur;
	}
        else {
	    for(mlast = mutation; mlast->next != NULL; mlast = mlast->next);

	    mlast->next = cur;
	    mlast       = cur;
	}
    }
    return ret;
}

void shiftRange(Range *iarray, int shift, int *num)
{
    int i;
    if(shift <= 0){
	return;
    }
    else if(shift >= *num){
	for(i=0; i < *num; i++){
	    iarray[i].low   = 0;
	    iarray[i].hight = 0;
	}
	*num = 0;
	return;
    }

    for(i=shift; i < *num;i++){
	iarray[i - shift] = iarray[i];
    }
    for(i = (*num - shift); i < *num;i++){
	iarray[i].low   = 0;
	iarray[i].hight = 0;
    }
    *num -= shift;
}

void shiftPos(int *iarray, int shift, int *num)
{
    int i;
    if(shift <= 0){
	return;
    }
    else if(shift >= *num){
	for(i=0; i < *num; i++){
	    iarray[i] = 0;
	}
	*num = 0;
	return;
    }

    for(i=shift; i < *num;i++){
	iarray[i - shift] = iarray[i];
    }
    for(i = (*num - shift); i < *num;i++){
	iarray[i] = 0;
    }
    *num -= shift;
}

int getHash(int position, PosCount *iwindow, int num)
{
    int i;
    int ret = 0;
    for(i=0; i < num; i++){
	if(iwindow[i].position == position){
	    ret = iwindow[i].counter;
	    break;
	}
    }
    return ret;
}

void show_realign(TypeSNP_list *mptr, PosCount *iwindow, int num)
{
    int i;
    FILE *REALIGN_ptr = NULL;
    PosCount *iw;
    /* mutationは、0->1->2で生成済み */
    /* sortは、省略                  */
    for(; mptr != NULL; mptr = mptr->next){
	int tmp = mptr->ival;
	if(tmp < 2){
	    if(getHash(mptr->snp->position, iwindow, num) >= 1){
		continue;
	    }
	    if(tmp == 0){
		total_del++;
	    }
	    else if(tmp == 1){
		total_ins++;
	    }
	    fprintf(INDEL_fp, "%s\t%d\t%c\t%s\t%d\t%d\t%d\t%.15g\t%.15g\n",
		mptr->snp->gicode,
		mptr->snp->position,
		mptr->snp->kchar,
		mptr->snp->sstring,
		mptr->snp->key1,
		mptr->snp->key2,
		mptr->snp->key3,
		mptr->snp->d1,
		mptr->snp->d2
            );
	}
	if(tmp == 2){
	    fprintf(SNP_fp, "%s\t%d\t%c\t%s\t%d\t%d\t%d\t%d\t%d\t%.15g\t%.15g\n",
		mptr->snp->gicode,
		mptr->snp->position,
		mptr->snp->kchar,
		mptr->snp->sstring,
		mptr->snp->key1,
		mptr->snp->key2,
		mptr->snp->key3,
		mptr->snp->key4,
		mptr->snp->key5,
		mptr->snp->d1,
		mptr->snp->d2
            );
	    total_SNPs++;
	}
    }
    if((REALIGN_ptr = fopen(REALIGNFile,"w")) == NULL){
	fprintf(stderr,"Cann not open REALIGNFile (%s)\n", REALIGNFile);
	exit(1);
    }
    iw = (PosCount *)malloc(sizeof(PosCount) * num);
    for(i=0; i < num; i++){
	iw[i] = iwindow[i];
    }

    SortRange((Range *)iw, num, 0);
    for(i=0; i < num; i++){
	fprintf(REALIGN_ptr, "%s\t%d\t%d\n", chr, iw[i].position, iw[i].counter);
    }
    fclose(REALIGN_ptr);
    myfree(iw);
}

int indel_SNP_callc( int numSNP, TypeSNP SNP[],
                    int numneighbor_indel, int neighbor_indel[],
                    int numneighbor_low_cq, int neighbor_low_cq[],
                    int *t_SNPs, int *t_ins, int *t_del)
{
    TypeSNP_list *SNP_out = NULL;
    TypeSNP_list *del_out = NULL;
    TypeSNP_list *ins_out = NULL;
    TypeSNP_list *snpl    = NULL;
    TypeSNP_list *mutation = NULL;
    TypeSNP_list *del_and_ins = NULL;
    Range        *indel_region = (Range *)calloc(sizeof(Range), ARRAY_SIZE) ;
    int          num_del_region = 0;
    int          sz_del_region = ARRAY_SIZE;
    int i, j, k;
    int total_SNPs = 0;
    int total_ins  = 0;
    int total_del  = 0;
    int num_SNP_out = 0;
    char *subs;
    TypeSNP *geno;
    int *neighbor_SNP;
    int num_neighbor_SNP;
    int allocsz_neighbor_SNP;
    int irange = INDEL_RANGE;
    int nrange = NEIGHBOR_SNP_RANGE;
    int nnum   = NEIGHBOR_SNP_NUM;
    int *SNP_pos = NULL;
    PosCount *indel_in_window = (PosCount *)calloc(sizeof(PosCount), ARRAY_SIZE);
    int num_indel_in_window = 0;
    int allocsz_indel_in_window = ARRAY_SIZE;

    for(i=0; i < numSNP; i++){
		geno = &SNP[i];
        /* この判定変更が正しいか確認する */
		if(geno->kchar != '*'){
		    /* SNP type */
	    	pushSNP(&SNP_out, geno, 2);
            num_SNP_out++;
        } else if((subs = strstr(geno->sstring, "-")) != NULL){
		    /* 欠損 */
		    pushRange(&indel_region, geno->position + 1 - irange,
	            geno->position + c_get_largest_indel_size( geno->sstring ) + 
	            irange, &num_del_region, &sz_del_region);
		    pushSNP(&del_out, geno, 0);
		} else if((subs = strstr(geno->sstring, "+")) != NULL){
            /* 挿入 */
		    pushRange(&indel_region, geno->position + 1 - irange,
    	        geno->position + irange, &num_del_region, &sz_del_region);
	    	pushSNP(&ins_out, geno, 1);
        } else {
		    continue;
		}
    }

    /* sort by indel_reagion */
    SortRange(indel_region, num_del_region, 1);
    SortRange(indel_region, num_del_region, 0);

    SNP_pos = (int *)calloc(sizeof(int), num_SNP_out);
    for(i=0,snpl = SNP_out; i < num_SNP_out; i++, snpl = snpl->next){
	SNP_pos[i] = snpl->snp->position;
    }

    neighbor_SNP = NULL;
    num_neighbor_SNP = 0;
    allocsz_neighbor_SNP = 0;
    neighbor_SNP = neighbor_SNPs(SNP_pos, num_SNP_out, nrange, nnum,
                   &num_neighbor_SNP, &allocsz_neighbor_SNP);

    mutation = copyListSort(NULL, del_out);
    mutation = copyListSort(mutation, ins_out);
    del_and_ins = copyListSort(NULL, del_out);
    del_and_ins = copyListSort(del_and_ins, ins_out);

    for(i=0, snpl = SNP_out; i < num_SNP_out; i++, snpl = snpl->next){
	int output = 0;
	int shift  = 0;
        for(j=0; j < num_del_region; j++){
	    if(indel_region[j].hight < SNP_pos[i]){
		shift++;
	    }
	    else if((indel_region[j].low <= SNP_pos[i]) &&
                    (SNP_pos[i] <= indel_region[j].hight)){
		output = 1;
	    }
	    else if (SNP_pos[i] < indel_region[j].low) {
		break;
	    }
	    else {
		;
	    }
	}
	if(shift > 0){
	    shiftRange(indel_region, shift, &num_del_region);
	}
	if(output == 1){
	    continue;
	}
	shift = 0;
	
	for(k=0; k < num_neighbor_SNP; k++){
	    if(neighbor_SNP[k] < SNP_pos[i]){
		shift++;
	    }
	    else if(SNP_pos[i] == neighbor_SNP[k]){
		output = 1;
		break;
	    }
	    else {
		break;
	    }
	}
	if(shift > 0){
	    shiftPos(neighbor_SNP, shift, &num_neighbor_SNP);
	}
	if(output == 1){
	    continue;
	}
	/* snpl は、2 */
	mutation = pushMutation(mutation, snpl->snp);
    }

    {
	int nii = -1;

	for(i=0; i < numneighbor_indel; i++){
	    int ii;
	    int pos1 = neighbor_indel[i];
	    for(ii=nii+1; ii < numneighbor_indel; ii++){
		int pos2 = neighbor_indel[ii];
		if((pos1 - pos2) > INDEL_WINDOW){
		    nii++;
		    continue;
		}
		else if(abs(pos1 - pos2 ) <= INDEL_WINDOW &&
 			(pos1 - pos2) != 0 ){
                    push_window(pos1, &indel_in_window, &num_indel_in_window, &allocsz_indel_in_window);
                }
		if ((pos2 - pos1) > INDEL_WINDOW){
		    break;
		}
	    }
	}
    }

    if(REALIGNFile != NULL){
	show_realign(mutation, indel_in_window, num_indel_in_window);
    }
    else {
        TypeSNP_list *tmp1 = NULL;
	double maxd2 = 0.0;
        int *filtered_indel_pos = (int *)calloc(sizeof(int), ARRAY_SIZE);
	int num_filtered_indel_pos = 0;
	int allocsz_filtered_indel_pos = ARRAY_SIZE;
	TypeSNP_list *tlist=NULL;
	int idx = 0;

	for(tmp1 = del_and_ins; tmp1 != NULL; tmp1 = tmp1->next){
	    // neighbor_indel_tmp -> tlist
	    TypeSNP_list *ltmp = NULL;
	    TypeSNP_list *tmp2 = NULL;
	    int list_count = 1;

	    tlist = MALLOC(TypeSNP_list);
	    tlist->ival = 0;
	    tlist->snp  = tmp1->snp;
	    tlist->next = NULL;
	    ltmp  = tlist;

            // 最大値とその位置を求める
	    maxd2 = tmp1->snp->d2;
            idx   = tmp1->snp->position;

	    for(tmp2 = del_and_ins; tmp2 != NULL; tmp2 = tmp2->next){
		if(tmp1->snp->position != tmp2->snp->position &&
                   abs(tmp1->snp->position - tmp2->snp->position) <= 10){
		    TypeSNP_list *cur = MALLOC(TypeSNP_list);
		    cur->ival  = 0;
		    cur->snp   = tmp2->snp;
		    cur->next  = NULL;
		    ltmp->next = cur;
		    ltmp       = cur;
		    list_count++;
		    if(cur->snp->d2 > maxd2){
			maxd2 = cur->snp->d2;
			idx   = cur->snp->position;
		    }
		}
	    }
	    if(list_count == 1){
		myfreeList(tlist);
		tlist = NULL;
		continue;
	    }
	    if(list_count >= 3){
		pushInt(&filtered_indel_pos, &(tmp1->snp->position), 1,
		&num_filtered_indel_pos, &allocsz_filtered_indel_pos);
	    }
	    for(ltmp = tlist; ltmp != NULL; ltmp = ltmp->next){
		//最大値を持つ位置のものは、登録しない
		if(ltmp->snp->position != idx){
		    pushInt(&filtered_indel_pos, &(ltmp->snp->position), 1,
		    &num_filtered_indel_pos, &allocsz_filtered_indel_pos);
		}
	    }
	    if(tlist != NULL){
	        myfreeList(tlist);
	        tlist = NULL;
	    }
	}

	for(tmp1 = mutation; tmp1 != NULL; tmp1 = tmp1->next){
	    if(tmp1->ival < 2){
		if(search_index(filtered_indel_pos, num_filtered_indel_pos, tmp1->snp->position) == 1){
		    continue;
		}
		if(search_index_PosCount(indel_in_window, num_indel_in_window, tmp1->snp->position) >= 3) {
		    continue;
		}
		if(tmp1->ival == 0){
		    total_del++;
		}
		else if(tmp1->ival == 1){
		    total_ins++;
		}
		fprintf(INDEL_fp, "%s\t%d\t%c\t%s\t%d\t%d\t%d\t%.15g\t%.15g\n",
		    tmp1->snp->gicode,
		    tmp1->snp->position,
		    tmp1->snp->kchar,
		    tmp1->snp->sstring,
		    tmp1->snp->key1,
		    tmp1->snp->key2,
		    tmp1->snp->key3,
		    tmp1->snp->d1,
		    tmp1->snp->d2
		);
	    }
	    else if(tmp1->ival == 2){
		fprintf(SNP_fp, "%s\t%d\t%c\t%s\t%d\t%d\t%d\t%d\t%d\t%.15g\t%.15g\n",
		    tmp1->snp->gicode,
		    tmp1->snp->position,
		    tmp1->snp->kchar,
		    tmp1->snp->sstring,
		    tmp1->snp->key1,
		    tmp1->snp->key2,
		    tmp1->snp->key3,
		    tmp1->snp->key4,
		    tmp1->snp->key5,
		    tmp1->snp->d1,
		    tmp1->snp->d2
		);
		total_SNPs++;
	    }
	}
        myfree(filtered_indel_pos);
    }

    /* 戻り値の設定 */
    *t_SNPs = total_SNPs;
    *t_ins  = total_ins;
    *t_del  = total_del;
    myfree(SNP_pos);
    myfreeList(SNP_out);
    myfree(indel_region);
    myfree(indel_in_window);
    myfreeList(del_out);
    myfreeList(ins_out);
    myfreeList(mutation);
    myfree(neighbor_SNP);
    myfreeList(del_and_ins);
    return 0;
}
