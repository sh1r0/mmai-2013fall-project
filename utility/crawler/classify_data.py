import json
import os

def mkdir(dirpath):
	if not os.path.exists(dirpath):
	        	os.mkdir(dirpath)
for filename in os.listdir("metadata"):
    if filename.endswith(".txt"):
        print  filename
        filename = os.path.join("metadata",filename)
        file = open(filename, 'r')
        json_str = file.read()
        product_data = json.loads(json_str)
        product_code =  product_data['product_code']
        print product_data['clothing_category']
        img_dirpath = 'images/'+product_data['clothing_category']
        mkdir(img_dirpath)
        meta_dirpath = 'metadata/'+product_data['clothing_category']
        mkdir(meta_dirpath)
        command ='mv %s \'%s\''%(filename, meta_dirpath)
        print command
        os.system(command)
        command = 'mv images/%s_*.jpg \'%s\''%(product_code, img_dirpath)
        print command
        os.system(command)
        file.close()
        