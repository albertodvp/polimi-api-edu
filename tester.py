import random as rnd

from edu_utils import create_custom_tests
from execs import execute_debug, execute_public
from gens import gen_custom_files, gen_public_files

SEED = 42


CREATE = True
PUBLIC_TESTS = False
EXECUTE_DEBUG = True

if __name__ == "__main__":
    rnd.seed(SEED)

    # TEST CREATION
    create_custom_tests() if CREATE else None

    # PUBLIC TEST
    execute_public() if PUBLIC_TESTS else None

    # COMPLETE DEBUG TEST
    gen1 = gen_public_files() if EXECUTE_DEBUG else None
    gen2 = gen_custom_files() if EXECUTE_DEBUG else None
    execute_debug(gen1, gen2) if EXECUTE_DEBUG else None
