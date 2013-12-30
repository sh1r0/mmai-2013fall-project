
import json
import sys
import re

lower_part_category = ["Trousers","Shorts", "Jeans"]

collar_opt = ['rollneck', 'v neck','zip collar', 'crew neck','hoodie','shirt-collar']
pattern_opt = ['printed', 'striped', 'plain', 'checked']
sleeve_opt = ['long sleeve', 'short sleeve', 'vest']
material_opt = ['cotton', 'denim', 'leather','cashmere','poly']
class metadata_reader():
	metadata = ""
	attribute = {
		'clothing_category':None,
		'collar':None,
		'sleeve_len':None,
		'pattern':None,
		'button':None,
		'material':[]
	}
	def __init__(self,path):
		f = open(path)
		json_str = f.read()
		self.metadata = json.loads(json_str)
		self._fill_attr()

	def _fill_attr(self):
		self.attribute['clothing_category'] = self.metadata['clothing_category']
		print self.metadata['pageClass']
		
		if self.metadata['clothing_category'] in lower_part_category:
			if self.metadata['clothing_category']=='Shorts':
				self.attribuet['pants_len'] = "short"
			else:
				self.attribute['pants_len'] = "long"

		#default attribute setting
		if self.metadata['clothing_category'] == 'T-Shirts':
			self.attribute['sleeve_len'] = sleeve_opt[1] # short sleeve
		elif self.metadata['clothing_category'] == 'Knitwear':
			self.attribute['sleeve_len'] = sleeve_opt[0] # long sleeve
			if self.is_str_in_field('CARDIGANS',['pageClass']):
				self.attribute['button'] = 'yes'

		elif self.metadata['clothing_category'] == 'Coats and Jackets':
			self.attribute['sleeve_len'] = sleeve_opt[0] # long sleeve
		
		# shirts, not t-shirts
		elif 'Shirts' in self.metadata['clothing_category']:
			self.attribute['button'] = 'yes'
			self.attribute['collar'] = 'shirt-collar'
		elif self.metadata['clothing_category']=='Polos':
			self.attribute['collar'] = 'shirt-collar'
								

		for opt in sleeve_opt:
			if self.is_str_in_field(opt,['product_name','pageClass','detail_care']):
				self.attribute['sleeve_len'] = opt
				break
		for opt in collar_opt:
			if self.is_str_in_field(opt,['product_name','pageClass', 'detail_care']):
				self.attribute['collar'] = opt
				break
		for opt in pattern_opt:
			if self.is_str_in_field(opt,['product_name','pageClass', 'detail_care']):
				self.attribute['pattern'] = opt
				break
		for opt in material_opt:
			if self.is_str_in_field(opt,['product_name','detail_care']):
				self.attribute['material'].append(opt)

		
		

		print self.attribute

	def is_str_in_field(self, keyword, keys):
		keyword = self.normalized_string(keyword)
		for key in keys:
			desc = self.normalized_string(self.metadata[key])
			
			if keyword in desc:
				print keyword, desc
				return True
		return False
	def normalized_string(self,input_str):
		input_str = input_str.lower()
		input_str = re.sub('[-_]',' ', input_str)
		return input_str



print sys.argv[1]
metadata_reader(sys.argv[1])
