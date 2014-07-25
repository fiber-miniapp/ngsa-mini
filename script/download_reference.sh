#! /bin/sh

CMD_WGET=`which wget`

CURDIR=`pwd`
OUT_FILE=$CURDIR/reference.fa

SERVER="ftp.ncbi.nlm.nih.gov"
DIR_PREFIX="genomes/H_sapiens/ARCHIVE/ANNOTATION_RELEASE.104"
FILES=( "CHR_01/hs_ref_GRCh37.p10_chr1.fa.gz"
        "CHR_02/hs_ref_GRCh37.p10_chr2.fa.gz"
        "CHR_03/hs_ref_GRCh37.p10_chr3.fa.gz"
        "CHR_04/hs_ref_GRCh37.p10_chr4.fa.gz"
        "CHR_05/hs_ref_GRCh37.p10_chr5.fa.gz"
        "CHR_06/hs_ref_GRCh37.p10_chr6.fa.gz"
        "CHR_07/hs_ref_GRCh37.p10_chr7.fa.gz"
        "CHR_08/hs_ref_GRCh37.p10_chr8.fa.gz"
        "CHR_09/hs_ref_GRCh37.p10_chr9.fa.gz"
        "CHR_10/hs_ref_GRCh37.p10_chr10.fa.gz"
        "CHR_11/hs_ref_GRCh37.p10_chr11.fa.gz"
        "CHR_12/hs_ref_GRCh37.p10_chr12.fa.gz"
        "CHR_13/hs_ref_GRCh37.p10_chr13.fa.gz"
        "CHR_14/hs_ref_GRCh37.p10_chr14.fa.gz"
        "CHR_15/hs_ref_GRCh37.p10_chr15.fa.gz"
        "CHR_16/hs_ref_GRCh37.p10_chr16.fa.gz"
        "CHR_17/hs_ref_GRCh37.p10_chr17.fa.gz"
        "CHR_18/hs_ref_GRCh37.p10_chr18.fa.gz"
        "CHR_19/hs_ref_GRCh37.p10_chr19.fa.gz"
        "CHR_20/hs_ref_GRCh37.p10_chr20.fa.gz"
        "CHR_21/hs_ref_GRCh37.p10_chr21.fa.gz"
        "CHR_22/hs_ref_GRCh37.p10_chr22.fa.gz"
        "CHR_X/hs_ref_GRCh37.p10_chrX.fa.gz"
        "CHR_Y/hs_ref_GRCh37.p10_chrY.fa.gz"
        "CHR_MT/hs_ref_GRCh37.p10_chrMT.fa.gz" )


if [ $# -ne 1 ]; then
    echo "Specify a directory to store files" 1>&2
    exit 1
fi
OUTDIR=$1

# download files
for file in "${FILES[@]}"; do
    $CMD_WGET -P $OUTDIR ftp://$SERVER/$DIR_PREFIX/$file
    sleep 1
done

# concatenate files
cd $OUTDIR
gunzip *.gz
for file in "${FILES[@]}"; do
    _file=`basename $file .gz`
    echo "process $_file..."
    cat $_file >> $OUT_FILE
done
cd $CURDIR
