#! /bin/sh
#
#PJM --rsc-list "node=1"
#PJM --rsc-list "rscgrp=small"
#PJM --rsc-list "elapse=01:00:00"
#PJM --stg-transfiles all
#PJM --stgin "./bin/02-rmdup.sh ./"
#PJM --stgin "./bin/samtools ./"
#PJM --stgin "./input/reference.fa.fai ./"
#PJM --stgin "./input/02-rmdup/NT_008818.16.sam ./"
#PJM --stgout "./02-rmdup.sh_MED/* ./02-rmdup-job.sh_MED/"
#PJM --stgout "./02-rmdup.sh_OUT/* ./02-rmdup-job.sh_OUT/"
#PJM -s

. /work/system/Env_base

./02-rmdup.sh reference.fa.fai NT_008818.16.sam
