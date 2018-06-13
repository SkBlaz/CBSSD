## compute some result statistics

import Orange
import matplotlib.pyplot as plt
from collections import defaultdict
import operator
import pandas as pd
import numpy as np

def plot_critical_distance(fname):

    colx = ["Mean WRAcc","Max WRAcc","Min WRAcc","Mean IC","Max IC","Min IC","Mean overall IC","Max overall IC","Min overall IC","Coverage"]
    for tx in colx:
        rkx = fname.groupby(['dataset','algorithm'])[tx].mean()
        print(rkx)
        ranks = defaultdict(list)
        clf_ranks = defaultdict(list)
        for df,clf in rkx.index:
            print(df,clf)
            ranks[df].append((clf,rkx[(df,clf)]))

        print(ranks)
        for k,v in ranks.items():
            a = dict(v)
            sorted_d = sorted(a.items(), key=operator.itemgetter(1))
            for en, j in enumerate(sorted_d):
                print(en,j[0])
                clf_ranks[j[0]].append(len(sorted_d)-en)

        print(clf_ranks)
        clf_score = {k : np.mean(v) for k,v in clf_ranks.items()}
        names = list(clf_score.keys())
        avranks = list(clf_score.values())
        cd = Orange.evaluation.compute_CD(avranks, 10, alpha="0.05")
        Orange.evaluation.graph_ranks(avranks, names, cd=cd, width=6, textspace=1.5,reverse=True)
        plt.text(0.7, 0.1, "Measure: "+tx, fontsize=10)
        plt.show()

def result_frame_get(fname):

    cnames = ["dataset","algorithm","Mean WRAcc","Max WRAcc","Min WRAcc","Mean IC","Max IC","Min IC","Mean overall IC","Max overall IC","Min overall IC","Coverage"]
    rframe = pd.read_csv(fname,sep=" ",names = cnames)
    rframe['algorithm'] = rframe['algorithm']+" ("+rframe['dataset'].apply(lambda x: "".join(x.split("_")[0:2]))+")"

    rframe['algorithm'] = rframe['algorithm'].replace(
        {'rules (bmngoslim)': 'Rules (BMN+GOslim)',
         'rules (intactgoslim)': 'Rules (IntAct+GOslim)',
         'rules (bmngo)': 'Rules (BMN+GO)',
         'rules (intactgo)': 'Rules (IntAct+GO)',
         'terms (bmngoslim)': 'Terms (BMN+GOslim)',
         'terms (intactgoslim)': 'Terms (IntAct+GOslim)',
         'terms (bmngo)': 'Terms (BMN+GO)',
         'terms (intactgo)': 'Terms (IntAct+GO)'})
    
    rframe['dataset'] = rframe['dataset'].apply(lambda x: x.split("_")[2])

    return rframe
    
if __name__ == "__main__":

    import argparse
    parser_init = argparse.ArgumentParser()
    parser_init.add_argument("--fname", help="result file name..") # Mandatory
    parsed = parser_init.parse_args()

    rf = result_frame_get(parsed.fname)
    plot_critical_distance(rf)
