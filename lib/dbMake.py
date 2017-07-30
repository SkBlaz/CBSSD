## this simple script enables download of .obo and .gaf files, required for specific species.

import subprocess
import urllib.request
urllib.request.urlretrieve

def download_gaf():        
#    http://geneontology.org/gene-associations/goa_uniprot_all_noiea.gaf.gz
    urllib.urlretrieve ("hhttp://geneontology.org/gene-associations/goa_human.gaf", "./example_inputs/human_goa.gaf")
    print("GAF file successfully downloaded..")



def download_obo():        
    urllib.urlretrieve ("http://purl.obolibrary.org/obo/go/go-basic.obo", "./example_inputs/go-basic.obo")
    print("GO file successfully downloaded..")
