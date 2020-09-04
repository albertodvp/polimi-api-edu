import random as rnd

from edu_utils import create_custom_tests
from execs import execute, execute_debug
from gens import gen_custom_files, gen_public_files

SEED = 0


CREATE = True
EXECUTE = True
EXECUTE_DEBUG = False

if __name__ == "__main__":
    rnd.seed(SEED)

    # TEST CREATION
    create_custom_tests() if CREATE else None

    gen1 = gen_public_files()
    gen2 = gen_custom_files()

    # EXECUTE - check for with diffs if gt (aka *output*) is present
    execute(gen1, gen2) if EXECUTE else None

    # COMPLETE DEBUG TEST - "debugs" throwing valgrind, check for leaks with find_leaks.py
    execute_debug(gen1, gen2) if EXECUTE_DEBUG else None
