### this script converts obo based ontology to n3 based one..

import argparse
from collections import defaultdict
import rdflib
import os


def obo2n3(obofile,n3out):

    ontology = defaultdict(list)
    current_term = ""
    obofile = obofile.replace("/","")

    ## iterate through all files
    for onto in os.listdir(obofile):
        if ".obo" in onto:
            obopath = obofile+"/"+onto
            print ("INFO: parsing the",obopath)
            with open(obopath) as obo:
                for line in obo:            
                    parts = line.split()
                    try:
                        if parts[0] == "id:":
                            current_term = parts[1]
                        if parts[0] == "is_a:":
                            ontology[current_term].append(parts[1])
                    except:
                        pass

    print ("INFO: ontology terms added:", len(ontology.keys()))
    ## construct an ontology graph
    g = rdflib.graph.Graph()            
    KT = rdflib.Namespace('http://kt.ijs.si/hedwig#')
    amp_uri = 'http://kt.ijs.si/ontology/hedwig#'
    obo_uri = "http://purl.obolibrary.org/obo/"
    AMP = rdflib.Namespace(amp_uri)        

    for k,v in ontology.items():
        u = rdflib.term.URIRef('%s%s' % (obo_uri, k))
        for item in v:
            annotation_uri = rdflib.term.URIRef('%s%s' % (obo_uri, rdflib.Literal(item)))
            g.add((annotation_uri, rdflib.RDFS.subClassOf,u))
        
        
    g.serialize(destination=n3out,format="n3")
    

if __name__ == '__main__':
    
    parser_init = argparse.ArgumentParser()
    parser_init.add_argument("--input_obo", help="Graph in gpickle format.")
    parser_init.add_argument("--output_n3", help="Graph in gpickle format.")
    parsed = parser_init.parse_args()
    obo2n3(parsed.input_obo,parsed.output_n3)
    
