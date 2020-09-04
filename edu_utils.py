import random as rnd
import string

MAX = 10
N_FILES = 10
COMMANDS_PER_FILE = 20
STR_LEN_RANGE = range(20, 1024)

TEST_FILE_NAME = "tests_out/custom_input.{}.{}"
POSTFIXES = ["in", "out", "err"]


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
    if args[0] > hist[i]:
        hist.append(hist[i])
        return next_i, None

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


def create_custom_tests():
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
                if meta_gen and args[0] > 0:
                    # Change or delete
                    next_line, metadata = meta_gen(
                        args, hist=next_line_history, i=next_line
                    )

                    if metadata:
                        # Change
                        fp.write(metadata)
                # print("Next line index", next_line)
                # print("Next hist", next_line_history)
                # print("Command", c)
                # print("\n\n")

            fp.write("q\n")
