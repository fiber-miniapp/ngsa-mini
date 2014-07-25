#! /bin/sh
#
#PJM --rsc-list "node=1"
#PJM --rsc-list "rscgrp=small"
#PJM --rsc-list "elapse=01:00:00"
#PJM --stg-transfiles all
#PJM --stgin "./bin/01-alignment.sh ./"
#PJM --stgin "./bin/bwa ./"
#PJM --stgin "./bin/splitSam2Contig2 ./"
#PJM --stgin "./input/bwa_db/* ./bwa_db/"
#PJM --stgin "./input/seq_contig.md ./"
#PJM --stgin "./input/01-alignment/sra_1.fastq.0 ./"
#PJM --stgin "./input/01-alignment/sra_2.fastq.0 ./"
#PJM --stgout "./01-alignment.sh_MED/* ./01-alignment-job.sh_MED/"
#PJM --stgout "./01-alignment.sh_OUT/* ./01-alignment-job.sh_OUT/"
#PJM -s

. /work/system/Env_base

./01-alignment.sh -t 8 bwa_db/reference.fa seq_contig.md sra_1.fastq.0 sra_2.fastq.0
