#!/bin/sh

#PJM -L rscunit=gwmpc
#PJM -L rscgrp=batch
#PJM -g XXX
#PJM -L proc-core=unlimited
#PJM -L proc-data=unlimited
#PJM -L proc-stack=32Gi
#PJM -L node=12
#PJM --mpi proc=12
#PJM -L elapse=0:20:00
#PJM -j
#PJM -S

export OMP_NUM_THREADS=1
export XOS_MMM_L_ARENA_FREE=1

mpirun -np 12 ./bin/workflow ./input/bwa_db/reference.fa ./input/seq_contig.md ./input/reference.fa ./input/reference.fa.fai ./input/00-read-rank/
