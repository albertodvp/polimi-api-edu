import os
import random as rnd
import re
import string
import subprocess
import sys
import time

MAX = 10
N_FILES = 10000
COMMANDS_PER_FILE = 10
STR_LEN_RANGE = range(20, 200)
SEED = 0

TEST_FILE_NAME = "tests_out/edu_tester.{}.{}"
POSTFIXES = ["in", "out", "err"]

PUBLIC_TESTS = [
    "rollercoaster",
    "rollingback",
    "timeforachange",
    "write_only",
    "bulkreads",
]


_NEXT_LINE_HISTORY = [1]
_NEXT_LINE = 0


def generate_random_args():
    x = rnd.choice([rnd.randint(0, MAX), 1])
    y = rnd.randint(0, MAX)
    return (min(x, y), max(x, y))


def change_routine(args, hist, i):
    next_i = i + 1
    if len(hist) != next_i:
        # Dropping history
        del hist[next_i:]
    n = max(args[1] + 1, hist[i])
    hist.append(n)
    n = args[1] - args[0] + 1
    s = ""
    for _ in range(n):
        l = rnd.choice(STR_LEN_RANGE)
        letters = string.ascii_lowercase
        s += "".join(rnd.choice(letters) for _ in range(l))
        s += "\n"
    s += ".\n"
    return next_i, s


def delete_routine(args, hist, i):
    next_i = i + 1
    if len(hist) != next_i:
        # Dropping history
        del hist[next_i:]

    delta = args[1] - args[0] + 1
    n = hist[i]
    if hist[i] > args[0]:
        n -= min(delta, hist[i] - args[0])

    hist.append(n)
    return next_i, None


def undo_routine(args, hist, i):
    steps, _ = args
    return max(0, i - steps), None


def redo_routine(args, hist, i):
    steps, _ = args
    return min(len(hist) - 1, i + steps), None


COMMANDS = [
    ("{},{}c", change_routine),
    ("{},{}d", delete_routine),
    ("{},{}p", None),
    ("{}u", undo_routine),
    ("{}r", redo_routine),
]


def create_input_test_files():
    for i in range(N_FILES):
        next_line = 0
        next_line_history = [1]
        with open(TEST_FILE_NAME.format(i, POSTFIXES[0]), "w") as fp:
            for _ in range(COMMANDS_PER_FILE):
                c, meta_gen = rnd.choice(COMMANDS)
                args = generate_random_args()
                c = c.format(*args)
                metadata = None
                fp.write(c)
                fp.write("\n")
                # print("Next line", next_line)
                # print("Next hist", next_line_history)
                # print("Command", c)
                # print("\n\n")
                if meta_gen and args[0] > 0 and args[0] <= next_line_history[next_line]:
                    # Change or delete
                    next_line, metadata = meta_gen(
                        args, hist=next_line_history, i=next_line
                    )

                    if metadata:
                        # Change
                        fp.write(metadata)

            fp.write("q\n")


def test_public_tests():
    #    exit(0)
    for public_test in PUBLIC_TESTS:
        input_pattern = r".*input.*"
        files = os.listdir(f"./{public_test}")
        matches = [m.group() for f in files if (m := re.match(input_pattern, f))]

        for i in matches:
            if any([i.find(e) != -1 for e in ["#", "~"]]):
                # emacs stuff
                continue
            o = f"_{i}.out"
            i = f"{public_test}/{i}"
            odiff = f"{o}.diff"
            ifp = open(i)
            ofp = open(o, "w")
            p = sys.argv[1]
            #            print("Executing on", i)
            process = subprocess.Popen([p], stdin=ifp, stdout=ofp)
            ofp = open(odiff, "w")
            #           print("Checking diff between:", i, o, "out:", odiff)
            process = subprocess.Popen(
                ["diff", o, i.replace("input", "output")], stdout=ofp
            )
            ofp.close()
            time.sleep(0.5)

            stat = os.stat(odiff)
            if stat.st_size > 0:
                print("DIFF NOT EMPTY", "diff", odiff)


def execute_on_test_files():
    for j in range(N_FILES):
        i, o, e = [TEST_FILE_NAME.format(j, post) for post in POSTFIXES]
        fpi = open(i, "r")
        fpo = open(o, "w")
        fpe = open(e, "w")

        print(f"Executing on {i}")
        p = sys.argv[1]
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


if __name__ == "__main__":
    rnd.seed(SEED)
    #   create_input_test_files()
    #   execute_on_test_files()
    test_public_tests()
