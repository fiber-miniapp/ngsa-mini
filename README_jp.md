NGS Analyzer-MINI
=================

* version: 1.1 (based on NGS Analyzer 1.0)
* update: 2019/03/20 (Update the data download Web address)
* contact: miniapp@riken.jp


NGS Analyzerについて
----------------------

本ミニアプリは、理化学研究所ゲノム医科学研究センターで開発されているゲノム
解析プログラムNGS Analyzerをベースにしている。NGS Analyzerは次世代シークエ
ンサーの出力データを高速に解析し、ヒト個人間の遺伝的差異やがんゲノムの突然
変異を高い正確さで同定するプログラムである。

[オリジナルNGS Analyzer](http://www.islim.org/application_d_e.html#D2)


インストール
--------------

### プログラムのコンパイル ###

本プログラムのコンパイル・実行には、OpenMPをサポートしたCおよびC++コンパイ
ラ、MPIライブラリ、Gnu Makeが必要である。京/FX10に対応したMakefileが用意さ
れている。

1. 本パッケージの入手とファイルの展開

2. make
   * 京/FX10

            $ make -f makefile.k

   * X86_64マシン with GCC

            $ make -f makefile.x86_64_gcc

正しくmakeが完了すると、binディレクトリ下に実行プログラム群が作成される。


### データの用意 ###

NGS Analyer-MINIを実行するには入力データが必要なので、README_DATA_jp.md
を参照して作成すること。


プログラム計算内容
--------------------

本ミニアプリは、オリジナルNGS Analyerのゲノム解析のワークフローをシェルス
クリプトで実装したものである。ワークフロー全体を一括して実行するプログラム
と、ワークフローを3つのステップに分割し個別実行するプログラム群の2種類のプ
ログラムからなる。

ワークフローは次のステップより構成される。

1. マッピング  
   入力ゲノムデータを参照ゲノム配列にマッピングする。
   マッピング後、コンティグ（塩基配列のパターン）毎にマッピング結果を分割
   する。

2. マージ  
   マッピング結果をコンティグ毎に結合する。
   入力ゲノムを複数に分割して処理する場合のみ行われるステップである。

3. 重複除去  
   マッピング結果に含まれている、重複する塩基配列パターンを除去する。

4. 変異同定  
   尤度推定に基づき遺伝的多様性の検出を行う。

個々のステップは、一般的に使われているマッピングツールやゲノムフォーマット
ツール等を等を用いて実装されている。


プログラム実行方法
--------------------

### ワークフローの一括実行 ###

マッピング、マージ、重複除去、変異同定の4ステップをまとめて実行するプログ
ラムとして workflow を用意している。workflow はMPIにて並列実行する。


#### 実行方法 ####

1. 入力ファイルの配置  
   ランク名のディレクトリを作成し、その中に ./input/00-read ディレクトリ
   以下のファイルペアをコピーする。
   ここでは、12ペアのゲノムファイルを6ノード（MPIプロセス）で処理すると
   する。

        $ cd ./input
        $ mkdir -p 00-read-rank/0
        $ mkdir -p 00-read-rank/1
        $ mkdir -p 00-read-rank/2
        $ mkdir -p 00-read-rank/3
        $ mkdir -p 00-read-rank/4
        $ mkdir -p 00-read-rank/5
        $ cp 00-read/part_1.00 00-read-rank/0
        $ cp 00-read/part_2.00 00-read-rank/0
        $ cp 00-read/part_1.01 00-read-rank/0
        $ cp 00-read/part_2.01 00-read-rank/0
        $ cp 00-read/part_1.02 00-read-rank/1
        $ cp 00-read/part_2.02 00-read-rank/1
        $ cp 00-read/part_1.03 00-read-rank/1
        $ cp 00-read/part_2.03 00-read-rank/1
        $ cp 00-read/part_1.04 00-read-rank/2
        $ cp 00-read/part_2.04 00-read-rank/2
        $ cp 00-read/part_1.05 00-read-rank/2
        $ cp 00-read/part_2.05 00-read-rank/2
        $ cp 00-read/part_1.06 00-read-rank/3
        $ cp 00-read/part_2.06 00-read-rank/3
        $ cp 00-read/part_1.07 00-read-rank/3
        $ cp 00-read/part_2.07 00-read-rank/3
        $ cp 00-read/part_1.08 00-read-rank/4
        $ cp 00-read/part_2.08 00-read-rank/4
        $ cp 00-read/part_1.09 00-read-rank/4
        $ cp 00-read/part_2.09 00-read-rank/4
        $ cp 00-read/part_1.10 00-read-rank/5
        $ cp 00-read/part_2.10 00-read-rank/5
        $ cp 00-read/part_1.11 00-read-rank/5
        $ cp 00-read/part_2.11 00-read-rank/5
        $ cd ..

2. 実行  
   インタラクティブ実行

        $ mpiexec -n 6 ./bin/workflow ./input/bwa_db/reference.fa \
        ./input/seq_contig.md ./input/reference.fa ./input/reference.fa.fai \
        ./input/00-read-rank

   実行完了後、カレントディレクトリに workflow_OUT ディレクトリが作成され、
   結果ファイルが出力されている。

以上の手順1, 2を京でランクディレクトリを用いて実行するときのサンプルジョ
ブスクリプトを k_job_script/workflow-job.sh として用意した。このスクリ
プトでは12ノード使用する。

    $ pjsub ./k_job_script/workflow-job.sh


### ワークフローのステップ実行 ###

マッピング、重複除去、変異同定の3ステップを個別に逐次実行するスクリプトを
用意している。これらを実行して、プロファイルをとることにより、大規模解析時
の性能を外挿する。


#### マッピング ####

##### ファイルIO #####
* 入力ファイル
  * リードデータ1組(2ファイル)
  * BWA対応リファレンスシークエンス(6ファイル)
  * リファレンスシークエンス中のContigの情報
* 出力ファイル
  * Contigファイル群
* 中間ファイル
  * リード列のSuffix Array(SA)ファイル(2ファイル)
  * bwaマッピング結果(1ファイル)


##### 実行方法 #####
bin/01-alignment.sh を実行する。オプションとして入力リード列のSAを計算する
際のスレッド数を指定できる。スレッド数のデフォルトは8。

    $ ./bin/01-alignment.sh -t 8 ./input/bwa_db/reference.fa \
    ./input/seq_contig.md ./input/01-alignment/sra_1.fastq.0 \
    ./input/01-alignment/sra_2.fastq.0

中間ファイルは 01-alignment.sh_MED 、出力ファイルは 01-alignment.sh_OUT と
して生成される。

京用のジョブスクリプトを k_job_script/01-alignment-job.sh として用意してあ
る。


#### 重複除去 ####

##### ファイルIO #####
* 入力ファイル
  * SAMファイル
  * リファレンスシークエンスのインデックス
* 出力ファイル
  * BAMファイル(重複なし)
* 中間ファイル
  * BAMファイル(重複含む)
  * ソートされたBAMファイル(重複含む)


##### 実行方法 #####
bin/02-rmdup.sh を実行する。オプションとしてソート時のメモリ量を指定できる。
デフォルトは800MB。

    $ ./bin/02-rmdup.sh ./input/reference.fa.fai \
    ./input/02-rmdup/NT_008818.16.sam

中間ファイルは 02-rmdup.sh_MED 、出力ファイルは 02-rmdup.sh_OUT として生成
される。

京用のジョブスクリプトを k_job_script/02-rmdup-job.sh として用意してある。


#### 変異同定 ####

##### ファイルIO #####
* 入力ファイル
  * BAMファイル
  * リファレンスシークエンス
* 出力ファイル
  * 変異同定ファイル(3ファイル)
* 中間ファイル
  * PILEファイル


##### 実行方法 #####
bin/03-snp.sh を実行する。

    $ ./bin/03-snp.sh ./input/reference.fa ./input/03-snp/NT_008818.16.bam

中間ファイルは 03-snp.sh_MED 、出力ファイルは 03-snp.sh_OUT として生成され
る。

京用のジョブスクリプトを k_job_script/03-snp-job.sh として用意してある。


大規模解析時の性能外挿
------------------------

本ミニアプリが提供するプログラムとデータは小規模のものであるが、これらを用
いて大規模実行時の性能を外挿する手段を提供している。「ワークフローのステッ
プ実行」の各プログラムを実行し、プロファイルを取得した後に、その結果を付属
のExcelシート（model/perf-model_jp.xlsx）に入力することで、簡単な外挿が行
える。ただし、外挿モデルには以下の制約がある。

* ストレージの負荷は考慮していないため、算出されるIO性能は要求される最大性
  能を表す。また、多数ノード時の実行時間はモデルと実測で大きく異なることが
  ある。
* 「マージ」処理のコストはモデルに含まれていない。
* 「重複除去」と「変異同定」では、実行パターンによっては入力ファイルのサイ
  ズがファイル毎に大きく異なることがある。本モデルでは全ファイル同程度のサ
  イズの場合の性能予測は示すものであり、サイズが異なる場合の性能を示すもの
  ではない。


#### プロファイル取得 ####
プロファイラを用いて、「ワークフローのステップ実行」に示した3つのプログラ
ムを実行し、実行時間、FLOPS値、MIPS値、メモリスループットを取得する。また、
各プログラムの入力・中間・出力ファイルのサイズをカウントする。

本ミニアプリでは、京やFX10において、富士通プロファイラを用いて以上の情報を
取得するためのスクリプト群を提供する。利用の流れは以下の通りである。

1. プロファイルオプション「-p」をつけて各プログラムを実行する。

        $ ./bin/01-alignment.sh -p -t 8 ./input/bwa_db/reference.fa \
        ./input/seq_contig.md ./input/01-alignment/sra_1.fastq.0 \
        ./input/01-alignment/sra_2.fastq.0
        $ ./bin/02-rmdup.sh ../data/reference.fa.fai \
        ../data/02-rmdup/NT_005120.16.sam
        $ ./bin/03-snp.sh ../data/reference.fa ../data/03-snp/NT_005120.16.bam

   実行すると、中間ファイル、出力ファイルに加え、プロファイル
   （01-alignment.sh_PROF、02-rmdup.sh_PROF、03-snp.sh_PROF）が出力される。

   京用のジョブスクリプトを k_job_script/01-alignment-job-profile.sh 、
   k_job_script/02-rmdup-job-profile.sh 、 k_job_script/03-snp-job-profile.sh
   として用意してある。

2. 結果抽出プログラムを実行し、情報を得る。

        $ ./bin/parse-01-alignment.rb ./input/01-alignment/sra_1.fastq.0 \
        ./input/01-alignment/sra_2.fastq.0 01-alignment-job.sh_MED \
        01-alignment-job.sh_OUT 01-alignment-job.sh_PROF
        $ ./bin/parse-02-rmdup.rb ./input/02-rmdup/NT_008818.16.sam \
        ./02-rmdup-job.sh_MED ./02-rmdup-job.sh_OUT ./02-rmdup-job.sh_PROF
        $ ./bin/parse-03-snp.rb ./input/03-snp/NT_008818.16.bam \
        ./03-snp-job.sh_MED ./03-snp-job.sh_OUT ./03-snp-job.sh_PROF

   得られた結果をExcelシート2〜4ページの背景がオレンジ色のセルに書き込む。


#### 大規模実行時の性能予測 ####

「Summary」シート上部の表「Parameters for a large scale run」に各ステップ
ごとの入力ファイル数、CPU数を記入する（背景がオレンジのセル）。下の表
「Calculated performance of a large scale run」の値が更新される。


#### 実行時間制約を与えた時の要求性能の予測 ####

各ステップの処理を指定した時間内に終了させる際に要求される性能の予測値を計
算する。

「Summary」シートの表「Parameters for time restriction」に実行時間と処理対
象ファイル数（背景がオレンジのセル）を入力すると、表「Estimated performance
under given time threshold」に予測値が計算される。


#### 時間制約を与えたステージング要求性能の予測 ####

ステージングを採用しているシステムにおいて、指定された時間内に、各ステップ
での入出力データのステージングを完了するために必要なIO性能を計算する。

「Summary」シートの表「Parameters for time restriction - Staging」に、ステ
ージイン・アウトの要求時間、処理対象ファイル数、ノード数（背景がオレンジの
セル）を入力すると、表「Estimated time for staging under time threshold」に
予測値が計算される。


エクサスケールでの想定計算規模
--------------------------------

* 1ゲノムのデータ量
  * 現在の100倍（ヒト一人当たり100TB）
* 計算対象総量
  * ゲノム数: 200,000ゲノム
    * 総データ量は20,000倍（およそ2,000PB）
  * 要求計算量
    * 3年で解析する場合、1日あたり190ゲノムの処理が必要
      * 1ゲノムあたり平均455秒
    * 5年で解析する場合、1日あたり110ゲノムの処理が必要
      * 1ゲノムあたり平均785秒
    * ただし、1拠点で形跡しなければならない必要性はなく、複数箇所で分散解析
      しても良い


オリジナルNGS Analyzerからの変更点
------------------------------------

* ミニアプリ化に際し、不要となったプログラムのソースコードを削除
* コード削減
  * コマンドラインオプションの削減
  * 不要コード(コメント、不実行コード)の削除
* 京上でのワークフローを定義するシェルスクリプト群、およびそのサンプル設定
  ファイル群を削除
* ワークフロー実行スクリプトを実装
  * 複数MPIプロセスを用いて一括してワークフローを実行するスクリプト
  * ワークフローの主要ステップのみを独立して実行するスクリプト
