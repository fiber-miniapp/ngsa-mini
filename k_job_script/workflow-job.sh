#!/bin/bash -x
#
#PJM --rsc-list "node=12"
#PJM --rsc-list "rscgrp=small"
#PJM --rsc-list "elapse=01:00:00"
#PJM --stg-transfiles "all"
#PJM --mpi "use-rankdir"

#PJM --stgin "rank=* ./bin/workflow %r:./"
#PJM --stgin "rank=* ./bin/workflow_01.sh %r:./"
#PJM --stgin "rank=* ./bin/workflow_02.sh %r:./"
#PJM --stgin "rank=* ./bin/workflow_03.sh %r:./"
#PJM --stgin "rank=* ./bin/workflow_04.sh %r:./"
#PJM --stgin "rank=* ./bin/workflow_05.sh %r:./"
#PJM --stgin "rank=* ./bin/bwa %r:./"
#PJM --stgin "rank=* ./bin/splitSam2Contig2 %r:./"
#PJM --stgin "rank=* ./bin/samtools %r:./"
#PJM --stgin "rank=* ./bin/snp %r:./"
#PJM --stgin "rank=* ./input/bwa_db/* %r:./bwa_db/"
#PJM --stgin "rank=* ./input/seq_contig.md %r:./"
#PJM --stgin "rank=* ./input/reference.fa %r:./"
#PJM --stgin "rank=* ./input/reference.fa.fai %r:./"

#PJM --stgin "rank=0 ./input/00-read/part_1.00 %r:./input/0/"
#PJM --stgin "rank=0 ./input/00-read/part_2.00 %r:./input/0/"
#PJM --stgin "rank=1 ./input/00-read/part_1.01 %r:./input/1/"
#PJM --stgin "rank=1 ./input/00-read/part_2.01 %r:./input/1/"
#PJM --stgin "rank=2 ./input/00-read/part_1.02 %r:./input/2/"
#PJM --stgin "rank=2 ./input/00-read/part_2.02 %r:./input/2/"
#PJM --stgin "rank=3 ./input/00-read/part_1.03 %r:./input/3/"
#PJM --stgin "rank=3 ./input/00-read/part_2.03 %r:./input/3/"
#PJM --stgin "rank=4 ./input/00-read/part_1.04 %r:./input/4/"
#PJM --stgin "rank=4 ./input/00-read/part_2.04 %r:./input/4/"
#PJM --stgin "rank=5 ./input/00-read/part_1.05 %r:./input/5/"
#PJM --stgin "rank=5 ./input/00-read/part_2.05 %r:./input/5/"
#PJM --stgin "rank=6 ./input/00-read/part_1.06 %r:./input/6/"
#PJM --stgin "rank=6 ./input/00-read/part_2.06 %r:./input/6/"
#PJM --stgin "rank=7 ./input/00-read/part_1.07 %r:./input/7/"
#PJM --stgin "rank=7 ./input/00-read/part_2.07 %r:./input/7/"
#PJM --stgin "rank=8 ./input/00-read/part_1.08 %r:./input/8/"
#PJM --stgin "rank=8 ./input/00-read/part_2.08 %r:./input/8/"
#PJM --stgin "rank=9 ./input/00-read/part_1.09 %r:./input/9/"
#PJM --stgin "rank=9 ./input/00-read/part_2.09 %r:./input/9/"
#PJM --stgin "rank=10 ./input/00-read/part_1.10 %r:./input/10/"
#PJM --stgin "rank=10 ./input/00-read/part_2.10 %r:./input/10/"
#PJM --stgin "rank=11 ./input/00-read/part_1.11 %r:./input/11/"
#PJM --stgin "rank=11 ./input/00-read/part_2.11 %r:./input/11/"

#PJM --stgout "rank=* %r:./workflow_MED/%r/* ./workflow_MED/%r/"
#PJM --stgout "rank=* %r:./workflow_OUT/* ./workflow_OUT/"
#;;PJM --stgout "rank=* %r:./time.txt ./workflow_TIME/%r.txt"
#PJM -S

. /work/system/Env_base

mpiexec ./workflow ./bwa_db/reference.fa ./seq_contig.md ./reference.fa ./reference.fa.fai ./input

