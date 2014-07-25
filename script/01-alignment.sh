#! /bin/sh
#
# It performs DNA alignment using bwa.
# It assumes that the following programs used in this program are
# located in the same directory.
#   - bwa
#   - splitSam2Contig2

# if 1, dry run
DEBUG=0

BIN_DIR=`dirname $0`
BWA_BIN=./$BIN_DIR/bwa
SSC_BIN=./$BIN_DIR/splitSam2Contig2
OUT_DIR=./`basename $0`_OUT
MED_DIR=./`basename $0`_MED
PROF_DIR=./`basename $0`_PROF
N_THREADS=8
K_PROFILE=0


while getopts t:p opt
do
  case $opt in
  t)
    N_THREADS=$OPTARG;;
  p)
    K_PROFILE=1;;
  \?)
    exit 1;;
  esac
done
shift `expr $OPTIND - 1`

REF_FILE=$1
CONTIG_FILE=$2
SEQ1_FILE=$3
SEQ2_FILE=$4

if [ -z $REF_FILE ] || [ -z $CONTIG_FILE ] || \
   [ -z $SEQ1_FILE ] || [ -z $SEQ2_FILE ] ; then
  echo "Usage: `basename $0` REFERENCE_FILE CONTIG_FILE SEQUENCE1 SEQUENCE2"
  echo "  Options"
  echo "    -t N_THREADS   : specify the number of threads"
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


if [ $DEBUG == 0 ]; then
  mkdir -p $OUT_DIR
  mkdir -p $MED_DIR
fi

SEQ1_SAI=$MED_DIR/`basename $SEQ1_FILE`.sai
CMD="$BWA_BIN aln -t $N_THREADS $REF_FILE $SEQ1_FILE > $SEQ1_SAI"
run_command $CMD

SEQ2_SAI=$MED_DIR/`basename $SEQ2_FILE`.sai
CMD="$BWA_BIN aln -t $N_THREADS $REF_FILE $SEQ2_FILE > $SEQ2_SAI"
run_command $CMD

SAM_FILE=$MED_DIR/0.sam
CMD="$BWA_BIN sampe $REF_FILE $SEQ1_SAI $SEQ2_SAI $SEQ1_FILE $SEQ2_FILE > $SAM_FILE"
run_command $CMD

INFO_FILE=$OUT_DIR/info.txt
CMD="$SSC_BIN $CONTIG_FILE $SAM_FILE $OUT_DIR > $INFO_FILE"
run_command $CMD

