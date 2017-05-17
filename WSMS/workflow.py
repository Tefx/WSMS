from bs4 import BeautifulSoup
from math import ceil
import simplejson as json
from os.path import basename
from random import gauss


def generate_task_demand(chars):
    cores = gauss(chars["cores"]["mean"], chars["cores"]["std"]) / 100
    memory = gauss(chars["memory"]["mean"], chars["memory"]["std"])

    if cores <= 0:
        cores = 1

    if memory < 0:
        memory = 0

    return round(cores, 2), ceil(memory)


def read_dax(dax_name):
    xml_file = "resources/workflows/{}.xml".format(dax_name)
    soup = BeautifulSoup(open(xml_file), "html.parser")

    with open("resources/workflows/Characteristics.json") as f:
        app_name = basename(dax_name).split("_")[0]
        chars = json.load(f)[app_name]

    tasks = {}
    for job in soup.find_all("job"):
        tasks[job["id"]] = {
            "runtime": ceil(abs(float(job["runtime"]))),
            "demands": generate_task_demand(chars[job["name"]]),
            "prevs": set(),
            "nexts": set()
        }

    for sec in soup.find_all("child"):
        child = sec["ref"]
        for p in sec.find_all("parent"):
            parent = p["ref"]
            tasks[child]["prevs"].add(parent)
            tasks[parent]["nexts"].add(child)

    return tasks


def read_VM_types(type_file, families=["c4"]):
    with open("resources/{}".format(type_file)) as f:
        data = json.load(f)

    return {k: v for k, v in data.items() if k.split(".")[0] in families}


if __name__ == '__main__':
    print(read_dax("Montage_50"))
