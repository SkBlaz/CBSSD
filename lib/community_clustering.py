### this algorithm is capable of community clustering


#import community
import networkx as nx
import argparse
import rdflib
from collections import defaultdict
import itertools
import community

def run_infomap(infile):

    from subprocess import call
    call(["infomap/Infomap", "tmp/multiplex_edges.net","out/","-i multiplex","-N 1000","--silent"])

def parse_infomap(outfile):

    outmap = {}
    with open(outfile) as of:
        for line in of:
            parts = line.strip().split()
            try:
                module = parts[0].split(":")[0]
                node = parts[3]
                outmap[int(node)] = int(module)
            except:
                pass

    return outmap

def multiplex_community(graph):

    outstruct = []
    layermap = {x.split("_")[0] : y for y, x in enumerate(set(x.split("_")[0] for x in graph.nodes()))}

    nodemap = {x : y for y,x in enumerate(graph.nodes())}
    inverse_nodemap = {k : f for f,k in nodemap.items()}
    for edge in graph.edges():
        
        layer_first = layermap[edge[0].split("_")[0]]
        layer_second = layermap[edge[1].split("_")[0]]
        node_first = nodemap[edge[0]]
        node_second = nodemap[edge[1]]
        outstruct.append((layer_first,node_first,layer_second,node_second,1))

    import os
    
    if not os.path.exists("tmp"):
        os.makedirs("tmp")

    if not os.path.exists("out"):
        os.makedirs("out")
        
    file = open("tmp/multiplex_edges.net","w")
    
    for el in outstruct:
        file.write(" ".join([str(x) for x in el])+"\n")        
 
    file.close() 

    ## run infomap
    print("INFO: Multiplex community detection in progress..")
    run_infomap("tmp/multiplex_edges.net")
    partition = parse_infomap("out/multiplex_edges.tree")
    partitions = {}
    for k,v in partition.items():
        try:
            partitions[inverse_nodemap[k]] = v
        except:
            pass

    import shutil
    
    shutil.rmtree("out", ignore_errors=False, onerror=None)
    shutil.rmtree("tmp", ignore_errors=False, onerror=None)

    return partitions


def community_cluster_n3(input_graph, termlist_infile,mapping_file, output_n3,map_folder,method="louvain"):

    G = nx.read_gpickle(input_graph)

    ## split into distinct layers before doing community detection        
    
    Gx = nx.Graph()
    nodes = G.nodes(data=False)
    edges = G.edges(data=False)
    Gx.add_nodes_from(nodes)
    Gx.add_edges_from(edges)

    if method == "louvain":    
        predictions = community.best_partition(Gx)

    if method == "infomap_multiplex":
        predictions = multiplex_community(Gx)
    
    uniGO = defaultdict(list)    
    with open(mapping_file) as im:
        for line in im:
            parts = line.split("\t")
            try:
                uniGO[parts[1]].append(parts[4]) ## GO and ref both added
                uniGO[parts[1]].append(parts[3])
            except:
                pass

    print ("INFO: number of terms parsed:",len(uniGO.keys()))
    termlist = []

    with open(termlist_infile) as nl:
        for line in nl:
            parts = line.strip().split()
            termlist.append(parts[0])

    community_map = []
    for k,v in predictions.items():
        term = k.split(":")[1]
        for te in termlist:
            if te == term:
                outterm = term+" "+str(v)
                community_map.append(outterm)
    with open(map_folder, 'w') as f:
        f.write("\n".join(community_map))


    g = rdflib.graph.Graph()
    KT = rdflib.Namespace('http://kt.ijs.si/hedwig#')
    amp_uri = 'http://kt.ijs.si/ontology/hedwig#'
    obo_uri = "http://purl.obolibrary.org/obo/"
    AMP = rdflib.Namespace(amp_uri)

    ## FOR PAIR, add to class community, node.split(), mapped term to class..

    ntuple = [(k,v) for k,v in predictions.items()]
    id_identifier = 0
    
    for node, com in ntuple:
        node = node.split(":")[1]
        id_identifier += 1        
        u = rdflib.term.URIRef('%sexample%s' % (amp_uri, com))
        g.add((u, rdflib.RDF.type, KT.Example))
        g.add((u, KT.class_label, rdflib.Literal(str(com)+"_community")))
        for goterm in uniGO[node]:
            if "GO:" in goterm:
                annotation_uri = rdflib.term.URIRef('%s%s' % (obo_uri, rdflib.Literal(goterm)))
                blank = rdflib.BNode()
                g.add((u, KT.annotated_with, blank))
                g.add((blank, KT.annotation, annotation_uri))

    g.serialize(destination=output_n3, format='n3')    
    

if __name__ == '__main__':

    parser_init = argparse.ArgumentParser()
    parser_init.add_argument("--input_graph", help="Graph in gpickle format.")
    parser_init.add_argument("--input_nodelist", help="Nodelist input..")
    parser_init.add_argument("--ontology_id", help="prediction_file..")
    parser_init.add_argument("--input_mapping", help="prediction_file..")
    
    parsed = parser_init.parse_args()
    G = nx.read_gpickle(parsed.input_graph)
    multiplex_community(G)    
