## this will be the python interface for command line execution..

###### example
#python3 CBSSD.py --step_size 10 --knowledge_graph example_outputs/epigenetics.gpickle --term_list example_inputs/epigenetics.list --ontology_BK example_inputs --output_BK example_outputs/epiBK.n3 --n3_samples example_outputs/epiSam.n3 --gaf_mapping example_inputs/goa_human.gaf --community_map example_outputs/EpiCom.txt --rule_output example_outputs/Epirules.txt

######


################

#python3 CBSSD.py --step_size 10 --knowledge_graph example_outputs/diabetes.gpickle --term_list example_inputs/uniprot-diabetes.list --ontology_BK example_inputs --output_BK example_outputs/diabBK.n3 --n3_samples example_outputs/diabSam.n3 --gaf_mapping example_inputs/goa_human.gaf --community_map example_outputs/diabcom.txt --rule_output example_outputs/diabrules.txt

#######

## internal imports
from lib.get_bk import *
from lib.obo2n3 import *
from lib.community_clustering import *
from lib.dbMake import * ## this creates the mandatory databases

import subprocess
import argparse

if __name__ == '__main__':

    parser_init = argparse.ArgumentParser()
    parser_init.add_argument("--step_size", help="When building the graph..") # Mandatory
    parser_init.add_argument("--knowledge_graph", help="Nodelist input..") # Mandatory
    parser_init.add_argument("--term_list", help="Input term list in arbitrary bio-format..") # Mandatory
    parser_init.add_argument("--ontology_BK", help="Background ontology in .obo file..") # Mandatory
    parser_init.add_argument("--output_BK", help="Background knowledge outfile..")
    parser_init.add_argument("--n3_samples", help="Learning samples, derived from the term list..") # Mandatory
    parser_init.add_argument("--gaf_mapping", help="GAF map file, from term to GO..") # Mandatory
    parser_init.add_argument("--rule_output", help="Results..") # Mandatory

    parser_init.add_argument("--community_map", help="Identified subgroups..") # Mandatory
    parser_init.add_argument("--download_minimal", help="Download GAF and obo files..")
    
    parsed = parser_init.parse_args()
    source = read_example_datalist(parsed.term_list,whole=True)

    hedwig_command = "python2 hedwig/hedwig BK/ "+parsed.n3_samples+" -o "+parsed.rule_output+" -l --adjust=none --beam=10"


    ## either download ontology or use own
    if parsed.download_minimal:
        download_obo("./ontologies")
        obo2n3("./ontologies", parsed.output_BK)

    ## generate the graph
    if parsed.step_size:
        request = make_request()
        result_graph = request.execute_query_inc(source,div=int(parsed.step_size),connected=False)

        print ("STEP 1: Writing pickle datadump..")        
        nx.write_gpickle(result_graph, parsed.knowledge_graph)

    ## parse custom .obo folder
    print("STEP 2: Background knowledge processing..")
    if parsed.ontology_BK:
        obo2n3(parsed.ontology_BK, parsed.output_BK)

    ## identify subgroups
    print ("STEP 3: subgroup identification")
    community_cluster_n3(parsed.knowledge_graph,parsed.term_list,parsed.gaf_mapping,parsed.n3_samples,parsed.community_map)

    ## learn details about subgroups
    print ("STEP 4: Learning")
    print("HEDWIG: "+hedwig_command)
    process = subprocess.Popen(hedwig_command.split(), stdout=subprocess.PIPE)
    output, error = process.communicate()
    print(output)

