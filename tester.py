import random as rnd
import string
import subprocess
import sys

MAX = 10
N_FILES = 10000
COMMANDS_PER_FILE = 10
STR_LEN_RANGE = range(20, 200)
SEED = 0

_NEXT_LINE = 1


def generate_random_args(two=True):
    x = rnd.choice([rnd.randint(0, MAX), 1])
    y = rnd.randint(0, MAX)
    if two:
        return (min(x, y), max(x, y))
    return (x,)


def change_routine(args):
    global _NEXT_LINE
    _NEXT_LINE = max(args[1] + 1, _NEXT_LINE)

    n = args[1] - args[0] + 1
    s = ""
    for _ in range(n):
        l = rnd.choice(STR_LEN_RANGE)
        letters = string.ascii_lowercase
        s += "".join(rnd.choice(letters) for _ in range(l))
        s += "\n"
    s += ".\n"
    return s


def delete_routine(args):
    n = args[1] - args[0] + 1
    global _NEXT_LINE
    if _NEXT_LINE > args[0]:
        _NEXT_LINE -= min(n, _NEXT_LINE - args[0])
    return None


COMMANDS = [
    ("{},{}c", True, change_routine),
    ("{},{}d", True, delete_routine),
    ("{},{}p", True, None),
    #    ("{}u", False, None),
    #    ("{}r", False, None),
]


if __name__ == "__main__":
    rnd.seed(SEED)
    file_name = "tests_out/edu_tester.{}"
    for i in range(N_FILES):
        _NEXT_LINE = 1
        with open(file_name.format(i), "w") as fp:
            for _ in range(COMMANDS_PER_FILE):
                c, two_args_flag, meta_gen = rnd.choice(COMMANDS)
                args = generate_random_args(two_args_flag)
                c = c.format(*args)
                metadata = None
                fp.write(c)
                fp.write("\n")

                if meta_gen and args[0] > 0 and args[0] <= _NEXT_LINE:

                    # Change or delete
                    metadata = meta_gen(args)  # hase side effects

                    if metadata:
                        # Change
                        fp.write(metadata)

            fp.write("q\n")
    for j in range(N_FILES):
        _NEXT_LINE = 1
        i = file_name.format(j)
        fpi = open(i, "r")
        p = sys.argv[1]
        o = file_name.format(j) + ".out"
        e = file_name.format(j) + ".err"
        fpo = open(o, "w")
        fpe = open(o, "w")
        print(f"Executing on {i}")
        process = subprocess.Popen(
            [
                "valgrind",
                "--leak-check=full",
                "--show-leak-kinds=all",
                "--track-origins=yes",
                "--verbose",
                p,
            ],
            stdin=fpi,
            stdout=fpo,
            stderr=fpe,
        )
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            print("Killed")
            process.kill()
