## parse a binary interaction file to gpickle object used for training

import argparse
import networkx as nx

parser_init = argparse.ArgumentParser()
parser_init.add_argument("--bin_file", help="When building the graph..") # Mandatory
parser_init.add_argument("--out_gpickle", help="When building the graph..") # Mandatory
parser_init.add_argument("--score_threshold", help="When building the graph..",type=float,default=0.5) # Mandatory
args = parser_init.parse_args()

G = nx.Graph()
with open(args.bin_file) as abf:
    for line in abf:
        line = line.strip().split("\t")
        uni_first = line[0]
        uni_second = line[1]
        if "intact-miscore" in line[14]:            
            try:
                conf = float(line[14].split(":")[1])
                if conf > args.score_threshold:
                    G.add_edge(uni_first,uni_second,score=conf)                
            except:
                pass
            
print (nx.info(G))
nx.write_gpickle(G, args.out_gpickle)
