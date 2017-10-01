
from collections import defaultdict
import seaborn as sns
import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap
import pandas as pd
import json
import re
import itertools

## compare two lists with a heatmap
## kar hedwig output direktno

def compare_lists(file1,file2):
    
    termlist1 = []
    hedwig_out = defaultdict(list)
    
    print("processing:",file1,file2)
          
    df = pd.DataFrame(columns=("SDM","Enrichment","Match"))

    with open(file1) as f1:
        for line in f1:
            term = line.strip().split()[0]
            termlist1.append(term)

    compiled = re.compile("GO:.......")
    with open(file2) as data_file:    
        data = json.load(data_file)
        for k,v in data.items():
            for rule in v:                
                results = compiled.findall(str(rule))
                for result in results:
                    hedwig_out[k].append((result,len(results)))
            
    ### for term in termlist 1, if sam as termlist 2, then 1, else 0

    termlist1 = set(termlist1)
    cvals = []
    for k in hedwig_out.values():
        for x in k:
            cvals.append(x)

    cvals = set(cvals)    
    print(len(termlist1),len(cvals))

    for k,v in hedwig_out.items():
        for gt in termlist1:
            k = k.split("_")[0]
            dv = dict(v)
            l1,l2 = zip(*v)
            if gt in l1:
                df = df.append(pd.DataFrame([[k, gt,int(dv[gt])]], columns=["SDM","Enrichment","Match"]),ignore_index=True)
            else:
                pass
#                df = df.append(pd.DataFrame([[k, gt,int(0)]], columns=["SDM","Enrichment","Match"]),ignore_index=True)
    

    ##plot the dataset
    result = df.pivot(index='SDM', columns='Enrichment', values='Match')
    ax = sns.heatmap(result, cbar=False,yticklabels=True,xticklabels=True,linewidths=.01,cmap=ListedColormap(['green', 'yellow', 'red']))
    ax.set_xlabel("DAVID enrichment result terms",fontsize=15)
    ax.set_ylabel("CBSSD",fontsize=15)
    plt.yticks(rotation=0)
    plt.xticks(rotation=90)
    plt.show()
