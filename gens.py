import os
import re
from typing import Iterator, Tuple

from edu_utils import N_FILES, POSTFIXES, TEST_FILE_NAME

In = str
Out = str
Err = str
GenFiles = Iterator[Tuple[In, Out, Err]]

PUBLIC_TESTS = [
    "rollercoaster",
    "rollingback",
    "timeforachange",
    "write_only",
    "bulkreads",
]


def gen_public_files() -> GenFiles:
    for public_test in PUBLIC_TESTS:
        input_pattern = r".*input.txt$"
        files = os.listdir(f"./{public_test}")
        matches = [m.group() for f in files if (m := re.match(input_pattern, f))]
        for i in matches:
            if all([i.find(e) == -1 for e in ["#", "~"]]):  # emacs stuff
                i = f"{public_test}/{i}"
                yield i, f"{i}.out", f"{i}.err"


def gen_custom_files() -> GenFiles:
    for j in range(N_FILES):
        i, o, e = [TEST_FILE_NAME.format(j, post) for post in POSTFIXES]
        yield i, o, e
