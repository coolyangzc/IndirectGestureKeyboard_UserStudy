#include "Keyboard.h"
#include "DataReader.h"

#include <map>
#include <cmath>
#include <cstdio>
#include <vector>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

const int USER_L = 1;
const int RANK = 13;

const int SAMPLE_NUM = 32;

double dtw[MAXSAMPLE][MAXSAMPLE];

string dict[LEXICON_SIZE];
int freq[LEXICON_SIZE];
map<string, double> katz_alpha;
map<string, ll> unigram_map;
map<pair<string, string>, double> bigram_map;
vector<Vector2> dict_location[LEXICON_SIZE][2], dict_shape[LEXICON_SIZE][2]; //[scale: 0 for 1x1, 1 for 1x3]

string userID;
fstream checkFout;

void initLexicon()
{
    fstream fin;
    fin.open("bigrams-LDC-10k-katz-1m-phrase.txt", fstream::in);
    string s1, s2;
    int bigrams_num;
    ll freq;
    double prob;

    fin >> bigrams_num;
    cout << "Bigram pairs: " << bigrams_num << endl;
    rep(i, bigrams_num)
    {
        fin >> s1 >> s2 >> prob;
        bigram_map[mk(s1, s2)] = prob;
    }
    fin.close();

    checkFout.open("res/Bigrams_OOV.txt", fstream::out);
}

void init()
{
    initKeyboard(dtw);
    initLexicon();
}

void check(int id)
{
    string pre;
    rep(w, words.size())
    {
        if (w)
            pre = words[w-1];
        else
            pre = "<s>";
        if (bigram_map[mk(pre, words[w])] == 0)
            checkFout << pre << " " << words[w] << endl;
    }
}

int main()
{
    init();
    FOR(p, USER_L - 1, USER_NUM - 1)
    {
        userID = id[p];
        rep(i, PHRASES)
        {
            readData(user[p], i);
            check(i);
        }
    }
    return 0;
}

