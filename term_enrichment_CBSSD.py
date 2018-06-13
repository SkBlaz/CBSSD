## a script for running term enrichment

from lib.enrichment_modules import *

if __name__ == "__main__":

    print("Starting enrichment analysis..")
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--gaf_filename",default="./test.txt")
    parser.add_argument("--partition_mappings",default="./test.txt")
    parser.add_argument("--outfile",default="./test.txt")
    args = parser.parse_args()

    ## 1.) read the database.
    term_dataset, term_database, all_counts =  read_uniprot_GO(args.gaf_filename)
    
    ## 2.) partition function dict.
    topology_map = read_topology_mappings(args.partition_mappings)

    ## 3.) calculate p-vals.
    significant_results = compute_enrichment(term_dataset, term_database, topology_map, all_counts,whole_term_list=False)
    ## 4.) write the results
    significant_results.to_csv(args.outfile,sep=" ",header=False)
