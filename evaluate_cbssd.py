## evaluation script

from lib.evaluation import *

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
