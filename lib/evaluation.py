## this python script serves for evaluation of obtained rules on the ontology level
import numpy as np
import json
import re
from collections import defaultdict,Counter
from parsers import parse_gaf_file,read_termlist,read_uniprot_GO,read_topology_mappings

def load_terms(tfile):
    terms = []
    with open(tfile) as tfile:
        for line in tfile:
            line = line.strip()
            terms.append(line)
    return terms
    
def load_rules(rfile):

    compiled = re.compile("GO:\d+")
    hedwig_out = {}
    with open(rfile) as data_file:    
        data = json.load(data_file)
        for k,v in data.items():
            enr = {}            
            for en, rule in enumerate(v):                
                results = compiled.findall(str(rule))
                enr[en] = results                    
            hedwig_out[k] = enr
    return hedwig_out


def WRAcc_measure_rules(topology_mapping, rules_individual,term_dataset):

    ## N = len topmap
    ## n(C) all examples of this community
    ## n(Cnd) covered examples
    ## n(cnnd and c) correctly covered examples

    all_examples = []
    for k,v in topology_mapping.items():
        all_examples.append(v)
        
    class_counts = {x : len(y) for x,y in topology_mapping.items()}
    all_examples = set.union(*all_examples)
    N = len(all_examples)

    all_terms = set.union(*topology_mapping.values())
    all_wracc = []
    for annotated_community, rules in rules_individual.items():
        try:
            unis = topology_mapping[annotated_community.split("_")[0]]
        except:
            unis = topology_mapping[annotated_community]
        for k,v in rules.items():            
            all_examples_target = len(unis)
            covered_target = 0
            covered_all = 0
            for uni in unis:
                terms = term_dataset[uni]
                for x in v:
                    if x in terms:
                        covered_target+=1
                    else:
                        break

            for uni in all_terms:
                terms = term_dataset[uni]
                for x in v:
                    if x in terms:
                        covered_all+=1
                    else:
                        break

#            print(covered_all,N,covered_target,all_examples_target)
            try:
                wracc = (covered_all/N)*((covered_target/covered_all)-all_examples_target/N)
                all_wracc.append(wracc)
                
            except:
                
                pass            

    return (np.mean(all_wracc),np.amax(all_wracc),np.amin(all_wracc))

def get_p_term(term,term_database,all_terms):
    return len(term_database[term])/all_terms


def ic_measure_term_list(term_list,term_dataset,all_c):

    inverse_term_dataset = defaultdict(set)
    for x,y in term_dataset.items():
        for j in y:
            inverse_term_dataset[j].add(x)
    results = []
    for en, term in enumerate(term_list):
        pic = 0
        total_ic=-np.log(get_p_term(term,inverse_term_dataset,all_c))
        results.append((term,total_ic))
        
    return results
        
def ic_measure_rules(topology_mapping, rules_individual, term_dataset, all_c):

    inverse_term_dataset = defaultdict(set)
    for x,y in term_dataset.items():
        for j in y:
            inverse_term_dataset[j].add(x)
    
    all_ic = []
    partition_specific_ic = []
    for annotated_community, rules in rules_individual.items():
        pic = 0
        for name, rule in rules.items():
            total_ic = 0
            for term in rule:
                total_ic+=-np.log(get_p_term(term,inverse_term_dataset,all_c))
            all_ic.append(total_ic)
            pic+=total_ic
            
        partition_specific_ic.append(pic)

    
    return (np.mean(all_ic),np.amax(all_ic),np.amin(all_ic),np.mean(partition_specific_ic),np.amax(partition_specific_ic),np.amin(partition_specific_ic))


def compute_coverage_rules(rules_individual, term_dataset, topology_map):

    all_unis = set.union(*topology_map.values())
    all_terms = set()
    for annotated_community, rules in rules_individual.items():
        for k,v in rules.items():
            for x in v:
                all_terms.add(x)
                
    inverse_mappings = defaultdict(list)
    for k,v in term_dataset.items():
        for j in v:
            inverse_mappings[j].append(k)

    all_tmaps = set.union(*[set(inverse_mappings[x]) for x in all_terms])
    return len(set.intersection(all_tmaps,all_unis))/len(all_unis)
    
def compute_statistics_rules(gaf_file,result_file,inputs,ftype="rules",outname=None):

    ## return values for results    
    term_dataset, term_database, all_counts =  read_uniprot_GO(gaf_file,verbose=False)
    topology_map = read_topology_mappings(inputs)
    if ftype == "rules":
        rules_individual = load_rules(result_file)
    else:
        rules_individual = result_file
    mean_wracc,max_wracc,min_wracc = WRAcc_measure_rules(topology_map,rules_individual,term_dataset)

    mean_ic,max_ic,min_ic,mean_pic,max_pic,min_pic = ic_measure_rules(topology_map,rules_individual,term_dataset,all_counts)

    coverage = compute_coverage_rules(rules_individual,term_dataset,topology_map)
    print(outname, ftype, mean_wracc,max_wracc,min_wracc,mean_ic,max_ic,min_ic,mean_pic,max_pic,min_pic,coverage)

def convert_terms_to_rules(tfile):

    ## parse output from local enrichment
    cmap = defaultdict(list)
    with open(tfile) as tfl:
        for line in tfl:
            line = line.strip().split()
            cmap[line[1]].append([line[2]])

    nd = {}
    for k,v in cmap.items():
        v = {e : j for e,j in enumerate(v)}
        nd[k] = v

    return nd
    
if __name__ == "__main__":

    import argparse
    parser_init = argparse.ArgumentParser()    
    parser_init.add_argument("--input_gaf", help="Used ontology as background knowledge")
    parser_init.add_argument("--enrichment_results", help="hewdig outputs")
    parser_init.add_argument("--result_type", help="hewdig outputs")
    parser_init.add_argument("--partition_map", help="Accession input file.")
    parser = parser_init.parse_args()

    if parser.result_type == "rules":
        compute_statistics_rules(parser.input_gaf,parser.enrichment_results,parser.partition_map,ftype="rules",outname = parser.enrichment_results.split("/")[-1].split(".")[0])
        
    elif parser.result_type == "terms":
        ruleset = convert_terms_to_rules(parser.enrichment_results)
        compute_statistics_rules(parser.input_gaf,ruleset,parser.partition_map,ftype="terms",outname = parser.enrichment_results.split("/")[-1].split(".")[0])

    elif parser.result_type == "single_terms":

        ## compute IC for only a list of individual terms!
        terms = []
        with open(parser.enrichment_results) as er:
            for line in er:
                line = line.strip()
                terms.append(line)
        print(terms)
    term_dataset, term_database, all_counts =  read_uniprot_GO(parser.input_gaf,verbose=False)        
    termwise_IC = ic_measure_term_list(terms,term_dataset,all_counts)
    for pair in termwise_IC:
        print(pair)
        
        
