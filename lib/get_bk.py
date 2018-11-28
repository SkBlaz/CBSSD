## This is some basic code used to obtain the data from the Biomine API
#from joblib import Parallel, delayed
#import multiprocessing## and transform it into a graph, which will be further on processed.

import argparse
import urllib.request
import urllib.parse
import json
import urllib
import requests
import networkx as nx
import numpy as np
#import rdfmodule as rm
import sys
import os


class make_request:

    def __init__(self):

        self.db_url =  'https://biomine.ijs.si/list_databases'
        self.bm_api = 'https://biomine.ijs.si/api'
        self.databases = json.loads(urllib.request.urlopen(self.db_url).read().decode())['databases']
        self.graph = nx.MultiGraph()
        self.graph_nodes = []
        
    def get_info(self):

        print ('databases:{}  API:{}'.format(self.db_url,self.bm_api))

    def list_db(self):

        print (self.databases)

    def test_query(self):
        
        ##additional source and target nodes can be added.

        #params2 = urllib.parse.urlencode({'sourceTerms': 'InterPro:IPR011364, EntrezGene:672','graph_type': 'json'}).encode('utf-8')

        params = urllib.parse.urlencode({'database': self.databases['biomine'][0],
                                    'sourceTerms': 'EntrezGene:27086',
                                    'targetTerms': 'EntrezGene:93986',
                                    'maxnodes': 500,
                                    'grouping': 5,
                                         'graph_type': 'json'}).encode("utf-8")


        json_graph =  json.loads(urllib.request.urlopen(self.bm_api, params).read().decode())['graph']
        
        ## save for possible further use..
        print ("Data obtained, constructing the graph..")
        nodes = json.loads(json_graph)['nodes']
        edges = json.loads(json_graph)['links']
        ## colors

        ## lets create a graph..

        G = nx.Graph()

        ## those are the names..
        labels = {}
        for id,node in enumerate(nodes):
            #print (id, node['id'])
            G.add_node(id)
            labels[id] = node['id']
        for id,edge in enumerate(edges):
            #print (id, edge['source'],edge['target'])
            G.add_edge(int(edge['source']),int(edge['target']))
            
        
        print ("Finished")
        pos = nx.spring_layout(G)
        nx.draw(G,pos)
        nx.draw_networkx_labels(G,pos,labels,font_size=16)
        plt.show()

        return G

    def execute_query_inc(self,sourceterms,targetterms=None, maxnodes=2000,grouping=0, div=4,connected=False):
        
        max_terms = len(sourceterms)
        ## init the graph structure..

        G = self.graph

        step = div

