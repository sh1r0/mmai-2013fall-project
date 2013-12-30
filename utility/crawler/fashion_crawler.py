from bs4 import BeautifulSoup
import urllib2
import urllib
import re
import os
import urlparse
import sys
import json
import urlparse
from Queue import Queue
import re
import json

fail_list = list()
visited_set = set()
discovered_set = set() #url that is in the container or visited
def test():
	if not os.path.exists("images"):
		os.mkdir("images")
	if not os.path.exists("metadata"):
		os.mkdir("metadata")
	url= "http://www.mrporter.com/product/361615"
	try:
		parse_product_info(url)
	except:
		exit(-1)
	exit(0)
def main():
	if not os.path.exists("images"):
		os.mkdir("images")
	if not os.path.exists("metadata"):
		os.mkdir("metadata")
	target_filename = sys.argv[1]
	file = open(target_filename,"r")
	
	for clothing_link in file:	
		url = clothing_link.strip()+"?viewall=on" 
		print url
		try:
			htmlData = urllib2.urlopen(url).read()
		except:
			continue
		print "reading ", url
		soup = BeautifulSoup(htmlData)
		links = soup.find_all('a')
		for link in links:
			if not link.has_attr('href'):
				continue
			link_url = urlparse.urljoin(url, link['href'])
			product_code = is_product_url(link_url)
			if product_code != None:
				print "parsing", link_url
				try:
					parse_product_info(link_url)
				except:
					fail_list.append(product_code)


	
	print "fail to dowload these... ", fail_list
	file = open("fail_list","w")
	for product_code in fail_list:
		file.write(product_code)
	file.close()
def samehost(url, site_hostname):
	
	return (site_hostname==urlparse.urlparse(url).hostname)

def parse_analyticsPageData(product_data,analyticsPageData):
	pattern_template = "%s: \"(.*)\""
	keys = ["department","pageClass","title","category"]
	for key in keys:
		pattern = pattern_template % key
		
		match = re.search(pattern,analyticsPageData)
		if match != None:
			product_data[key] = match.group(1)
	print product_data

def is_product_url(url):
	product_url_pattern = "http://www.\\mrporter.\\com/product/([0-9]*)"
	match = re.match(product_url_pattern, url)
	if match == None:
		return None
	return match.group(1)

def parse_product_info(url):
	
	product_code = is_product_url(url)
	htmlData = urllib2.urlopen(url).read()
	soup = BeautifulSoup(htmlData)
	product_data = dict()
	product_data["product_code"] = product_code
	analyticsPageData = soup.find("script").text
	parse_analyticsPageData(product_data, analyticsPageData)
	product_detail = soup.find(id="product-details")
	
	product_data["product_name"] = product_detail.h4.string
	product_data["price_value"] = product_detail.find("span",{"class":"price-value"}).string.strip()


	product_info = soup.find(id="product-more-info")
	product_contents = product_info.find_all("div",{"class":"productContentPiece"})
	product_data['editor_notes'] = product_contents[0].text.strip()
	
	product_data["detail_care"] = ""
	detail_care_list= product_contents[2].find_all("li")
	for list_item in detail_care_list:
		product_data["detail_care"] += list_item.text+"\n"
	product_data["pair_item"] = [product_data["product_code"]]
	
	pair_product_links = product_contents[0].find_all("a",{"class":"product-item"})
	for pair_product_link in pair_product_links:
		pair_proudct_code = pair_product_link['href'].split('/')[-1]
		product_data["pair_item"].append(pair_proudct_code)
	product_data["pair_item"].sort()


	product_carousel = soup.find(id="product-carousel")
	list_item = product_carousel.find_all("li")
	imgurl_list = []
	for li in list_item:
		product_img_url = get_xl_img_url(li.img['src'])
		url = urlparse.urljoin(url, product_img_url)
		imgurl_list.append(url)
	product_data["product_img_url"] = imgurl_list
	
	pairing_img_url = get_pairing_img_url(imgurl_list)
	product_data["pairing_img_url"] = urlparse.urljoin(url, pairing_img_url)


	color_div = soup.find(id="colour-text")
	if color_div!=None:
		product_data["color"] = color_div.span.text
	product_data["clothing_category"] = soup.find(id="product-links-list").find_all("a")[1].string.strip()
	print json.dumps(product_data, sort_keys=True, indent=4)
	os.system("wget %s -O images/%s_p.jpg"%(product_data["product_img_url"][0],product_data["product_code"]))
	os.system("wget %s -O images/%s_h.jpg"%(product_data["product_img_url"][1],product_data["product_code"]))
	os.system("wget %s -O images/%s_ou.jpg"%(product_data["pairing_img_url"],product_data["product_code"]))
	file = open("metadata/"+product_data["product_code"]+".txt","w")
	file.write(json.dumps(product_data, sort_keys=True, indent=4))
	file.close()

def get_xl_img_url(imgurl):
	pattern = "(.*)_(.*)\.jpg"
	match  = re.match(pattern, imgurl)
	if match == None:
		return None
	return match.group(1)+"_xl.jpg"

def get_pairing_img_url(imgurl_list):
	pattern = ".*_ou_.*.jpg"
	for imgurl in imgurl_list:
		match  = re.match(pattern, imgurl)
		if match != None:
			return imgurl
	return None

main()
#http://cache.mrporter.com/images/products/395797/395797_mrp_bk_xxl.jpg
#http://cache.mrporter.com/images/products/395797/395797_mrp_bk_xl.jpg
#http://cache.mrporter.com/images/products/395797/395797_mrp_bk_l.jpg
#http://cache.mrporter.com/images/products/395797/395797_mrp_bk_xs.jpg
#http://cache.mrporter.com/images/products/395797/395797_mrp_bk_s.jpg