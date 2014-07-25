#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <mpi.h>

/* #define MEASURE_TIME */

/* assume a shared directory */
#define GLOBAL_MED_DIR "../SH_GLOBAL"
#define BUF_LEN 256

static void setup_directories(char *cmddir, char *indir, char *meddir,
			      char *outdir, int rank, const char *prog_name,
			      const char *indir_prefix)
{
    /* find program directory */
    char *p = strrchr(prog_name, '/');
    if (p == NULL) {
	snprintf(cmddir, BUF_LEN, "./");
    } else {
	int i;
	for (i = 0; prog_name + i <= p; i++) {
	    cmddir[i] = *(prog_name + i);
	}
	cmddir[i] = '\0';
    }

    /* find program basename */
    char prog_basename[BUF_LEN];
    if (p == NULL) {
	snprintf(prog_basename, BUF_LEN, "%s", prog_name);
    } else {
	int i;
	for (i = 0; i < strlen(p) + 1; i++) {
	    prog_basename[i] = *(p + i + 1);
	}
    }

    /* create directories */
    snprintf(indir, BUF_LEN, "%s/%d", indir_prefix, rank);
    int ret = access(indir, F_OK);
    if (ret != 0) {
	fprintf(stderr,
		"Input files should be located under rank-named directory, "
		"like \"%s\"\n", indir);
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
    snprintf(meddir, BUF_LEN, "./%s_MED", prog_basename);
    snprintf(outdir, BUF_LEN, "./%s_OUT", prog_basename);
    ret = mkdir(meddir, S_IRWXU);
    if (ret != 0 && errno != EEXIST) {
	fprintf(stderr, "Failed to create Directory[%s].\n", meddir);
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
    ret = mkdir(outdir, S_IRWXU);
    if (ret != 0 && errno != EEXIST) {
	fprintf(stderr, "Failed to create Directory[%s].\n", outdir);
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if (rank == 0) {
	ret = mkdir(GLOBAL_MED_DIR, S_IRWXU);
	if (ret != 0 && errno != EEXIST) {
	    fprintf(stderr, "Failed to create Directory["GLOBAL_MED_DIR"].\n");
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}
    }
    MPI_Barrier(MPI_COMM_WORLD);
    snprintf(meddir, BUF_LEN, "./%s_MED/%d", prog_basename, rank);
    ret = mkdir(meddir, S_IRWXU);
    if (ret != 0) {
	fprintf(stderr, "Failed to create Directory[%s].\n", meddir);
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

static void cleanup_directories(int rank)
{
    if (rank == 0) {
	int ret = rmdir(GLOBAL_MED_DIR);
	if (ret != 0) {
	    fprintf(stderr, "Failed to delete Directory["GLOBAL_MED_DIR"].\n");
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}
    }
}

static void run_task(char *const *cmdline, const char *task_name, int rank)
{
    int ret = access(cmdline[0], R_OK);
    if (ret != 0) {
	fprintf(stderr,
		"Command[%s] is not found on Rank[%d].\n", cmdline[0], rank);
	MPI_Abort(MPI_COMM_WORLD, 1);
    }

#ifdef MEASURE_TIME
    double start = MPI_Wtime();
#endif

    pid_t pid = fork();
    if (pid < 0) {
	fprintf(stderr, "Failed to fork a process on RANK[%d].\n", rank);
	MPI_Abort(MPI_COMM_WORLD, 1);
    } else if (pid == 0) {
	/* child process */
	int ret = execv(cmdline[0], cmdline);
	if (ret < 0) {
	    fprintf(stderr, "Failed to run a command on RANK[%d].\n", rank);
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}
    } else {
	/* parent process */
	int status;
	pid_t rpid = waitpid(pid, &status, 0);
	if (rpid < 0) {
	    fprintf(stderr, "Failed to finish a command on RANK[%d].\n", rank);
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}
	if (!WIFEXITED(status)) {
	    fprintf(stderr, "Failed to finish a command on RANK[%d].\n", rank);
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}
    }

#ifdef MEASURE_TIME
    double end = MPI_Wtime();
    char timefn[BUF_LEN];
    snprintf(timefn, BUF_LEN, "./time.txt");
    FILE *fp = fopen(timefn, "a+");
    if (fp != NULL) {
	fprintf(fp, "%s: %f\n", task_name, (end - start));
    }
    fclose(fp);
#endif
}


int main(int argc, char **argv)
{
    int rank, nprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (argc != 6) {
	if (rank == 0) {
	    fprintf(stderr,
		    "Usage: %s BWA_DB_FILE CONTIG_FILE REFERENCE_FILE \\\n"
		    "                  REFERENCE_INDEX_FILE INPUT_DIR\n",
		    argv[0]);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 1;
    }

    char *bwa_db_file  = argv[1];
    char *contig_file  = argv[2];
    char *ref_file     = argv[3];
    char *ref_idx_file = argv[4];
    char cmddir[BUF_LEN], indir[BUF_LEN], meddir[BUF_LEN], outdir[BUF_LEN];
    setup_directories(cmddir, indir, meddir, outdir, rank, argv[0], argv[5]);
    MPI_Barrier(MPI_COMM_WORLD);

    /* mapping */
    char **cmd1 = (char **)malloc(6 * sizeof(char *));
    cmd1[0] = (char *)malloc(BUF_LEN * sizeof(char));
    snprintf(cmd1[0], BUF_LEN, "%s/workflow_01.sh", cmddir);
    cmd1[1] = indir;
    cmd1[2] = meddir;
    cmd1[3] = bwa_db_file;
    cmd1[4] = contig_file;
    cmd1[5] = NULL;
    run_task((char *const *)cmd1, "mapping", rank);
    free(cmd1[0]);
    free(cmd1);

    /* merge */
    char **cmd2 = (char **)malloc(5 * sizeof(char *));
    cmd2[0] = (char *)malloc(BUF_LEN * sizeof(char));
    snprintf(cmd2[0], BUF_LEN, "%s/workflow_02.sh", cmddir);
    cmd2[1] = (char *)malloc(BUF_LEN * sizeof(char));
    snprintf(cmd2[1], BUF_LEN, "%d", rank);
    cmd2[2] = meddir;
    cmd2[3] = GLOBAL_MED_DIR;
    cmd2[4] = NULL;
    run_task((char *const *)cmd2, "merge1", rank);
    free(cmd2[0]);
    free(cmd2[1]);
    free(cmd2);
    MPI_Barrier(MPI_COMM_WORLD);
    char **cmd3 = (char **)malloc(6 * sizeof(char *));
    cmd3[0] = (char *)malloc(BUF_LEN * sizeof(char));
    snprintf(cmd3[0], BUF_LEN, "%s/workflow_03.sh", cmddir);
    cmd3[1] = (char *)malloc(BUF_LEN * sizeof(char));
    snprintf(cmd3[1], BUF_LEN, "%d", rank);
    cmd3[2] = (char *)malloc(BUF_LEN * sizeof(char));
    snprintf(cmd3[2], BUF_LEN, "%d", nprocs);
    cmd3[3] = meddir;
    cmd3[4] = GLOBAL_MED_DIR;
    cmd3[5] = NULL;
    run_task((char *const *)cmd3, "merge2", rank);
    free(cmd3[0]);
    free(cmd3[1]);
    free(cmd3[2]);
    free(cmd3);

    /* rmdup */
    char **cmd4 = (char **)malloc(4 * sizeof(char *));
    cmd4[0] = (char *)malloc(BUF_LEN * sizeof(char));
    snprintf(cmd4[0], BUF_LEN, "%s/workflow_04.sh", cmddir);
    cmd4[1] = meddir;
    cmd4[2] = ref_idx_file;
    cmd4[3] = NULL;
    run_task((char *const *)cmd4, "rmdup", rank);
    free(cmd4[0]);
    free(cmd4);

    /* analyze */
    char **cmd5 = (char **)malloc(5 * sizeof(char *));
    cmd5[0] = (char *)malloc(BUF_LEN * sizeof(char));
    snprintf(cmd5[0], BUF_LEN, "%s/workflow_05.sh", cmddir);
    cmd5[1] = meddir;
    cmd5[2] = outdir;
    cmd5[3] = ref_file;
    cmd5[4] = NULL;
    run_task((char *const *)cmd5, "analyze", rank);
    free(cmd5[0]);
    free(cmd5);

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
	fprintf(stdout, "Finished.\n");
    }

    cleanup_directories(rank);
    MPI_Finalize();
    return 0;
}
