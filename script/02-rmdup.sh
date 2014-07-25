#! /bin/sh
#
# It removes duplicates from the alignment result.
# It actually does the following operations.
#   1. convert SAM file to BAM file.
#   2. sort the BAM file.
#   3. remove duplication from the sorted BAM file.
# It assumes that 'samtools' program is located in the same directory.

# if 1, dry run
DEBUG=0

BIN_DIR=`dirname $0`
SAMT_BIN=./$BIN_DIR/samtools
OUT_DIR=./`basename $0`_OUT
MED_DIR=./`basename $0`_MED
PROF_DIR=./`basename $0`_PROF
N_MEMORY=800000000
K_PROFILE=0


while getopts m:p opt
do
  case $opt in
  m)
    N_MEMORY=$OPTARG;;
  p)
    K_PROFILE=1;;
  \?)
    exit 1;;
  esac
done
shift `expr $OPTIND - 1`

REF_FILE=$1
SAM_FILE=$2

if [ -z $REF_FILE ] || [ -z $SAM_FILE ]; then
  echo "Usage: `basename $0` REFERENCE_INDEX_FILE SAM_FILE"
  echo "  Options"
  echo "    -m MEMORY   : specify the amount of memory"
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

BAM_FILE=$MED_DIR/`basename $SAM_FILE .sam`.bam
CMD="$SAMT_BIN import $REF_FILE $SAM_FILE $BAM_FILE"
run_command $CMD

S_BAM_FILE=$MED_DIR/`basename $BAM_FILE .bam`.sort
CMD="$SAMT_BIN sort -m $N_MEMORY $BAM_FILE $S_BAM_FILE"
run_command $CMD

OUT_FILE=$OUT_DIR/`basename $SAM_FILE .sam`.bam
CMD="$SAMT_BIN rmdup $S_BAM_FILE.bam $OUT_FILE"
run_command $CMD