#        step = int(len(sourceterms)/div)

        if step > 1500:
            step = 1000
        # to make it run in parallel, simply divide sourceterms by step before this loop
        
        print ("Initiating the graph construction phase with step: ",str(step))

        tmplist = []
        iteration = 0
        
        for e,k in enumerate(sourceterms):

            tmplist.append(k)
            
            if e % step == 0 and e > 0:

                sterms = ",".join(tmplist)
                tmplist = []                
                try:
                    if targetterms == None:

                        params = {'database': self.databases['biomine'][0],
                                  'sourceTerms': sterms,
                                  'maxnodes': maxnodes,
                                  'grouping': grouping,
                                  'maxnodes':10000,
                                  'graph_type': 'json'}
                        
                        graph_obj = requests.post(self.bm_api, params).json()['graph']
                        json_graph =  json.loads(graph_obj)
                        
                        
                except Exception as es:
                    print ("Walker did not find any relevant connections..")
                    json_graph = json.dumps({'nodes' : [],'links' : []})
                    continue
        
                ## save for possible further use..
                print ("Progress: ",str(round(float(e/max_terms)*100,2)),"% complete.", nx.info(self.graph))

                iteration += 1
                nodes = json_graph['nodes']
                edges = json_graph['links']               
                node_hash = {}

                for id,node in enumerate(nodes):

                    col_value = "r"
                    
                    try:

                        if node['organism'] == 'hsa':

                            col_value = "r"
                        
                        elif node['organism'] == 'mmu':
                        
                            col_value = "g"
                            
                        else:

                            col_value = "y"
                            
                    except:
                        pass
                        
                    node_hash[id] = (node['id'], node['degree'],col_value)
                    if iteration == 1:
                        self.graph_nodes.append(node['id'])
                    
                for edge in edges:

                    sourceterms = node_hash[int(edge['source'])]
                    targets = node_hash[int(edge['target'])]

                    reliability = 1
                    try:
                        reliability = edge['reliability']
                    except:
                        pass ## edge is empirical!
                    if connected == False:

                        G.add_node(sourceterms[0],degree=sourceterms[1],color=sourceterms[2])
                        G.add_node(targets[0],degree=targets[1],color=targets[2])
                        G.add_edge(sourceterms[0],targets[0],weight=reliability, key=edge['key'])
                        
                    else:                        
                        if targets[0] in self.graph_nodes or sourceterms[0] in self.graph_nodes:

                            if targets[0] not in self.graph_nodes:
                                self.graph_nodes.append(targets[0])
                            if sourceterms[0] not in self.graph_nodes:
                                self.graph_nodes.append(sourceterms[0])
                            
                            G.add_node(sourceterms[0],degree=sourceterms[1],color=sourceterms[2])
                            G.add_node(targets[0],degree=targets[1],color=targets[2])
                            G.add_edge(sourceterms[0],targets[0],weight=reliability, key=edge['key'])


        ## color according to db entry at least.           
        nodesG = G.nodes(data=True)
        print ("Final size: \n",nx.info(G))

        ## assign values to the object for further use
        
        self.graph_node_degree = [int(u[1]['degree']) for u in nodesG]
        self.graph_node_colors = [u[1]['color'] for u in nodesG]    
        self.graph_weights = [v[2]['weight'] for v in G.edges(data=True)]
        self.graph = G
        return G

    def reset_graph(self):

        self.graph = nx.Graph()
        
    def get_graph(self):

        return self.graph    
    
    def trim_graph(self, degreetrim):

        print ("Trimming the graph..")
        to_remove = [n if self.graph.degree(n) < degreetrim else None  for n in self.graph.nodes()]
        self.graph.remove_nodes_from(to_remove)

    def draw_graph(self, labs = False, weights = True, fsize = 10):

        nsize = [deg*0.1 for deg in self.graph_node_degree]

        if weights == False:

            if labs == True:

                nx.draw(self.graph,self.pos,node_size=nsize, node_color = self.graph_node_colors)
                nx.draw_networkx_labels(self.graph,self.pos,self.labels,font_size=16)
                plt.show()
            else:
                nx.draw(self.graph,self.pos,node_size=nsize,node_color = self.graph_node_colors)
                #nx.draw_networkx_labels(self.graph,self.pos,font_size=16)
                plt.show()
        else:
            if labs == True:
                nx.draw(self.graph,self.pos, width=self.graph_weights,node_size=nsize,node_color = self.graph_node_colors)
                nx.draw_networkx_labels(self.graph,self.pos,self.labels,font_size=fsize)
                plt.show()
            else:
                nx.draw(self.graph,self.pos, width=self.graph_weights,node_size=nsize,node_color = self.graph_node_colors)
                #nx.draw_networkx_labels(self.graph,self.pos,font_size=fsize)
                plt.show()

    def draw_graph_ortolog(self, labs = False, weights = True, fsize = 10):
        import matplotlib.pyplot as plt
        nsize = [deg*0.1 for deg in self.graph_node_degree_ortolog]

        if weights == False:

            if labs == True:

                nx.draw(self.graph_ortolog,self.pos,node_size_ortolog=nsize, node_color = self.graph_node_colors_ortolog)
                nx.draw_networkx_labels(self.graph_ortolog,self.pos_ortolog,self.labels_ortolog,font_size=16)
                plt.show()
            else:
                nx.draw(self.graph_ortolog,self.pos_ortolog,node_size=nsize,node_color = self.graph_node_colors_ortolog)
                nx.draw_networkx_labels(self.graph_ortolog,self.pos_ortolog,font_size=16)
                plt.show()
        else:
            if labs == True:

                nx.draw(self.graph_ortolog,self.pos_ortolog, width=self.graph_weights_ortolog,node_size=nsize,node_color = self.graph_node_colors_ortolog)
                nx.draw_networkx_labels(self.graph_ortolog,self.pos_ortolog,self.labels_ortolog,font_size=fsize)
                plt.show()
            else:
                nx.draw(self.graph_ortolog,self.pos_ortolog, width=self.graph_weights_ortolog,node_size=nsize,node_color = self.graph_node_colors_ortolog)
                nx.draw_networkx_labels(self.graph_ortolog,self.pos_ortolog,font_size=fsize)
                plt.show() 

    def export_graph(self,gname):

        nx.write_gml(G, gname+".gml")
        
        return

def read_example_datalist(datafile,whole=False,database="UniProt:"):

    outlist = []
    with open(datafile) as cl:
        for line in cl:
            outlist.append(database+line.replace("\n",""))

    return (outlist)        

def visualize_multiplex_biomine(input_graph,limit=False):

    from collections import defaultdict
    
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
    draw_multilayer_default(networks,background_shape="circle",display=False,labels=labs)

    mx_edges = []

    for e in input_graph.edges():
        if e[0].split("_")[0] != e[1].split("_")[0]:

            ## we have a multiplex edge!
            layer1 = e[0].split("_")[0]
            layer2 = e[1].split("_")[0]            
            mx_edges.append((e[0],e[1]))
            
    
    draw_multiplex_default(networks,mx_edges,alphachannel=0.2)

#    plt.show()
    plt.savefig('foo.png')

if __name__ == '__main__':

    import matplotlib.pyplot as plt
    from py3plex.multilayer import * ## visualization
    
    parser_init = argparse.ArgumentParser()
    
    parser_init.add_argument("--step_size", help="How large subgraphs are taken when building main graph")
    parser_init.add_argument("--output_name", help="Custom job ID for saving graph")
    parser_init.add_argument("--visualize", help="Basic network visualization with networkx module")
    parser_init.add_argument("--ontology_output", help="Ontology mapping generation")
    parser_init.add_argument("--py3plex", help="Multiplex network visualization")
    parser_init.add_argument("--instructions", help="Load the bio identifier lists in separate files into data folder and run this tool at least with --step_size option")
    parser_init.add_argument("--visualize_multiplex", help="Multiplex BioMine graph image generator..")
    parser_init.add_argument("--output_image", help="Image outfile..")
    parser_init.add_argument("--term_list", help="Image outfile..")
    parser = parser_init.parse_args()
    
    source = read_example_datalist(parser.term_list,whole=True)
    ## init a request
    
    request = make_request()

    print(source)
    ## this returns graph for further reduction use..
    
    if(parser.step_size):

        result_graph = request.execute_query_inc(source,div=int(parser.step_size),connected=False)

        if (parser.output_name):
            job_id = parser.output_name
            print ("Writing pickle datadump..")
            nx.write_gpickle(result_graph, "graph_datasets/"+job_id+".gpickle")

        ## possible direct community detection and output in time.. 
            
        if (parser.visualize): 
            request.draw_graph(labs=False)

        ##separate layers
        if (parser.visualize_multiplex):
            visualize_multiplex_biomine(result_graph,parser.output_image)
