NGS Analyzer-MINI Input Data
============================

* version: 1.1
* update: 2019/03/20 (Update the data download Web address)
* contact: miniapp@riken.jp


About this document
---------------------
This document describes how to create input date used in NGS Analyzer-MINI.


Type of data
--------------

NGS Analyzer-MINI uses the following files as input.  These files can be
downloaded from National Center for Biotechnology Information (NCBI).

* Reference genome
* Analysis target genome (Japanese man)

We also provides pseudo-genome data to test the program.


Notice for creating data
--------------------------

The following operation should be performed on machines whose architecture
and OS are same as machines that run NGS Analyzer-MINI.  We assume to use
Fujitsu FX10, but do not use staging in the following.  In this case, the
resultant data can be used on K computer and Fujitsu FX10, but can't be
used on Intel machines.


Reference genome
------------------

1. Download from a FTP server (2.9GB)  
   Use a download script, named download_reference.sh, to download the
   file.  We assume that the reference genome will be downloaded to
   ~/ngsa_mini_input directory as reference.fa.  In this case, intermediate
   files are stored in ~/ngsa_mini_input/work directory.
   This script assumes that 'wget' command is installed to the working
   machine.

        $ cd ~/ngsa_mini_input
        $ PATH_TO_NGSA_MINI/bin/download_reference.sh ./work

   After execution is completed, you can find refenrence.fa in the current
   directory.

   This script download files from the following FTP servers, uncompresses
   them and concatenates them in order.  If you can not download by using
   this script because of some error, such as lack of wget command and
   dead link, manually download them referring to the information.
   * Site: ftp.ncbi.nlm.nih.gov
    * User: anonymous
    * Directory: genomes/H_sapiens/ARCHIVE/ANNOTATION_RELEASE.104
    * File (860MB in total)
      * CHR_01/hs_ref_GRCh37.p10_chr1.fa.gz
      * CHR_02/hs_ref_GRCh37.p10_chr2.fa.gz
      * CHR_03/hs_ref_GRCh37.p10_chr3.fa.gz
      * CHR_04/hs_ref_GRCh37.p10_chr4.fa.gz
      * CHR_05/hs_ref_GRCh37.p10_chr5.fa.gz
      * CHR_06/hs_ref_GRCh37.p10_chr6.fa.gz
      * CHR_07/hs_ref_GRCh37.p10_chr7.fa.gz
      * CHR_08/hs_ref_GRCh37.p10_chr8.fa.gz
      * CHR_09/hs_ref_GRCh37.p10_chr9.fa.gz
      * CHR_10/hs_ref_GRCh37.p10_chr10.fa.gz
      * CHR_11/hs_ref_GRCh37.p10_chr11.fa.gz
      * CHR_12/hs_ref_GRCh37.p10_chr12.fa.gz
      * CHR_13/hs_ref_GRCh37.p10_chr13.fa.gz
      * CHR_14/hs_ref_GRCh37.p10_chr14.fa.gz
      * CHR_15/hs_ref_GRCh37.p10_chr15.fa.gz
      * CHR_16/hs_ref_GRCh37.p10_chr16.fa.gz
      * CHR_17/hs_ref_GRCh37.p10_chr17.fa.gz
      * CHR_18/hs_ref_GRCh37.p10_chr18.fa.gz
      * CHR_19/hs_ref_GRCh37.p10_chr19.fa.gz
      * CHR_20/hs_ref_GRCh37.p10_chr20.fa.gz
      * CHR_21/hs_ref_GRCh37.p10_chr21.fa.gz
      * CHR_22/hs_ref_GRCh37.p10_chr22.fa.gz
      * CHR_X/hs_ref_GRCh37.p10_chrX.fa.gz
      * CHR_Y/hs_ref_GRCh37.p10_chrY.fa.gz
      * CHR_MT/hs_ref_GRCh37.p10_chrMT.fa.gz

2. Create index using SAMtools (22KB)

        $ cd ~/ngsa_mini_input
        $ pjsub --interact
        $ PATH_TO_NGSA_MINI/bin/samtools faidx reference.fa
        $ exit

   After execution is completed, you can find refenrence.fa.fai in the
   current directory.

