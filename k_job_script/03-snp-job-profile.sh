#! /bin/sh
#
#PJM --rsc-list "node=1"
#PJM --rsc-list "rscgrp=small"
#PJM --rsc-list "elapse=01:00:00"
#PJM --stg-transfiles all
#PJM --stgin "./bin/03-snp.sh ./"
#PJM --stgin "./bin/samtools ./"
#PJM --stgin "./bin/snp ./"
#PJM --stgin "./input/reference.fa ./"
#PJM --stgin "./input/03-snp/NT_008818.16.bam ./"
#PJM --stgout "./03-snp.sh_MED/* ./03-snp-job.sh_MED/"
#PJM --stgout "./03-snp.sh_OUT/* ./03-snp-job.sh_OUT/"
#PJM --stgout "./03-snp.sh_PROF/fapp_1_samtools/* ./03-snp-job.sh_PROF/fapp_1_samtools/"
#PJM --stgout "./03-snp.sh_PROF/fapp_2_snp/* ./03-snp-job.sh_PROF/fapp_2_snp/"
#PJM -s

. /work/system/Env_base

./03-snp.sh -p reference.fa NT_008818.16.bam
