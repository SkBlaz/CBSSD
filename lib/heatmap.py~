###### this draws a heatmap of two lists along with their intersection

## read the first one, read the second one
## read heatmap


# file1 = "OUTPUT/eppi_terms.txt"
# file2 = "OUTPUT/rule_terms.txt"

file1 = "OUTPUT/snps_sign2.txt"
file2 = "OUTPUT/significant_eppiterms.txt"

import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd

termlist1 = []
termlist2 = []

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
ax.set_ylabel("Proposed SDM approach",fontsize=25)

plt.show()
