
## get term lists in form of uniprot lists:
## for each term list

## conduct all experiments in a single step

## epigenetics - CBSSD + enrichment #######################################################

python3 CBSSD.py --step_size 10 --knowledge_graph network_datasets/epigenetics.gpickle --term_list term_lists/epigenetics.list --ontology_BK background_knowledge/goslim_generic.obo --output_BK experimental_evaluation_outputs/epiBK.n3 --n3_samples experimental_evaluation_outputs/epiSam_bmn.n3 --method components --gaf_mapping example_inputs/goa_human.gaf --community_map experimental_evaluation_outputs/partition_epigenetics_bmn_cbssd.txt --rule_output experimental_evaluation_outputs/bmn_epi_rules.json

## compute enriched terms
python3 term_enrichment_CBSSD.py --gaf_filename example_inputs/goa_human.gaf --partition_mappings experimental_evaluation_outputs/partition_epigenetics_bmn_cbssd.txt --outfile experimental_evaluation_outputs/bmn_epi_term_enrichment.txt

## evaluate
python3 evaluate_cbssd.py --input_gaf example_inputs/goa_human.gaf --enrichment_results experimental_evaluation_outputs/bmn_epi_rules.json --result_type rules --partition_map experimental_evaluation_outputs/partition_epigenetics_bmn_cbssd.txt >> evaluation_results/run_2.txt

python3 evaluate_cbssd.py --input_gaf example_inputs/goa_human.gaf --enrichment_results experimental_evaluation_outputs/bmn_epi_term_enrichment.txt --result_type terms --partition_map experimental_evaluation_outputs/partition_epigenetics_bmn_cbssd.txt >> evaluation_results/run_2.txt

## visualizaition part
#python3 heatmap.py --enrichment example_inputs/epi_gotermsDAVID.list --cbssd experimental_evaluation_outputs/Epirules.json --out experimental_evaluation_outputs/hm_epi.png

#python3 heatmap.py --enrichment example_inputs/diabDAVIDterms.list --cbssd experimental_evaluation_outputs/Diabrules.json --out experimental_evaluation_outputs/hm_diab.png

#python3 heatmap.py --enrichment example_inputs/snpsDAVID.list --cbssd experimental_evaluation_outputs/Snpsrules.json --out experimental_evaluation_outputs/hm_snps.png

## compute also statistics for each of the examples.
## compute tables for individual datasets
