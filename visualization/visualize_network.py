## this python code uses in house py3plex lib for complex network visualization

import matplotlib.pyplot as plt
import networkx as nx
from collections import defaultdict
from py3plex.multilayer import *
import argparse

parser_init = argparse.ArgumentParser()
parser_init.add_argument("--input_graph", help="Load graph file")
parser = parser_init.parse_args()

def view_by_type(inputgraph,limit=False):

    input_graph = nx.read_gpickle(inputgraph)
    type_segments = defaultdict(list)
    
    for node in input_graph.nodes(data=True):
        type_segments[node[0].split("_")[0]].append(node[0])        
    
    networks = []
    labs = []
    
    for k,v in type_segments.items():
        if limit != False:
            tmp_graph = input_graph.subgraph(v[0:limit]) 
        else:
            tmp_graph = input_graph.subgraph(v)
            
        #if tmp_graph.number_of_edges() > 2:
        labs.append(k)
        tmp_pos=nx.spring_layout(tmp_graph)
        nx.set_node_attributes(tmp_graph,'pos',tmp_pos)
        networks.append(tmp_graph)

    print ("Visualizing..")
    draw_multilayer_default(networks,background_shape="circle",display=False,labels=labs,networks_color="rainbow")
    mx_edges = []

    for e in input_graph.edges():
        if e[0].split("_")[0] != e[1].split("_")[0]:

            ## we have a multiplex edge!
            layer1 = e[0].split("_")[0]
            layer2 = e[1].split("_")[0]            
            mx_edges.append((e[0],e[1]))
            
    
    draw_multiplex_default(networks,mx_edges,alphachannel=0.2,linepoints="-.",linecolor="black",curve_height=5,linmod="upper",linewidth=0.2)

    plt.show()

def view_basic(inputgraph):

    input_graph = nx.read_gpickle(inputgraph)
    type_segments = defaultdict(list)

    ## for each node, remember the color..
    ncold = [n[0].split("_")[0] for n in input_graph.nodes(data=True)]
    d = {ni: indi for indi, ni in enumerate(set(ncold))}
    cols = [d[ni] for ni in ncold]
    nsizes = list(nx.degree(input_graph).values())
    nx.draw_spring(input_graph,node_color=cols,node_size=nsizes)
    plt.show()
    
view_by_type(parser.input_graph)
view_basic(parser.input_graph)
