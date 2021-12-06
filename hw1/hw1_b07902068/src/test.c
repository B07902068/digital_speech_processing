#include "hmm.h"
#include <stdio.h>
#include <assert.h>

typedef struct sequence{
	int len;
	char data[MAX_SEQ];
} SEQ;
SEQ sequence[2501];
double delta[MAX_SEQ][MAX_STATE] = {};

int read_seq(SEQ sequence[], char *seq_path)
{
	FILE *seq_fp = open_or_die(seq_path, "r");
	int i = 0;
	while (fscanf(seq_fp, "%s", sequence[i].data) != EOF) {
		sequence[i].len = strlen(sequence[i].data);
		for (int j = 0; j < sequence[i].len; j++){
			sequence[i].data[j] -= 'A';
		}
		i++;
	}
	return i;
}

double viterbi(double delta[][MAX_STATE], SEQ *seq, HMM *hmm)
{
	int N = hmm->state_num;
	int T = seq->len;
	for (int i = 0; i < N; i++) {
		delta[0][i] = hmm->initial[i] * hmm->observation[seq->data[0]][i];
	}
	for (int t = 1; t < T; t++) {
		for (int j = 0; j < N; j++) {
			double max = 0, temp;
			for (int i = 0; i < N; i++) {
				temp = delta[t - 1][i] * hmm->transition[i][j];
				if (temp > max) {
					max = temp;
				}
			}
			delta[t][j] = max * hmm->observation[seq->data[t]][j];
		}
	}
	double p_star = 0;
	for (int i = 0; i < N; i++) {
		if (delta[T-1][i] > p_star) {
			p_star = delta[T-1][i];
		}
	}
	return p_star;
}

void accuracy(char mylabels[][50])
{
	FILE *fp = open_or_die("./data/test_lbl.txt", "r");
	int i = 0;
	char labels[2501][50];
	while (fscanf(fp, "%s", labels[i]) != EOF) {
		i++;
	}
	int count = 0;
	for (int n = 0; n < 2500; n++) {
		if (strcmp(mylabels[n], labels[n]) == 0) {
			count++;
		}
	}
	fprintf(stderr, "accuracy = %f\n", (double)count / 2500);
}
char mylabels[2501][50];
//./test <models_list_path> <seq_path> <output_result_path>
//./test modellist.txt data/test_seq.txt result.txt
int main(int argc, char *argv[])
{
	assert(argc == 4);
	char *models_list_path = argv[1], *seq_path = argv[2], *result_path = argv[3];
	int count = 0, seq_num = 0;
	HMM hmms[5];
	count = load_models(models_list_path, hmms, 5);
	seq_num = read_seq(sequence, seq_path);

	
	FILE *result_fp = open_or_die(result_path, "w");
	int maxID;
	double maxP;
	for (int n = 0; n < seq_num; n++) {
		maxID = 0;
		maxP = 0;
		double temp = 0;
		for (int i = 0; i < count; i++) {
			temp = viterbi(delta, &sequence[n], &hmms[i]);
			if (temp > maxP) {
				maxP = temp;
				maxID = i;
			}
		}
		fprintf(result_fp, "%s %e\n", hmms[maxID].model_name, maxP);
//		strcpy(mylabels[n], hmms[maxID].model_name);
	}	
//	accuracy(mylabels);
	return 0;
}
