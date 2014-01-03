import json
import os

lower_part = ['Trousers', 'Jeans', 'Shorts']
exclude_part = ['Underwear', 'Pyjamas', 'Swimwear']
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
        if product_data['clothing_category'] in exclude_part:
            continue
        if product_data['clothing_category'] in lower_part:
            img_dirpath = 'images/lower_part'
            meta_dirpath = 'metadata/lower_part'
        else:
            img_dirpath = 'images/upper_part'
            meta_dirpath = 'metadata/upper_part'
        mkdir(img_dirpath)
        mkdir(meta_dirpath)
        command ='mv %s \'%s\''%(filename, meta_dirpath)
        print command
        os.system(command)
        command = 'mv images/%s_*.jpg \'%s\''%(product_code, img_dirpath)
        print command
        os.system(command)
        file.close()
        