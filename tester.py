import random as rnd

from edu_utils import create_custom_tests
from execs import execute_debug, execute_public
from gens import gen_custom_files, gen_public_files

SEED = 0


if __name__ == "__main__":
    rnd.seed(SEED)

    # TEST CREATION
    create_custom_tests() if False else None

    # PUBLIC TEST
    execute_public() if True else None

    # COMPLETE DEBUG TEST
    gen1 = gen_public_files() if False else None
    gen2 = gen_custom_files() if False else None
    execute_debug(gen1, gen2) if False else None
