
import json
import sys
import re
import glob

lower_part_category = ["Trousers", "Shorts", "Jeans"]

collar_opt = ['rollneck', 'v neck','zip collar', 'crew neck', 'hoodie', 'shirt-collar']
pattern_opt = ['printed', 'striped', 'plain', 'check']
sleeve_opt = ['long sleeve', 'short sleeve', 'vest']
material_opt = ['cotton', 'denim', 'leather', 'cashmere', 'poly']

class metadata_reader(object):

    def __init__(self, product_code, metadata_dir_path):
        
        
        path_pattern = "%s/*/%s.txt"%(metadata_dir_path, product_code)
        ret_path = glob.glob(path_pattern)
        assert(len(ret_path)>0)
        self.img_path = ret_path[0]
        f = open(self.img_path)
        json_str = f.read()
        self.metadata = json.loads(json_str)
        
        if self.metadata['clothing_category'] in lower_part_category:
            self.attribute = {
                'clothing_category': None,
                'pattern': None,
                'material': [],
                'pair_item': [],
                'pants_len': None
            }
        else: # upper part
            self.attribute = {
                'clothing_category': None,
                'collar': None,
                'sleeve_len': None,
                'pattern': None,
                'button': None,
                'material': [],
                'pair_item': []
            }

        self.metadata_dir_path = metadata_dir_path
        #print self.metadata['pair_item']
        self.metadata['pair_item'] = self._trim_non_exist_pair_item(self.metadata['pair_item'])
        self.attribute['pair_item'] = self.metadata['pair_item']
        #print "trimmed_pair_item", self.metadata['pair_item']
        self._fill_attr()

    def _trim_non_exist_pair_item(self, pair_item_list):
        trimmed_pair_item_list = []
        for item in pair_item_list:
            item_path = self.metadata_dir_path + '/*/%s.txt'%item
            if len(glob.glob(item_path)) > 0:
                trimmed_pair_item_list.append(item)
        return trimmed_pair_item_list

    def get_attr(self):
        return self.attribute

    def _fill_attr(self):
        self.attribute['clothing_category'] = self.metadata['clothing_category']
        for opt in pattern_opt:
            if self.is_str_in_field(opt, ['product_name', 'pageClass', 'detail_care']):
                self.attribute['pattern'] = opt
                break
        for opt in material_opt:
            if self.is_str_in_field(opt, ['product_name','detail_care']):
                if opt not in self.attribute['material']:
                    self.attribute['material'].append(opt)


        if self.metadata['clothing_category'] in lower_part_category:
            if self.metadata['clothing_category'] == 'Shorts':
                self.attribute['pants_len'] = "short"
            else:
                self.attribute['pants_len'] = "long"
        else: # upper part
            #default attribute setting
            if self.metadata['clothing_category'] == 'T-Shirts':
                self.attribute['sleeve_len'] = sleeve_opt[1]  # short sleeve
            elif self.metadata['clothing_category'] == 'Knitwear':
                self.attribute['sleeve_len'] = sleeve_opt[0]  # long sleeve
                if self.is_str_in_field('CARDIGANS', ['pageClass']):
                    self.attribute['button'] = 'yes'

            elif self.metadata['clothing_category'] == 'Coats and Jackets':
                self.attribute['sleeve_len'] = sleeve_opt[0]  # long sleeve

            # shirts, not t-shirts
            elif 'Shirts' in self.metadata['clothing_category']:
                self.attribute['button'] = 'yes'
                self.attribute['collar'] = 'shirt-collar'
                
            elif self.metadata['clothing_category']=='Polos':
                self.attribute['collar'] = 'shirt-collar'
                
            for opt in sleeve_opt:
                if self.is_str_in_field(opt, ['product_name', 'pageClass', 'detail_care']):
                    self.attribute['sleeve_len'] = opt
                    break
            for opt in collar_opt:
                if self.is_str_in_field(opt, ['product_name', 'pageClass', 'detail_care']):
                    self.attribute['collar'] = opt
                    break
        


    def is_str_in_field(self, keyword, keys):
        keyword = self.normalized_string(keyword)
        for key in keys:
            desc = self.normalized_string(self.metadata[key])            
            if keyword in desc:
                #print keyword, desc
                return True
        return False
    def normalized_string(self, input_str):
        input_str = input_str.lower()
        input_str = re.sub('[-_]', ' ', input_str)
        return input_str


if __name__ == "__main__":
    assert(len(sys.argv) == 3)
    print sys.argv[1]
    #metadata_reader(product_code, metadata_dir)
    metadata_reader(sys.argv[1], sys.argv[2])
