#! /bin/sh

CMD_WGET=`which wget`

CURDIR=`pwd`
SERVER="ftp.ncbi.nlm.nih.gov"
DIR_PREFIX="genomes/H_sapiens/ARCHIVE/ANNOTATION_RELEASE.104/mapview"
FILE="seq_contig.md.gz"

$CMD_WGET -P $CURDIR ftp://$SERVER/$DIR_PREFIX/$FILE
gunzip $FILE
