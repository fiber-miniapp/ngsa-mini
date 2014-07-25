#! /bin/sh

CMD_WGET=`which wget`

CURDIR=`pwd`
SAMTOOL_HOME=./sratoolkit.2.3.5-2-centos_linux64

FILE=DRR000617.sra
DOWNLOAD_FROM=ftp://ftp-trace.ncbi.nlm.nih.gov/sra/sra-instant/reads/ByStudy/sra/DRP/DRP000/DRP000223/DRR000617


if [ $# -ne 1 ]; then
    echo "Specify a directory to store files" 1>&2
    exit 1
fi
OUTDIR=$1

# download
$CMD_WGET -P $OUTDIR $DOWNLOAD_FROM/$FILE

# convert file format
echo "converting file format..."
cd $OUTDIR
FASTQ_OUT=`basename $FILE .sra`.fastq
$SAMTOOL_HOME/bin/fastq-dump --split-3 -A $FILE -O $FASTQ_OUT
cd $CURDIR
echo "done"
