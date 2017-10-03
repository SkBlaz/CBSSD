
## conduct all experiments in a single step

## epigenetics
python3 CBSSD.py --step_size 2 --knowledge_graph example_outputs/epigenetics.gpickle --term_list example_inputs/epigenetics.list --ontology_BK example_inputs --output_BK example_outputs/epiBK.n3 --n3_samples example_outputs/epiSam.n3 --gaf_mapping example_inputs/goa_human.gaf --community_map example_outputs/EpiCom.txt --rule_output example_outputs/Epirules.json

## diabetes
# python3 CBSSD.py --step_size 2 --knowledge_graph example_outputs/diabetes.gpickle --term_list example_inputs/uniprot-diabetes.list --ontology_BK example_inputs --output_BK example_outputs/diabBK.n3 --n3_samples example_outputs/diabSam.n3 --gaf_mapping example_inputs/goa_human.gaf --community_map example_outputs/diabCom.txt --rule_output example_outputs/Diabrules.json

## SNPs
python3 CBSSD.py --step_size 2 --knowledge_graph example_outputs/snps.gpickle --term_list example_inputs/snps_clean.list --ontology_BK example_inputs --output_BK example_outputs/snpsBK.n3 --n3_samples example_outputs/snpsSam.n3 --gaf_mapping example_inputs/goa_human.gaf --community_map example_outputs/snpCom.txt --rule_output example_outputs/Snpsrules.json

## visualizaition part
python3 heatmap.py --enrichment example_inputs/epi_gotermsDAVID.list --cbssd example_outputs/Epirules.json --out example_outputs/hm_epi.png

#python3 heatmap.py --enrichment example_inputs/diabDAVIDterms.list --cbssd example_outputs/Diabrules.json --out example_outputs/hm_diab.png

python3 heatmap.py --enrichment example_inputs/snpsDAVID.list --cbssd example_outputs/Snpsrules.json --out example_outputs/hm_snps.png
