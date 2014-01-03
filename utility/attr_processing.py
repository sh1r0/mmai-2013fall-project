import json
import glob
from metadata_reader import metadata_reader
upper_data_path = "../data/upper_dataset.json"
lower_data_path = "../data/lower_dataset.json"
meta_data_path = "../data/classify_dataset/metadata"


def load_dataset(path):
    file = open(path, "r")
    json_str = file.read()
    return json.loads(json_str)


def output_svm_format(attr_field, feature_field, data_item):
    assert attr_field in data_item
    assert feature_field in data_item
    row = data_item[attr_field]
    if row is not None:
        for idx, value in enumerate(upper_item['gabor']):
            append_str = " %d:%s" % (idx+1, value[0])
            row += append_str
        print row


if __name__ == "__main__":
    upper_dataset = load_dataset(upper_data_path)
    lower_dataset = load_dataset(lower_data_path)

    count = 0
    for upper_item in upper_dataset:
        #print upper_item['product_code']
        metadata = metadata_reader(upper_item['product_code'], meta_data_path)
        attr = metadata.get_attr()
        upper_item.update(attr)
        #print upper_item
        #output_svm_format('pattern', 'gabor', upper_item)
    file = open(upper_data_path+'.out', 'w')
    json_str = json.dumps(upper_dataset)
    file.write(json_str)
