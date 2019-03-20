NGS Analyzer-MINI Input Data
============================

* version: 1.1
* update: 2019/03/20 (Update the data download Web address)
* contact: miniapp@riken.jp


本ドキュメントについて
----------------------

本ドキュメントはNGS Analyerミニアプリで使用する入力データの作成方法につい
て述べる。


データの種類
--------------

NGS Analyzerミニアプリでは次のデータを入力とし、すべてNational Center for
Biotechnology Information (NCBI)から入手できる。

* リファレンスゲノム
* 解析対象ゲノム（日本人男性ゲノム）

また、動作確認用にダミーの解析ゲノムを用意している。


データの作成についての注意
----------------------------

作業はミニアプリを実行するマシンと同じアーキテクチャ・OSのマシンで行うこと。
以下では、富士通FX10での作業を想定する。ステージングは利用しない。このとき、
結果として生成されたデータは京コンピュータでも利用可能であるが、Intelアーキ
テクチャマシンでは利用できない。


リファレンスゲノム
--------------------

1. FTPサイトからダウンロード（2.9GB）  
   ダウンロード用スクリプト download_reference.sh を用いてダウンロードする。
   リファレンスゲノムは ~/ngsa_mini_input/ ディレクトリに reference.fa と
   してダウンロードするとする。このとき、~/ngsa_mini_input/work ディレクトリ
   に作業ファイルが保存される。
   このスクリプトではwgetコマンドがインストールされていること想定している。

        $ cd ~/ngsa_mini_input
        $ PATH_TO_NGSA_MINI/bin/download_reference.sh ./work

   実行完了後、カレントディレクトリに reference.fa が生成される。

   このスクリプトでは、以下のFTPサーバからファイル群をダウンロードし、解凍
   後、順番に結合している。wgetコマンドがインストールされていない、リンクが
   存在しない等でダウンロードできない場合には、以下を参考に個別にダウンロー
   ドすること。
    * サイト: ftp.ncbi.nlm.nih.gov
    * ユーザ: anonymous
    * ディレクトリ: genomes/H_sapiens/ARCHIVE/ANNOTATION_RELEASE.104
    * ファイル（計860MB）
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

2. SAMtoolsでインデックス作成（22KB）

        $ cd ~/ngsa_mini_input
        $ pjsub --interact
        $ PATH_TO_NGSA_MINI/bin/samtools faidx reference.fa
        $ exit

   実行完了後、カレントディレクトリに reference.fa.fai が生成される。

3. bwa用ファイルの作成（4.2GB）  
   FX10 1ノードで処理に2時間ほど時間がかかる。

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
            ジョブ実行完了後
        $ cd ~/ngsa_mini_input

4. 生成されたファイルの確認  
   以上により、~/ngsa_mini_inputディレクトリには次のファイルが作成されている
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


### コンティグの染色体へのマッピング情報 ###

ダウンロード用スクリプト download_contig.sh を用いてダウンロードする。この
ファイルは ~/ngsa_mini_input/ ディレクトリに seq_contig.md としてダウンロー
ドするとする。このスクリプトではwgetコマンドがインストールされていること想
定している。

    $ cd ~/ngsa_mini_input
    $ PATH_TO_NGSA_MINI/bin/download_contig.sh

実行完了後、カレントディレクトリに seq_contig.md （634KB）が生成される。

このスクリプトでは、以下のFTPサーバからダウンロードしたファイルを解凍してい
る。wgetコマンドがインストールされていない、リンクが存在しない等でダウンロ
ードできない場合には、以下を参考にダウンロードすること。

* サイト: ftp.ncbi.nlm.nih.gov
* ユーザ: anonymous
* ディレクトリ: genomes/H_sapiens/ARCHIVE/ANNOTATION_RELEASE.104/mapview
* ファイル: seq_contig.md.gz


解析対象ゲノム
----------------

解析対象ゲノムには日本人男性ゲノムデータと、試験のための疑似データの2種類
を用意している。いずれかのデータを以下の手順に従い用意すること。実際の評価
には「日本人男性ゲノム」を利用すること。


### 疑似データ ###

疑似データ一式をダウンロードする。

    $ cd ~/ngsa_mini_input
    $ wget http://mt.r-ccs.riken.jp/hpci-miniapp/ngsa-data/ngsa-dummy.tar.gz
    $ tar zxf ngsa-dummy.tar.gz
    $ rm ngsa-dummy.tar.gz

