from bs4 import BeautifulSoup
import urllib2
import re
import os
import urlparse
import sys
import json
from Queue import Queue
import re
import json
import glob

fail_list = list()
exclude_set = set()
discovered_set = set()  # url that is in the container or visited


def test():
    if not os.path.exists("images"):
        os.mkdir("images")
    if not os.path.exists("metadata"):
        os.mkdir("metadata")
    url = "http://www.mrporter.com/product/408493"
    try:
        parse_product_info(url)
    except:
        exit(-1)
    exit(0)


def repair_pair_item():
    match_path_list = glob.glob('metadata/*.txt')
    for path in match_path_list:
        metafile = open(path, 'r')
        metadata = json.loads(metafile.read())
        metafile.close()
        for product_code in metadata['pair_item']:
            if not os.path.exists("metadata/" + product_code + ".txt"):
                link_url = "http://www.mrporter.com/product/" + product_code
                print link_url
                parse_product_info(link_url)

    while discovered_set:
        product_code = discovered_set.pop()
        if not os.path.exists("metadata/" + product_code + ".txt"):
            link_url = "http://www.mrporter.com/product/" + product_code
            print link_url
            parse_product_info(link_url)
    exit(0)


def main():

    if not os.path.exists("images"):
        os.mkdir("images")
    if not os.path.exists("metadata"):
        os.mkdir("metadata")
    target_filename = sys.argv[1]
    file = open(target_filename, "r")

    for clothing_link in file:
        url = clothing_link.strip() + "?viewall=on"
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
            if product_code is not None:
                metadata_path = "metadata/" + product_code + ".txt"
                if os.path.exists(metadata_path):
                    print product_code + " exists!!"
                else:
                    print "parsing", link_url
                    try:
                        parse_product_info(link_url)
                    except:
                        fail_list.append(product_code)

    while discovered_set:
        product_code = discovered_set.pop()
        if not os.path.exists("metadata/" + product_code + ".txt"):
            link_url = "http://www.mrporter.com/product/" + product_code
            print link_url
            parse_product_info(link_url)

    print "fail to dowload these... ", fail_list
    file = open("fail_list", "w")
    for product_code in fail_list:
        file.write(product_code)
    file.close()


def samehost(url, site_hostname):

    return (site_hostname == urlparse.urlparse(url).hostname)


def parse_analyticsPageData(product_data, analyticsPageData):
    pattern_template = "%s: \"(.*)\""
    keys = ["department", "pageClass", "title", "category", "subsection1"]
    for key in keys:
        pattern = pattern_template % key

        match = re.search(pattern, analyticsPageData)
        if match is not None:
            product_data[key] = match.group(1)
    # print product_data


def is_product_url(url):
    product_url_pattern = "http://www.\\mrporter.\\com/product/([0-9]*)"
    match = re.match(product_url_pattern, url)
    if match is None:
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
    if product_data['department'] != 'CLOTHING':
        print "NOT CLOTHING, it's " + product_data['department']
        exclude_set.add(product_code)
        return
    product_detail = soup.find(id="product-details")

    product_data["product_name"] = product_detail.h4.string
    try:
        product_data["price_value"] = product_detail.find(
            "span", {"class": "price-value"}).string.strip()
    except:
        product_data["price_value"] = None

    try:
        product_data["clothing_category"] = soup.find(
            id="product-links-list").find_all("a")[1].string.strip()
    except:
        product_data["clothing_category"] = subsection_to_category(
            product_data["subsection1"])

    product_info = soup.find(id="product-more-info")
    product_contents = product_info.find_all(
        "div", {"class": "productContentPiece"})
    product_data['editor_notes'] = product_contents[0].text.strip()

    product_data["detail_care"] = ""
    detail_care_list = product_contents[2].find_all("li")
    for list_item in detail_care_list:
        product_data["detail_care"] += list_item.text + "\n"
    product_data["pair_item"] = [product_data["product_code"]]

    pair_product_links = product_contents[
        0].find_all("a", {"class": "product-item"})
    for pair_product_link in pair_product_links:
        pair_proudct_code = pair_product_link['href'].split('/')[-1]
        product_data["pair_item"].append(pair_proudct_code)
    product_data["pair_item"].sort()
    for item in product_data["pair_item"]:
        if item not in exclude_set and not os.path.exists("metadata/" + item + ".txt"):
            discovered_set.add(item)

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
    if color_div is not None:
        product_data["color"] = color_div.span.text

    print json.dumps(product_data, sort_keys=True, indent=4)
    os.system("wget %s -O images/%s_p.jpg" %
              (product_data["product_img_url"][0], product_data["product_code"]))
    os.system("wget %s -O images/%s_h.jpg" %
              (product_data["product_img_url"][1], product_data["product_code"]))
    os.system("wget %s -O images/%s_ou.jpg" %
              (product_data["pairing_img_url"], product_data["product_code"]))
    file = open("metadata/" + product_data["product_code"] + ".txt", "w")
    file.write(json.dumps(product_data, sort_keys=True, indent=4))
    file.close()


def subsection_to_category(subsection_str):
    subsection_str = subsection_str.lower()
    words = subsection_str.split("_")
    ret = words[0].capitalize()
    for word in words[1:]:
        if word != "and":
            word = word.capitalize()
        ret += " " + word

    return ret


def get_xl_img_url(imgurl):
    pattern = "(.*)_(.*)\.jpg"
    match = re.match(pattern, imgurl)
    if match is None:
        return None
    return match.group(1) + "_xl.jpg"


def get_pairing_img_url(imgurl_list):
    pattern = ".*_ou_.*.jpg"
    for imgurl in imgurl_list:
        match = re.match(pattern, imgurl)
        if match is not None:
            return imgurl
    return None

main()
# http://cache.mrporter.com/images/products/395797/395797_mrp_bk_xxl.jpg
# http://cache.mrporter.com/images/products/395797/395797_mrp_bk_xl.jpg
# http://cache.mrporter.com/images/products/395797/395797_mrp_bk_l.jpg
# http://cache.mrporter.com/images/products/395797/395797_mrp_bk_xs.jpg
# http://cache.mrporter.com/images/products/395797/395797_mrp_bk_s.jpg