3. Create files for bwa (4.2GB)  
   It takes about 2 hours for processing on 1 node in FX10.

        $ cd ~/ngsa_mini_input
        $ mkdir ./bwa_db
        $ cd ./work
	        $ vi job1.sh
           #!/bin/bash -x
           #
           #PJM --rsc-list "node=1"
           #PJM --rsc-list "elapse=04:00:00"
           #PJM --rsc-list "proc-core=unlimited"
           #PJM -s
           . /work/system/Env_base
           PATH_TO_NGSA_MINI/bin/bwa index -a bwtsw -p ../bwa_db/reference.fa \
           ../reference.fa
        $ pjsub job1.sh
            After job is completed.
        $ cd ~/ngsa_mini_input

4. Check generated files  
   By proceeding the above procedure, following files will be generated in
   ~/ngsa_mini_input/ directory.
   * reference.fa
   * reference.fa.fai
   * bwa_db/
     * reference.fa.amb
     * reference.fa.ann
     * reference.fa.bwt
     * reference.fa.pac
     * reference.fa.rbwt
     * reference.fa.rpac
     * reference.fa.rsa
     * reference.fa.sa
   * work/


### Information of mapping between contigs and chromosomes ###

Use a download script, named download_contig.sh, to download the file.  We
assume that the mapping file will be downloaded to ~/ngsa_mini_input
directory as seq_contig.md.
This script assumes that 'wget' command is installed to the working machine.

    $ cd ~/ngsa_mini_input
    $ PATH_TO_NGSA_MINI/bin/download_contig.sh

After execution is completed, seq_contig.md (634KB) will be generated in
the current directory.

This script downloads a file from the following FTP server and uncompresses
it.  If you can not download by using this script because of some error,
such as lack of wget command and dead link, manually download it referring
to the information.

* Site: ftp.ncbi.nlm.nih.gov
* User: anonymous
* Directory: genomes/H_sapiens/ARCHIVE/ANNOTATION_RELEASE.104/mapview
* File: seq_contig.md.gz


Analysis target genome
------------------------

### Pseudo-Genome data ###

Download the pseudo-genome data.

    $ cd ~/ngsa_mini_input
    $ wget http://mt.r-ccs.riken.jp/hpci-miniapp/ngsa-data/ngsa-dummy.tar.gz
    $ tar zxf ngsa-dummy.tar.gz
    $ rm ngsa-dummy.tar.gz

This pseudo-genome data are generated by ArtificialFastqGenerator using
the above reference genome as input. 

