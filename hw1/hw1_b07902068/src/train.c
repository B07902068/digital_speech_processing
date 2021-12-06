#include "hmm.h"
#include <stdio.h>
#include <assert.h>

typedef struct sequence{
	int len;
	char data[MAX_SEQ];
	double gamma[MAX_SEQ][MAX_STATE];
	double epsilon[MAX_SEQ][MAX_STATE][MAX_STATE];
} SEQ;
SEQ sequence[10001];
double alpha[MAX_SEQ][MAX_STATE] = {}, beta[MAX_SEQ][MAX_STATE] = {};

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

void forward(double alpha[][MAX_STATE], SEQ *seq, HMM *hmm)
{
	int N = hmm->state_num;
	int T = seq->len;
	for (int i = 0; i < N; i++) {
		alpha[0][i] = hmm->initial[i] * hmm->observation[seq->data[0]][i];	
	}
	for (int t = 0; t < T - 1; t++) {
		for (int j = 0; j < N; j++) {
			double sum = 0;
			for (int i = 0; i < N; i++) {
				sum += alpha[t][i] * hmm->transition[i][j];
			}
			alpha[t+1][j] = sum * hmm->observation[seq->data[t + 1]][j];
		}
	}
}

void backward(double beta[][MAX_STATE], SEQ *seq, HMM *hmm)
{
	int N = hmm->state_num;
	int T = seq->len;
	for (int i = 0; i < N; i++) {
		beta[T-1][i] = 1;
	}
	for (int t = T - 2; t >= 0; t--) {
		for (int i = 0; i < N; i++) {
			beta[t][i] = 0;
			for (int j = 0; j < N; j++) {
				beta[t][i] += hmm->transition[i][j] * hmm->observation[seq->data[t+1]][j] * beta[t+1][j];
			}
		}
	}
}

void gamma_epsilon(double alpha[][MAX_STATE], double beta[][MAX_STATE], SEQ *seq, HMM *hmm)
{
	int N = hmm->state_num;
	int T = seq->len;
	double P_olambda = 0;
	for (int i = 0; i < N; i++) {
		P_olambda += alpha[T-1][i];
	}

	//gamma
	for (int t = 0; t < T; t++) {
		for (int i = 0; i < N; i++) {
			seq->gamma[t][i] = (alpha[t][i] * beta[t][i]) / P_olambda;
		}
	}

	//epsilon
	for (int t = 0; t < T - 1; t++) {
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				seq->epsilon[t][i][j] = (alpha[t][i] * hmm->transition[i][j] * hmm->observation[seq->data[t + 1]][j] * beta[t+1][j]) / P_olambda;
			}
		}
	}
	
		
}

void update_model(int seq_num, SEQ sequence[], HMM *hmm)
{
	// N = seq_num
	int qN = hmm->state_num;
	int T  = 0;	

	// pi = initial
	for (int i = 0; i < qN; i++) {
		double sum = 0;
		for (int n = 0; n < seq_num; n++) {
			sum += sequence[n].gamma[0][i];
		}
		hmm->initial[i] = sum / (double)seq_num; 
	}

	// denominator of new A
	double bot_sum[MAX_STATE] = {};
	for (int i = 0; i < qN; i++) {
		for (int n = 0; n < seq_num; n++) {
			T = sequence[n].len;
			for (int t = 0; t < T - 1; t++) {
				bot_sum[i] += sequence[n].gamma[t][i];
			} 
		}
	}

 	//A = transtion
	for (int i = 0; i < qN; i++) {
		for (int j = 0; j < qN; j++) {
			double top_sum = 0;
			for (int n = 0; n < seq_num; n++) {
				T = sequence[n].len;
				for (int t = 0; t < T-1; t++) {
					top_sum += sequence[n].epsilon[t][i][j];
				} 
			}
			hmm->transition[i][j] = top_sum / bot_sum[i];
		}
	}

	
	// denominator of new B
	for (int i = 0; i < qN; i++) {
		for (int n = 0; n < seq_num; n++) {
			T = sequence[n].len;
			bot_sum[i] += sequence[n].gamma[T - 1][i];
		}
	}
	
	//B = obervation probability
	for (int k = 0; k < hmm->observ_num; k++) {
		for (int i = 0; i < qN; i++) {
			double top_sum = 0;
			for (int n = 0; n < seq_num; n++) {
				T = sequence[n].len;
				for (int t = 0; t < T; t++) {
					if (sequence[n].data[t] == k) {
						top_sum += sequence[n].gamma[t][i];
					} 
				}
			}
			hmm->observation[k][i] = top_sum / bot_sum[i];
		}
	}
}


//./train <iter> <model_init_path> <seq_path> <output_model_path>
//./train 100 model_init.txt data/train_seq_01.txt model_01.txt
int main(int argc, char *argv[])
{
	assert(argc == 5);
	int iter = atoi(argv[1]), seq_num = 0;
	char *model_init_path = argv[2], *seq_path = argv[3], *output_path = argv[4];
	
	HMM hmm;
	loadHMM( &hmm, "model_init.txt" );
//	dumpHMM( stderr, &hmm);
	
	seq_num = read_seq(sequence, seq_path);
		
	for (int i = 0; i < iter; i++) {
		for (int n = 0; n < seq_num; n++) {
			forward(alpha, &sequence[n], &hmm);
			backward(beta, &sequence[n], &hmm);
			gamma_epsilon(alpha, beta, &sequence[n], &hmm);
		}
		update_model(seq_num, sequence, &hmm);	
	}

//	fprintf(stderr, "\n\n");
//	dumpHMM( stderr, &hmm);





	FILE *out_fp = open_or_die(output_path, "w");
	dumpHMM(out_fp, &hmm);


	return 0;
}
