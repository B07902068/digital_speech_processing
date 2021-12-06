#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <assert.h>
#include "File.h"
#include "Ngram.h"
#include "Vocab.h"

using namespace std;

Vocab voc;
Ngram lm(voc, 2);

double getBigramProb(const char *w1, const char *w2)
{
	VocabIndex wid1 = voc.getIndex(w1);
	VocabIndex wid2 = voc.getIndex(w2);

	if(wid1 == Vocab_None) //OOV
		wid1 = voc.getIndex(Vocab_Unknown);
	if(wid2 == Vocab_None) //OOV
		wid2 = voc.getIndex(Vocab_Unknown);

	VocabIndex context[] = { wid1, Vocab_None };
	return lm.wordProb( wid2, context);
	
}

string removeBlank(string s) 
{
	string::iterator iter;
	for (iter = s.begin(); iter != s.end();) {
		if (*iter == ' ') {
			iter = s.erase(iter);
		} else {
			iter++;
		}
	}
	return s;
}
void readMap(char map_path[], map<string, string> &mapping)
{
	ifstream infile(map_path);
	string line;
	while(getline(infile, line)) {
		line = removeBlank(line);
		string key = line.substr(0, 2);
		mapping[key] = line.substr(2, line.length()-2);
	}
	infile.close();
}
string Viterbi(string s, map<string, string> mapping)
{
	int T = s.length();
	double delta[T / 2 + 2][10000] = {};
	int psi[T / 2 + 2][10000] = {};
	
	int qT = 0;
	double P_star = -999;
/*	
	string w1;
	string w2;
	string states1;
	string states2;*/
	string w  = s.substr(0, 2);
	string states = mapping[w];
	
	int N = states.length();
	for (int j = 0; j < N; j += 2) {
		const char *wj = (states.substr(j, 2)).c_str();
		delta[0][j] = getBigramProb("<s>", wj);
	}

	for(int t = 0; t < T - 2; t += 2) {
		string w1  = s.substr(t, 2);
		string w2  = s.substr(t+2, 2);
		string states1 = mapping[w1];
		string states2 = mapping[w2];
		
		int N1 = states1.length();
		int N2 = states2.length();
		for (int j = 0; j < N2; j += 2) {
			double max = -999;
			const char *wj = (states2.substr(j, 2)).c_str();

			for (int i = 0; i < N1; i += 2) {
				const char *wi = (states1.substr(i, 2)).c_str();
				double prob = getBigramProb(wi, wj);

				if (delta[t/2][i] + prob > max) {
					max = delta[t/2][i] + prob;
					psi[t/2 + 1][j] = i;
				}

			}

			delta[t/2 + 1][j] = max;
			if (t == T - 4) {
				max = delta[t/2 + 1][j] + getBigramProb(wj, "</s>");
				if (max > P_star) {
					qT = j;
					P_star = max;
				}
			}
		}


	}
	
	string result = "";
	//string w;
	//string states; 
	for (int t = T/2 - 1; t >= 0; t--) {
		string w = s.substr(2 * t, 2);
		string states = mapping[w];
		string best = states.substr(qT, 2);
		
		result = " " +  best + result;
		qT = psi[t][qT];
	}
	result = "<s>" + result + " </s>\n";
	
	return result;
}

int main(int argc, char *argv[])
{
	assert(argc == 5);

	{
		File lmFile(argv[3], "r");
		lm.read(lmFile);
		lmFile.close();
	}
	char c[2];
	c[0] = 0xA4;
	c[1] = 0x40;
//	char *cs=  "<s>";
	string s(c);

	map<string, string> mapping;
	readMap(argv[2], mapping);
	//printf("%f\n", getBigramProb(c, "</s>"));

	ifstream inFile(argv[1]);
	ofstream outFile(argv[4]);
	string line;
	while (getline(inFile,line) ) {
		line = removeBlank(line);	
		string result = Viterbi(line, mapping);
		outFile << result; 
	}	
	inFile.close();
	outFile.close();

	return 0;	
	
}