この疑似データはリファレンスゲノムを入力として、ArtificialFastqGenerator
を用いて生成している。

[ArtificialFastqGenerator](http://sourceforge.net/projects/artfastqgen/)

疑似データは12ペアのゲノムファイルからなる。12プロセス以下のプロセス数で
実行すること。それ以上のサイズのプロセス数で実行する場合、下記に従い、
日本人男性ゲノムデータを用いること。


### 日本人男性ゲノム ###

1. SRA Toolkitのインストール
   1. ダウンロード
      * http://www.ncbi.nlm.nih.gov/Traces/sra/sra.cgi?view=software
      * ここでは最新のCentOS Linux 64bit向けのバイナリをダウンロードする
        - http://ftp-trace.ncbi.nlm.nih.gov/sra/sdk/2.3.5-2/sratoolkit.2.3.5-2-centos_linux64.tar.gz

   2. インストール  
      ~/ngsa_mini_input/workディレクトリにダウンロードしたとする

            $ cd ~/ngsa_mini_input/work
            $ tar zxf sratoolkit.2.3.5-2-centos_linux64.tar.gz
            $ cd sratoolkit.2.3.5-2-centos_linux64
            $ ./bin/configuration-assistant.perl

2. ゲノムファイルのダウンロード  
   ダウンロード用スクリプト download_target.sh を用いてダウンロードする。
   ゲノムファイルは ~/ngsa_mini_input/work/DRR000617.sra （14GB）としてダ
   ウンロードされ、フォーマット変換されたものが
   ~/ngsa_mini_input/work/DRR000617.fastq ディレクトリに保存される。このフ
   ァイルは、総容量 121.9GB （SRAフォーマット）の日本人男性ヒトゲノムデー
   タの一部である。
   このスクリプトではwgetコマンドがインストールされていること想定している。

        $ cd ~/ngsa_mini_input
        $ PATH_TO_NGSA_MINI/bin/download_target.sh ./work

   ダウンロード時間とマシン性能によるが、実行完了までに3時間ほど要する。
   実行完了後、~/ngsa_mini_input/work/DRR000617.fastq ディレクトリに次の
   ファイルが生成される。
   * DRR000617.sra_1.fastq（36GB）
   * DRR000617.sra_2.fastq（36GB）

   このスクリプトでは、以下のFTPサーバからファイルをダウンロードし、フォー
   マット変換している。wgetコマンドがインストールされていない、リンクが存在
   しない等でダウンロードできない場合には、以下を参考に個別にダウンロードす
   ること。
    * サイト: ftp-trace.ncbi.nlm.nih.gov
      * 参考URL: http://www.ncbi.nlm.nih.gov/Traces/sra/sra.cgi?study=DRP000223
    * ユーザ: anonymous
    * ディレクトリ:  
    sra/sra-instant/reads/ByStudy/sra/DRP/DRP000/DRP000223/DRR000617/
    * ファイル: DRR000617.sra

    個別にダウンロードした場合、ダウンロード完了後、フォーマット変換は次の
    通りに実行できる。

        $ cd ~/ngsa_mini_input/work
        $ ./sratoolkit.2.3.5-2-centos_linux64/bin/fastq-dump --split-3 \
        -A DRR000617.sra -O DRR000617.fastq

3. データの削減 (Optional)  
   このファイルをそのまま入力とすると長時間処理に時間がかかるため（京コン
   ピュータ546ノードで2時間弱）、ミニアプリを実行するマシンの性能に応じて
   データ量の削減を行う。  

   NGS Analyzer-MINIでは1,000,000行(250,000配列)単位で分割されたゲノムファ
   イル群を入力とする。この分割数が並列度となる。以下の例では 12 並列で
   実行するために12,000,000行のファイルを作成している。並列度を変更する場
   合は、1,000,000の倍数を保ちながら、行数を変更すれば良い。

        $ cd ~/ngsa_mini_input/work
        $ head -12000000 DRR000617.fastq/DRR000617.sra_1.fastq \
        > sra_1.12000000
        $ head -12000000 DRR000617.fastq/DRR000617.sra_2.fastq \
        > sra_2.12000000

4. ファイルの分割  
   250,000配列単位でそれぞれのファイルを分割する。

        $ cd ~/ngsa_mini_input/work
        $ PATH_TO_NGSA_MINI/bin/read_split.py -f -n 250000 -p 'part_1.' \
        -d ../00-read sra_1.12000000
        $ PATH_TO_NGSA_MINI/bin/read_split.py -f -n 250000 -p 'part_2.' \
        -d ../00-read sra_2.12000000

   ~/ngsa_mini_input ディレクトリに次のファイルが生成される
   * 00-read/
     * part_1.X (00 <= X <= 11)
     * part_2.X (00 <= X <= 11)

5. ワークフローの実行  
   NGS Analyzerのワークフローを実行し、中間ファイル群を生成する。入力は
   ~/ngsa_mini_input/00-read ディレクトリ以下の12ペアのファイルとし、こ
   れらを6ノードで処理するとする。

   1. 入力ファイルの配置  
      ランク名のディレクトリを作成し、その中に ~/ngsa_mini_input/00-read
      ディレクトリ以下のファイルペアをコピーする。

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

   2. 実行  
      PATH_TO_NGSA_MINI/bin/workflow を実行する。

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
                ジョブ実行完了まで待つ。

      実行完了後、~/ngsa_mini_input/work 以下に次のディレクトリが生成さ
      れる。
         * 中間ファイル（32GB）: workflow_MED
         * 出力ファイル（604MB）: workflow_OUT

      以上の手順1, 2を京でランクディレクトリを用いて実行するときのサンプ
      ルジョブスクリプトを k_job_script/workflow-job.sh として用意した。
      このスクリプトでは12ノード使用する。

            $ pjsub ./k_job_script/workflow-job.sh

6. ファイルをコピー
   1. マッピング処理用ファイルのコピー  
      ~/ngsa_mini_input/00-read 以下の任意の1ファイルペアをコピーする。

            $ cd ~/ngsa_mini_input
            $ mkdir 01-alignment
            $ cp 00-read/part_1.00 01-alignment/sra_1.fastq.0
            $ cp 00-read/part_2.00 01-alignment/sra_2.fastq.0

   2. 重複除去処理用ファイルのコピー  
      中間ファイルディレクトリから、拡張子が「.sam」のファイルをコピー
      する。ファイル毎にサイズが大きく異なるので、サイズの最小のもの、最
      大のもの、平均的なサイズのものの3種類をコピーする。ここでは
      NW_003315921.1.sam 、 NT_007933.15.sam 、 NT_008818.16.sam とする。
      以下のコマンド実行例で「*」はランク番号に相当する。

            $ cd ~/ngsa_mini_input
            $ mkdir 02-rmdup
            $ cp work/workflow_MED/*/NW_003315921.1.sam 02-rmdup
            $ cp work/workflow_MED/*/NT_007933.15.sam 02-rmdup
            $ cp work/workflow_MED/*/NT_008818.16.sam 02-rmdup

   3. 変異同定処理用ファイルのコピー  
      上でコピーしたファイルと同じプレフィックスを持つ、拡張子
      「.sort.rmdup.bam」ファイルをコピーする。

            $ cd ~/ngsa_mini_input
            $ mkdir 03-snp
            $ cp work/workflow_MED/*/NW_003315921.1.sort.rmdup.bam \
            03-snp/NW_003315921.1.bam
            $ cp work/workflow_MED/*/NT_007933.15.sort.rmdup.bam \
            03-snp/NT_007933.15.bam
            $ cp work/workflow_MED/*/NT_008818.16.sort.rmdup.bam \
            03-snp/NT_008818.16.bam


ファイルの確認
--------------

以上の手順を実行すると、 ngsa_mini_input ディレクトリは次のレイアウトになる。

    ngsa_mini_input/
        00-read/
            part_1.X (00 <= X <= 11)
            part_2.X (00 <= X <= 11)
        01-alignment/                 (疑似データではなし)
            sra_1.fastq.0
            sra_2.fastq.0
        02-rmdup/                     (疑似データではなし)
            NW_003315921.1.sam
            NT_007933.15.sam
            NT_008818.16.sam
        03-snp/                       (疑似データではなし)
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

これらのファイルから、workディレクトリを除くすべてをミニアプリをインストー
ルしたディレクトリ以下の input ディレクトリに移動する。
