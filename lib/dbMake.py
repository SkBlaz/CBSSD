## this simple script enables download of .obo and .gaf files, required for specific species.

import subprocess
import urllib.request
import os
import shutil


def download_gaf():        
#    http://geneontology.org/gene-associations/goa_uniprot_all_noiea.gaf.gz
#    urllib.urlretrieve ("hhttp://geneontology.org/gene-associations/goa_human.gaf", "./example_inputs/human_goa.gaf")
#    print("GAF file successfully downloaded..")
    pass

def download_obo(ontology_folder = "./ontologies"):
    
    print ("INFO: Constructing the ontology database..")
    if not os.path.exists(ontology_folder):
        os.makedirs(ontology_folder)

    url = "http://purl.obolibrary.org/obo/go/go-basic.obo"
    out_file = ontology_folder+"/go-basic.obo"
    with urllib.request.urlopen(url) as response, open(out_file, 'wb') as to_write:
        shutil.copyfileobj(response, to_write)
        
    print("GO file successfully downloaded..")
