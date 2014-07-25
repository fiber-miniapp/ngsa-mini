#! /bin/sh
#
# It does pileup and calculates snp and intel.
# It assumes that the following programs used in this program are
# located in the same directory.
#   - samtools
#   - snp

# if 1, dry run
DEBUG=0

BIN_DIR=`dirname $0`
SAMT_BIN=./$BIN_DIR/samtools
SNP_BIN=./$BIN_DIR/snp
OUT_DIR=./`basename $0`_OUT
MED_DIR=./`basename $0`_MED
PROF_DIR=./`basename $0`_PROF
K_PROFILE=0


while getopts p opt
do
  case $opt in
  p)
    K_PROFILE=1;;
  \?)
    exit 1;;
  esac
done
shift `expr $OPTIND - 1`

REF_FILE=$1
BAM_FILE=$2

if [ -z $REF_FILE ] || [ -z $BAM_FILE ]; then
  echo "Usage: `basename $0` REFERENCE_FILE BAM_FILE"
  echo "  Options"
  echo "    -p             : take profile on K/FX10"
  exit 1
fi

RUN_COUNT=1
function run_command() {
  CMD=$*
  echo $CMD
  if [ $DEBUG == 0 ]; then
    T_START=`date +%s/%N`
    if [ $K_PROFILE == 0 ]; then
      eval $CMD
    else
      SUFFIX=`basename $1`
      eval "fapp -C -I hwm -d ${PROF_DIR}/fapp_${RUN_COUNT}_${SUFFIX} $CMD"
    fi
    T_END=`date +%s/%N`

    SAVED_IFS="$IFS"
    IFS='/'
    T_SA=( $T_START )
    T_EA=( $T_END )
    IFS="$SAVED_IFS"

    SEC=`expr ${T_EA[0]} - ${T_SA[0]}`
    NANO_SEC=`expr ${T_EA[1]} - ${T_SA[1]}`
    NANO_TIME=`expr $SEC \* 1000000000 + $NANO_SEC`
    TIME=`echo "$NANO_TIME / 1000000000" | bc -l`
    echo
    echo "Time: $TIME"
    echo
  fi
  RUN_COUNT=`expr $RUN_COUNT + 1`
}

if [ $DEBUG == 0 ] ; then
  mkdir -p $OUT_DIR
  mkdir -p $MED_DIR
fi

PILEUP_FILE=$MED_DIR/`basename $BAM_FILE .bam`.pile
CMD="$SAMT_BIN pileup -s -cf $REF_FILE $BAM_FILE > $PILEUP_FILE"
run_command $CMD

OUT_FILE1=$OUT_DIR/`basename $BAM_FILE .bam`.indel
OUT_FILE2=$OUT_DIR/`basename $BAM_FILE .bam`.snp
OUT_FILE3=$OUT_DIR/`basename $BAM_FILE .bam`.sum
CMD="$SNP_BIN -INF $PILEUP_FILE -INDEL $OUT_FILE1 -SNP $OUT_FILE2 -SUM $OUT_FILE3"
run_command $CMD

