#include "Game.h"
#include "memory.h"

static solverdata* solve(bool debug);
static void saveSolverData(solverdata* sd, const char* fileName, bool debug);
static void testCustomInput();

int main(int argc, char** argv) {
//    testCustomInput();

    if(argc != 2) {
        printf("Usage: %s <filename>", argv[0]);
        return 1;
    }
    solverdata* primitives = solve(true);
    if (primitives == NULL) {
        return 1;
    }
//    saveSolverData(primitives, argv[1], true);

    playerdata* pd = initializeplayerdata(argv[1]);
    if (verifyPlayerData(primitives, pd)) {
        printf("passed\n");
    }

    freesolver(primitives);
    freeplayer(pd);
    return 0;
}

static solverdata* solve(bool debug) {
    initialize_constants();
    // game pos = getStartingPositions();
    solverdata* primitives = initializesolverdata(hashLength());
    game* fringe = calloc(sizeof(game), getMaxMoves() * getMaxDepth());
    if (primitives == NULL || fringe == NULL) {
        printf("Memory allocation error\n");
        free(fringe);
        return NULL;
    }
    int index = 1;
    fringe[0] = getStartingPositions();
    game g;
    game newg;
    gamehash h;
    unsigned char primitive;
    unsigned char minprimitive;
    unsigned char moves[getMaxMoves()];
    int movecount;
    int i;
    int oldindex;
    gamehash minindex = maxHash();
    gamehash maxindex = 0;
    int* positionsfound = calloc(sizeof(int), 256);
    printf("Beginning main loop\n");
    fflush(stdout);
    while (index) {
        minprimitive = 255;
        oldindex = index;
        g = fringe[index-1];
        h = getHash(g);
        if (solverread(primitives, h) == 0) {
            movecount = generateMoves((unsigned char*)&moves, g);
            for (i = 0; i < movecount; i++) {
                newg = doMove(g, moves[i]);
                h = getHash(newg);
                primitive = solverread(primitives, h);
                if (!primitive) {
                    primitive = isPrimitive(newg, moves[i]);
                    if (primitive != (unsigned char) NOT_PRIMITIVE) {
                        solverinsert(primitives, h, primitive);
                        if (h < minindex) {
                            minindex = h;
                        }
                        if (h > maxindex) {
                            maxindex = h;
                        }
                        //printf("Position 0x%08x determined primitive\n", newg);
                        positionsfound[primitive]++;
                        minprimitive = minprimitive <= primitive ? minprimitive : primitive;
                    } else {
                        fringe[index] = newg;
                        index++;
                    }
                } else {
                    minprimitive = minprimitive <= primitive ? minprimitive : primitive;
                }
            }
            if (index == oldindex) {
                if (minprimitive & 128) {
                    if (minprimitive & 64) {
                        minprimitive = 257 - minprimitive;
                    } else {
                        minprimitive = minprimitive + 1;
                    }
                } else {
                    minprimitive = 255 - minprimitive;
                }
                h = getHash(g);
                solverinsert(primitives, h, minprimitive);
                if (h < minindex) {
                    minindex = h;
                }
                if (h > maxindex) {
                    maxindex = h;
                }
                //printf("Position 0x%08x determined fully\n", g);
                positionsfound[minprimitive]++;
                index--;
            }
        } else {
            index--;
        }
    }
    if (debug) {
        printf("Done computing, listing statistics\n");
        int totalpositioncount = 0;
        for(int i = 1; i < 64; i++) {
            if(positionsfound[i]) printf("Loss in %d: %d\n", i - 1, positionsfound[i]);
            totalpositioncount+=positionsfound[i];
        }
        for(int i = 128; i < 192; i++) {
            if(positionsfound[i]) printf("Tie in %d: %d\n", i - 128, positionsfound[i]);
            totalpositioncount+=positionsfound[i];
        }
        for(int i = 254; i >= 192; i--) {
            if(positionsfound[i]) printf("Win in %d: %d\n", 255 - i, positionsfound[i]);
            totalpositioncount += positionsfound[i];
        }
        printf("In total, %d positions found\n", totalpositioncount);
        printf("%d primitive positions found\n", positionsfound[LOSS] + positionsfound[TIE]);
        printf("Starting position has value %d\n", solverread(primitives, getStartingPositions()));
        printf("%llx, %llx\n", minindex, maxindex);
        fflush(stdout);
    }
    free(fringe);
    free(positionsfound);
    return primitives;
}

static void saveSolverData(solverdata* sd, const char *fileName, bool debug) {
    if (debug) {
        printf("Starting output to file %s\n", fileName);
        fflush(stdout);
    }
    FILE* file = fopen(fileName, "wb");
    solversave(sd, file);
    if (debug) {
        printf("Done outputting\n");
        fflush(stdout);
    }
}

static void testCustomInput() {
    playerdata* pd = initializeplayerdata("out.txt");
    for (uint64_t i = 0; i < 16; ++i) {
        unsigned char c = playerread(pd, i);
        printf("playerread(%llx): %x\n", i, c);
    }
    freeplayer(pd);
}