import re
import sys

# grep -P "(\d+) allocs, (\d+) frees" * > find_leak_in
if __name__ == "__main__":
    with open(sys.argv[1]) as fp:
        for l in fp:
            pattern = r"(\d+) allocs, (\d+) frees"
            match = re.search(pattern, l)
            alloc, free = match.groups()
            if alloc != free:
                print("LEAK:", l[:-1])
