import re
import sys


def extract_alloc_data(path: str):
    # TODO
    command = 'grep -P "(\d+) allocs, (\d+) frees" path'
    raise NotImplementedError()


if __name__ == "__main__":
    # Valgrind output folder
    path = sys.argv[1]
    #    out_fp = extract_alloc_data(path)
    out_fp = open(path)
    for line in out_fp:
        pattern = r"(\d+) allocs, (\d+) frees"
        match = re.search(pattern, line)
        alloc, free = match.groups()
        if alloc != free:
            print("LEAK:", line[:-1])
