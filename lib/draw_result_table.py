import json
import sys

with open(sys.argv[1]) as data_file:    
    data = json.load(data_file)


table = "\\begin{tabular}{| l | c | c | c | r |} \hline \n"
table+="Dataset"+" & "+"Heuristic"+" & "+"No. nodes"+" & "+"No. edges & "+"Time(s)  \\\ \n \hline \hline \n"
for item in data:
    dataset,heuristic,nodes,edges,time = item
    table+=dataset.split("/")[1].split(".")[0]+" & "+heuristic+" & "+str(nodes)+" & "+str(edges)+" & "+str(round(time,2))+" \\\ \hline \n"


table += "\end{tabular}"

print(table)
