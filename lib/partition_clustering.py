### this algorithm is capable of community clustering
### import community

import networkx as nx
import argparse
import rdflib
from collections import defaultdict
import itertools
from .parsers import parse_gaf_file,read_termlist

def run_infomap(infile,multiplex=False,overlapping="no"):

    from subprocess import call
    if multiplex:
        call(["infomap/Infomap", "tmp/multiplex_edges.net","out/","-i multiplex","-N 500","--silent"])       
    else:
        if overlapping == "yes":
            call(["infomap/Infomap", "tmp/monoplex_edges.net","out/","-N 300","--overlapping"])
            
        else:
            call(["infomap/Infomap", "tmp/monoplex_edges.net","out/","-N 300"])

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

    #nodemap = {x : y for y,x in enumerate(graph.nodes())}
    
    #inverse_nodemap = {k : f for f,k in nodemap.items()}

    ## refers to -> multiplex edge

    synonym_mappings = []
    
    for edge in graph.edges(data=True):
        if edge[2]['key'] == "codes_for":
            synonym_mappings.append((edge[0],edge[1]))

    nodemap = {}
    node_counter = 0
    synonym_nodes = set()
    
    for en, x in enumerate(synonym_mappings):
        nodemap[x[0]] = en
        nodemap[x[1]] = en
        node_counter = en+1
        synonym_nodes.add(x[0])
        synonym_nodes.add(x[1])

    synonym_map = nodemap ## for faster iteration
    ## one to many dict
    for edge in graph.edges(data=True):

        n1 = edge[0]
        n2 = edge[1]
        
        w = float(edge[2]['weight'])
        layer_first = layermap[n1.split("_")[0]]
        layer_second = layermap[n2.split("_")[0]]

        if n1 not in synonym_nodes:        
            nodemap[node_counter] = n1
            node_first = node_counter
            node_counter+=1
            
        else:
            for k,v in synonym_map.items():
                if v == n1:
                    node_first = k

        
        if n2 not in synonym_nodes:     
            nodemap[node_counter] = n2
            node_second = node_counter
            node_counter+=1
        else:
            for k,v in synonym_map.items():
                if v == n2:
                    node_first = k
                        
        outstruct.append((layer_first,node_first,layer_second,node_second,w))

    import os
    
    if not os.path.exists("tmp"):
        os.makedirs("tmp")

    if not os.path.exists("out"):
        os.makedirs("out")
        
    file = open("tmp/multiplex_edges.net","w")
    
    for el in outstruct:
        outstr = " ".join([str(x) for x in el])+"\n"
        file.write(outstr)
 
    file.close()

    ## run infomap
    print("INFO: Multiplex community detection in progress..")
    run_infomap("tmp/multiplex_edges.net",multiplex=True)
    partition = parse_infomap("out/multiplex_edges.tree")
    partitions = {}
    
    ## node, partition
    for k,v in partition.items():
        try:
            partitions[nodemap[k]] = v
        except:
            print("error in partition assignment:",k)

    import shutil
    
    shutil.rmtree("out", ignore_errors=False, onerror=None)
    shutil.rmtree("tmp", ignore_errors=False, onerror=None)
    
    return partitions


def monoplex_community(graph,overlapping="no"):
    print("INFO: Monoplex community detection in progress..")
    outstruct = []
    layermap = {x.split("_")[0] : y for y, x in enumerate(set(x.split("_")[0] for x in graph.nodes()))}

    nodemap = {x : y for y,x in enumerate(graph.nodes())}
    inverse_nodemap = {k : f for f,k in nodemap.items()}
    for edge in graph.edges():        
        node_first = nodemap[edge[0]]
        node_second = nodemap[edge[1]]
        outstruct.append((node_first,node_second))

    import os
    
    if not os.path.exists("tmp"):
        os.makedirs("tmp")

    if not os.path.exists("out"):
        os.makedirs("out")
        
    file = open("tmp/monoplex_edges.net","w")
    
    for el in outstruct:
        file.write(" ".join([str(x) for x in el])+"\n")
 
    file.close()

    ## run infomap
    run_infomap("tmp/monoplex_edges.net",multiplex=False,overlapping=overlapping)
    partition = parse_infomap("out/monoplex_edges.tree")
    partitions = {}
    for k,v in partition.items():
        try:
            partitions[inverse_nodemap[k]] = v
        except:
            pass

    import shutil
    
    print("Found {} partitions.".format(len(partitions)))
    
    shutil.rmtree("out", ignore_errors=False, onerror=None)
    shutil.rmtree("tmp", ignore_errors=False, onerror=None)
    return partitions

