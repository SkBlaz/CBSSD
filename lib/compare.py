
import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd

## compare two lists with a heatmap

def compare_lists(file1,file2):
    
    termlist1 = []
    termlist2 = []
    
    print("processing:",file1,file2)
          
    df = pd.DataFrame(columns=("SDM","Enrichment","Match"))

    with open(file1) as f1:
        for line in f1:
            term = line.strip().split()[0]
            termlist1.append(term)

    with open(file2) as f2:
        for line in f2:
            term = line.strip().split()[0]
            termlist2.append(term)

    ### for term in termlist 1, if sam as termlist 2, then 1, else 0

    termlist1 = set(termlist1)
    termlist2 = set(termlist2)
    
    print(len(termlist1),len(termlist2))

    for term in termlist1:
        for term2 in termlist2:
            if term == term2:
                df = df.append(pd.DataFrame([[term, term2,int(1)]], columns=["SDM","Enrichment","Match"]),ignore_index=True)
        
            else:
                df = df.append(pd.DataFrame([[term, term2,int(0)]], columns=["SDM","Enrichment","Match"]),ignore_index=True)

    ##plot the dataset

    result = df.pivot(index='SDM', columns='Enrichment', values='Match')
    ax = sns.heatmap(result, annot=False, fmt="g", cbar=False,yticklabels=False,xticklabels=False,linewidths=.1,linecolor="lightgray")

    #ax.set(xlabel='Conventional enrichment analysis', ylabel='Semantic rule learning',fontsize=30)

    ax.set_xlabel("Conventional enrichment analysis (DAVID)",fontsize=25)
    ax.set_ylabel("CBSSD",fontsize=25)

    plt.show()

