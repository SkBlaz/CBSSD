## this python script serves for evaluation of obtained rules on the ontology level
import numpy as np
import json
import re
from collections import defaultdict

def parse_gaf(ifile):
    
    container = defaultdict(list)    
    with open(ifile) as inf:
        for line in inf:
            line = line.strip().split("\t")
            try:
                container[line[4]].append(line[1])
            except:
                pass

            
    return container


def load_terms(tfile):
    terms = []
    with open(tfile) as tfile:
        for line in tfile:
            line = line.strip()
            terms.append(line)
    return terms

def load_rules(rfile):

    compiled = re.compile("GO:.......")
    hedwig_out = defaultdict(list)
    resultterms = []
    with open(rfile) as data_file:    
        data = json.load(data_file)
        for k,v in data.items():
            for rule in v:                
                results = compiled.findall(str(rule))
                for result in results:
                    resultterms.append(result)
                    hedwig_out[k].append(result)


    return (hedwig_out, resultterms)


if __name__ == "__main__":

    import argparse
    parser_init = argparse.ArgumentParser()    
    parser_init.add_argument("--input_gaf", help="Used ontology as background knowledge")
    parser_init.add_argument("--rules", help="hewdig outputs")
    parser_init.add_argument("--terms", help="enrichment outputs")
    parser = parser_init.parse_args()
    
    count_data = parse_gaf(parser.input_gaf)
    count_data = {k : len(set(v)) for k,v in count_data.items()}
    total_count = sum(count_data.values())
    print(total_count)

    termset = load_terms(parser.terms)
    ruleset, all_termrules = load_rules(parser.rules)

    all_termrules = set(all_termrules)
    termset = set(termset)
    ## GO coverage
    print("Number rules: {}, Number terms: {}".format(len(all_termrules),len(termset)))

    ## interestingness
    itr= []
    itr2 = []
    for x in all_termrules:
        tcount1 = 0
        try:
            tcount1 += count_data[x]
            itr.append(-np.log(count_data[x]/total_count))
        except:
            pass

    for x2 in termset:
        tcount2 = 0
        try:
            tcount2 += count_data[x2]
            itr2.append(-np.log(count_data[x]/total_count))
        except:
            pass
            
    intR = tcount1/len(all_termrules)
    intT = tcount2/len(termset)
    itr = np.mean(itr)/len(all_termrules)
    itr2 = np.mean(itr2)/len(termset)
    
    ## term coverage
    print("Mean coverage rules: {}, Mean coverage terms: {}".format(intR,intT))
    print("Interestingness: {}, Interestingness: {}".format(itr,itr2))
