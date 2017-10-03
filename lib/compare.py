
from collections import defaultdict
import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap
import pandas as pd
import json
import re
import itertools

## compare two lists with a heatmap
## kar hedwig output direktno

def compare_lists(file1,file2,out):
    
    termlist1 = []
    hedwig_out = defaultdict(list)
    
    print("processing:",file1,file2)
          
    df = pd.DataFrame(columns=("SDM","Enrichment","Match"))

    with open(file1) as f1:
        for line in f1:
            term = line.strip().split()[0]
            termlist1.append(term)

    compiled = re.compile("GO:.......")
    resultterms = []
    with open(file2) as data_file:    
        data = json.load(data_file)
        for k,v in data.items():
            for rule in v:                
                results = compiled.findall(str(rule))
                for result in results:
                    resultterms.append(result)
                    hedwig_out[k].append((result,len(results)))
            
    ### for term in termlist 1, if sam as termlist 2, then 1, else 0

    termlist1 = set(termlist1)
    cvals = set(resultterms)
    print(len(termlist1),len(cvals))
    intersection = set.intersection(cvals,termlist1)
    print("Intersection %:",len(intersection)*100/len(termlist1))

    for k,v in hedwig_out.items():
        for gt in termlist1:
            k = k.split("_")[0]
            dv = dict(v)
            l1,l2 = zip(*v)
            if gt in l1:
                df = df.append(pd.DataFrame([[str(k), str(gt),int(dv[gt])]], columns=["SDM","Enrichment","Match"]),ignore_index=True)
            else:
                pass
#                df = df.append(pd.DataFrame([[k, gt,int(0)]], columns=["SDM","Enrichment","Match"]),ignore_index=True)
    
    ##plot the dataset
    result = df.pivot(index='SDM', columns='Enrichment', values='Match')
    
    result.fillna(value=np.nan, inplace=True)
    ax = sns.heatmap(result, cbar=False,yticklabels=True,xticklabels=True,linewidths=.1,cmap=ListedColormap(['green', 'red']))
    print("HM constructed..")
    ax.set_xlabel("DAVID enrichment result terms",fontsize=16,rotation=180)
    ax.set_ylabel("CBSSD (individual communities)",fontsize=16)
    plt.yticks(rotation=90,fontsize=5)
    plt.xticks(rotation=90,fontsize=5)
    fig = plt.gcf()
    print("Drawing..")
    fig.set_size_inches(12, 8)
    fig.savefig(out, dpi=800)