[ArtificialFastqGenerator](http://sourceforge.net/projects/artfastqgen/)

The pseudo-genome data consists of 12 file pairs.  When you use this data,
run the program with 12 processes or less.  If you want to run the program
with more processes for scalability test, use the following Japanese Human
Genome data.


### Japanese Human Genome ###

1. Install SRA Toolkit
   1. Download
      * http://www.ncbi.nlm.nih.gov/Traces/sra/sra.cgi?view=software
      * Here we download the latest binary for CentOS Linux 64bit
        - http://ftp-trace.ncbi.nlm.nih.gov/sra/sdk/2.3.5-2/sratoolkit.2.3.5-2-centos_linux64.tar.gz

   2. Install  
      Assuming the file is downloaded to ~/ngsa_mini_input/work/ directory

            $ cd ~/ngsa_mini_input/work
            $ tar zxf sratoolkit.2.3.5-2-centos_linux64.tar.gz
            $ cd sratoolkit.2.3.5-2-centos_linux64
            $ ./bin/configuration-assistant.perl

2. Download the gene file  
   Use a download script, named download_target.sh, to download the file.
   This script will download a gene file, named DRR000617.sra (14GB),
   convert its format and store the converted file in DRR000617.fastq
   directory.  We assume that all files are stored in ~/ngsa_mini_input/work
   directory.
   The gene file is a part of Japanese man genome whose total size is
   121.9GB in SRA format.
   This script assumes that 'wget' command is installed to the working
   machine.

        $ cd ~/ngsa_mini_input
        $ PATH_TO_NGSA_MINI/bin/download_target.sh ./work

   Depending on network and machine performance, it takes about 3 hours
   by finishing execution.
   After execution is completed, you can find the following files in
   ~/ngsa_mini_input/work/DRR000617.fastq directory.
   * DRR000617.sra_1.fastq (36GB)
   * DRR000617.sra_2.fastq (36GB)

   This script downloads a file from the following FTP server and convert
   its format.  If you can not download by using this script because of
   some error, such as lack of wget command and dead link, manually
   download it referring to the information.
   * Site: ftp-trace.ncbi.nlm.nih.gov
      * Reference:  
      http://www.ncbi.nlm.nih.gov/Traces/sra/sra.cgi?study=DRP000223
    * User: anonymous
    * Directory:  
    sra/sra-instant/reads/ByStudy/sra/DRP/DRP000/DRP000223/DRR000617/
    * File: DRR000617.sra

   If you download manually, you can convert format by following the next
   procedure.

        $ cd ~/ngsa_mini_input/work
        $ ./sratoolkit.2.3.5-2-centos_linux64/bin/fastq-dump --split-3 \
        -A DRR000617.sra -O DRR000617.fastq

3. Reduce data (Optional)  
   As it takes very long time for processing all the above data (2 hours
   on 546 node of K computer), we will reduce the size of data depending
   on power of machines we have.

   NGS Analyzer-Mini receives genome files each of which containts
   1,000,000 lines (250,000 arrays) as input.  The number of files
   (actually, number of file pairs) is the maximum number of processes
   used to run the program.  In the following example, we create files
   that have 12,000,000 lines to process them by 12 processes.
   If you want to change the number of processes, change the number of
   lines preserving its count is a multiple of 1,000,000.

        $ cd ~/ngsa_mini_input/work
        $ head -12000000 DRR000617.fastq/DRR000617.sra_1.fastq \
        > sra_1.12000000
        $ head -12000000 DRR000617.fastq/DRR000617.sra_2.fastq \
        > sra_2.12000000

4. Split files  
   Split each file so that every split files contains 250,000 arrays.

        $ cd ~/ngsa_mini_input/work
        $ PATH_TO_NGSA_MINI/bin/read_split.py -f -n 250000 -p 'part_1.' \
        -d ../00-read sra_1.12000000
        $ PATH_TO_NGSA_MINI/bin/read_split.py -f -n 250000 -p 'part_2.' \
        -d ../00-read sra_2.12000000

   There will be files in ~/ngsa_mini_input/ directory.
   * 00-read/
     * part_1.X (00 <= X <= 11)
     * part_2.X (00 <= X <= 11)

5. Execute workflow  
   Generate intermediate files of NGS Analyzer workflow by executing it.
   We use 12 genome file pais in ~/ngsa_mini_input/00-read/ directory as
   input and process them by 6 nodes.

   1. Place input files  
      Create directories whose names are MPI rank numbers, and then copy
      file pairs under ./input/00-read directory.

            $ cd ~/ngsa_mini_input/work
            $ mkdir -p wfinput/0
            $ mkdir -p wfinput/1
            $ mkdir -p wfinput/2
            $ mkdir -p wfinput/3
            $ mkdir -p wfinput/4
            $ mkdir -p wfinput/5
            $ cp ../00-read/part_1.00 wfinput/0
            $ cp ../00-read/part_2.00 wfinput/0
            $ cp ../00-read/part_1.01 wfinput/0
            $ cp ../00-read/part_2.01 wfinput/0
            $ cp ../00-read/part_1.02 wfinput/1
            $ cp ../00-read/part_2.02 wfinput/1
            $ cp ../00-read/part_1.03 wfinput/1
            $ cp ../00-read/part_2.03 wfinput/1
            $ cp ../00-read/part_1.04 wfinput/2
            $ cp ../00-read/part_2.04 wfinput/2
            $ cp ../00-read/part_1.05 wfinput/2
            $ cp ../00-read/part_2.05 wfinput/2
            $ cp ../00-read/part_1.06 wfinput/3
            $ cp ../00-read/part_2.06 wfinput/3
            $ cp ../00-read/part_1.07 wfinput/3
            $ cp ../00-read/part_2.07 wfinput/3
            $ cp ../00-read/part_1.08 wfinput/4
            $ cp ../00-read/part_2.08 wfinput/4
            $ cp ../00-read/part_1.09 wfinput/4
            $ cp ../00-read/part_2.09 wfinput/4
            $ cp ../00-read/part_1.10 wfinput/5
            $ cp ../00-read/part_2.10 wfinput/5
            $ cp ../00-read/part_1.11 wfinput/5
            $ cp ../00-read/part_2.11 wfinput/5

   2. Execute the workflow  
      Run PATH_TO_NGSA_MINI/bin/workflow.

            $ cd ~/ngsa_mini_input/work
            $ vi job2.sh
               #!/bin/bash -x
               #
               #PJM --rsc-list "node=6"
               #PJM --rsc-list "elapse=01:00:00"
               #PJM --rsc-list "proc-core=unlimited"
               #PJM -s
               . /work/system/Env_base
               mpiexec -n 6 PATH_TO_NGSA_MINI/bin/workflow \
               ../bwa_db/reference.fa ../seq_contig.md ../reference.fa \
               ../reference.fa.fai ./wfinput
            $ pjsub job2.sh
                Wait untill job finishes.

      After job is completed, There will be the following files in
      ~/ngsa_mini_input/work/ directory.
      * Intermediate files (32GB) : workflow_MED
      * Output files (604MB) : workflow_OUT

      A sample jobscript for K computer that performs the above procedure
      1 and 2 using rank-directory is provided as
      k_job_script/workflow-job.sh.  This jobscript uses 12 nodes.

           $ pjsub ./k_job_script/workflow-job.sh

6. Copy files  
   1. Copy files for Mapping step  
      Copy any 1 pair of files in ~/ngsa_mini_input/00-read.

            $ cd ~/ngsa_mini_input
            $ mkdir 01-alignment
            $ cp 00-read/part_1.00 01-alignment/sra_1.fastq.0
            $ cp 00-read/part_2.00 01-alignment/sra_2.fastq.0

   2. Copy files for Remove duplicates step  
      Copy files whose suffixes are '.sam' from the intermediate file
      directory.  As the size of file is differ from each other, Copy
      three types of file size: minimum size, maximum size and average
      size.  Here we assume NW_003315921.1.sam, NT_007933.15.sam and
      NT_008818.16.sam each.  "*" corresponds to rank number.

            $ cd ~/ngsa_mini_input
            $ mkdir 02-rmdup
            $ cp work/workflow_MED/*/NW_003315921.1.sam 02-rmdup
            $ cp work/workflow_MED/*/NT_007933.15.sam 02-rmdup
            $ cp work/workflow_MED/*/NT_008818.16.sam 02-rmdup

   3. Copy files for Detect mutations step  
      Copy files whose prefixes are same as files copied in the above
      step and whose suffixes are '.sort.rmdup.bam'.

            $ cd ~/ngsa_mini_input
            $ mkdir 03-snp
            $ cp work/workflow_MED/*/NW_003315921.1.sort.rmdup.bam \
            03-snp/NW_003315921.1.bam
            $ cp work/workflow_MED/*/NT_007933.15.sort.rmdup.bam \
            03-snp/NT_007933.15.bam
            $ cp work/workflow_MED/*/NT_008818.16.sort.rmdup.bam \
            03-snp/NT_008818.16.bam


Check files
---------------

Finally, the layout of ngsa_mini_input/ directory is the following.

    ngsa_mini_input/
        00-read/
            part_1.X (00 <= X <= 11)
            part_2.X (00 <= X <= 11)
        01-alignment/                (Do not exist when pseudo-data is used)
            sra_1.fastq.0
            sra_2.fastq.0
        02-rmdup/                    (Do not exist when pseudo-data is used)
            NW_003315921.1.sam
            NT_007933.15.sam
            NT_008818.16.sam
        03-snp/                      (Do not exist when pseudo-data is used)
            NW_003315921.1.bam
            NT_007933.15.bam
            NT_008818.16.bam
        bwa_db/
            reference.fa.amb
            reference.fa.ann
            reference.fa.bwt
            reference.fa.pac
            reference.fa.rbwt
            reference.fa.rpac
            reference.fa.rsa
            reference.fa.sa
        reference.fa
        reference.fa.fai
        seq_contig.md
        work/

Move all files and directories except work/ directory to input/ directory
located under a directory where NGS Analyzer-MINI is installed.
