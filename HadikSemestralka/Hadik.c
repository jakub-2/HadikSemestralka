#include "Hadik.h"

Clanok* vytvorClanok(int x, int y) {
    Clanok* clanok = (Clanok*)malloc(sizeof(Clanok));
    clanok->x = x;
    clanok->y = y;
    clanok->nasledujuciClanok = NULL;
    return clanok;
}

void freeHadik(Clanok* hlava) {
    while (hlava) {
        Clanok* temp = hlava;
        hlava = hlava->nasledujuciClanok;
        free(temp);
    }
}

void pridajClanok(Clanok* hlava) {
    Clanok* aktualnyClanok = hlava;

    while (aktualnyClanok->nasledujuciClanok) {
        aktualnyClanok = aktualnyClanok->nasledujuciClanok;
    }
    aktualnyClanok->nasledujuciClanok = vytvorClanok(aktualnyClanok->x, aktualnyClanok->y);
}

void vykresliHadika(Clanok* hlava) {
    Clanok* akutalnyClanok = hlava;

    while (akutalnyClanok) {
        mvprintw(akutalnyClanok->y, akutalnyClanok->x, "O");
        akutalnyClanok = akutalnyClanok->nasledujuciClanok;
    }
}

int kolizia(Clanok* hadik, int noveX, int noveY) {
    Clanok* aktualnyClanok = hadik;

    while (aktualnyClanok->nasledujuciClanok) {
        if (aktualnyClanok->x == noveX && aktualnyClanok->y == noveY) {
            return 1;
        }
        aktualnyClanok = aktualnyClanok->nasledujuciClanok;
    }

    return 0;
}