#pragma once

#include <ncurses.h>
#include <stdlib.h>

typedef struct Clanok {
    int x, y;
    struct Clanok* nasledujuciClanok;
} Clanok;

Clanok* vytvorClanok(int x, int y);
void freeHadik(Clanok* hlava);
void pridajClanok(Clanok* hlava);
void vykresliHadika(Clanok* hlava);
int kolizia(Clanok* hadik, int noveX, int noveY);