def return_community_mapping(predictions,termlist):
    community_map = []
    for k,v in predictions.items():
        try:
            term = k.split(":")[1]        
            if term in termlist:
                outterm = term+" "+str(v)
                community_map.append(outterm)
        except:
            pass ## mapping non-existent
        
    return community_map

def identify_components(G):
    comps = sorted(nx.connected_components(G),key=len,reverse=True)
    nmmap = {}
    for en,x in enumerate(comps):
        for j in x:
            nmmap[j] = en
    return nmmap

def partition_cluster_n3(input_graph, termlist_infile,mapping_file, output_n3,map_folder,method="louvain",multiplex = "no",community_size_threshold=0,overlapping="no",include_induced_neighborhood=True):
    Gx = nx.read_gpickle(input_graph)

    ## split into distinct layers before doing community detection            
    #nodes = G.nodes(data=False)
    #edges = G.edges(data=True)
    #Gx.add_nodes_from(nodes)
    #Gx.add_edges_from(edges)
    
    if method == "louvain":
        import community
        predictions = community.best_partition(Gx)
        
    if method == "infomap":
        if multiplex != "no":
            predictions = multiplex_community(Gx)
            
        else:
            predictions = monoplex_community(Gx, overlapping=overlapping)
            
    if method == "components":
        predictions = dict(identify_components(Gx))
    
    uniGO = parse_gaf_file(mapping_file)

    print ("INFO: number of terms parsed:",len(uniGO.keys()))
    termlist = read_termlist(termlist_infile)

    ## extract nodes, which are parts of communities -- works for UniProts currently.
    community_map = return_community_mapping(predictions,termlist)
    ## write to file
    with open(map_folder, 'w') as f:
        f.write("\n".join(community_map))

    ## generate input examples based on community assignment
    g = rdflib.graph.Graph()
    KT = rdflib.Namespace('http://kt.ijs.si/hedwig#')
    amp_uri = 'http://kt.ijs.si/ontology/hedwig#'
    obo_uri = "http://purl.obolibrary.org/obo/"
    AMP = rdflib.Namespace(amp_uri)

    ## include neighbors as instances or not..
    if include_induced_neighborhood:
        ntuple = [(k.split(":")[1],v) for k,v in predictions.items()]
    else:
        ntuple = [(x.split(" ")[0],x.split(" ")[1]) for x in community_map]
        
    id_identifier = 0
    
    ## iterate through community assignments and construct the trainset
    ## tukaj morda dodaj example name
    for node, com in ntuple:
        try:
            id_identifier += 1        
            u = rdflib.term.URIRef('%sexample#%s%s' % (amp_uri, node,str(id_identifier)))
            g.add((u, rdflib.RDF.type, KT.Example))
            g.add((u, KT.class_label, rdflib.Literal(str(com)+"_community")))
            for goterm in uniGO[node]:
                if "GO:" in goterm:
                    annotation_uri = rdflib.term.URIRef('%s%s' % (obo_uri, rdflib.Literal(goterm)))
                    blank = rdflib.BNode()
                    g.add((u, KT.annotated_with, blank))
                    g.add((blank, KT.annotation, annotation_uri))
        except:
            
            ## incorrect mappings are ignored..
            pass
        
    ## serialize to n3
    g.serialize(destination=output_n3, format='n3')
    
if __name__ == '__main__':

    ## example run
    parser_init = argparse.ArgumentParser()
    parser_init.add_argument("--input_graph", help="Graph in gpickle format.")
    parser_init.add_argument("--input_nodelist", help="Nodelist input..")
    parser_init.add_argument("--ontology_id", help="prediction_file..")
    parser_init.add_argument("--input_mapping", help="prediction_file..")
    
    parsed = parser_init.parse_args()
    G = nx.read_gpickle(parsed.input_graph)
    multiplex_community(G)    
