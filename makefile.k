#
#	File:	Makefile
#	Copyright(C) 2012-2013 RIKEN, Japan.
#
BWA_SRC			= bwa-0.5.9rc1_kei
SAMTOOLS_SRC		= samtools-0.1.8_kei
SPLITSAM2CONTIG2_SRC	= splitSam2Contig2
SNPCALL_SRC		= SNP_indel_caller
WF_SRC			= workflow
SCRIPT_SRC		= script

CC			= fccpx
CXX			= FCCpx
AR			= ar
CFLAGS			= -Kfast
BWA_OPT			= CC=$(CC) CXX=$(CXX) AR=$(AR) CFLAGS="-Xg -Kfast" DFLAGS="-D_TARGET_KEI -DHAVE_PTHREAD"
SAMROOLS_OPT		= CC=$(CC) AR=$(AR) CFLAGS="-Xg -Kfast" DFLAGS=-D_TARGET_KEI KNETFILE_O=
SPLITSAM2CONTIG2_OPT	= CXX=$(CXX) CFLAGS=$(CFLAGS)
SNPCALL_OPT		= CC=$(CC) CFLAGS="$(CFLAGS)" LDFLAGS="-Kfast"
WF_OPT			= CC=mpifccpx CFLAGS="$(CFLAGS)"


.PHONY: all
all: bwa samtools snp splitSam2Contig2 workflow script

.PHONY: bwa
bwa:
	mkdir -p bin
	( cd $(BWA_SRC) && make clean && make $(BWA_OPT) && cp bwa ../bin/ )

.PHONY: samtools
samtools:
	mkdir -p bin
	( cd $(SAMTOOLS_SRC) && make clean && make $(SAMROOLS_OPT) && cp samtools ../bin/ )

.PHONY: splitSam2Contig2
splitSam2Contig2:
	mkdir -p bin
	( cd $(SPLITSAM2CONTIG2_SRC) && make clean && make $(SPLITSAM2CONTIG2_OPT) && cp splitSam2Contig2 ../bin/ )

.PHONY: snp
snp:
	mkdir -p bin
	( cd $(SNPCALL_SRC) && make clean && make $(SNPCALL_OPT) && cp snp ../bin/ )

.PHONY: workflow
workflow:
	mkdir -p bin
	( cd $(WF_SRC) && make clean && make $(WF_OPT) && cp workflow ../bin/ && cp workflow_*.sh ../bin/)

.PHONY: script
script:
	mkdir -p bin
	cp $(SCRIPT_SRC)/* bin

.PHONY: clean
clean:
	-rm -rf bin/
	( cd $(BWA_SRC) && make clean )
	( cd $(SAMTOOLS_SRC) && make clean )
	( cd $(SPLITSAM2CONTIG2_SRC) && make clean )
	( cd $(SNPCALL_SRC) && make clean )
	( cd $(WF_SRC) && make clean )
