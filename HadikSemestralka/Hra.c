#include "Hra.h"

#include <time.h>
#include <unistd.h>

#define DELAY_SIRKA 100000
#define DELAY_VYSKA 150000

void vygenerujOkraje(int sirka, int vyska) {
    for (int riadok = 0; riadok < vyska; riadok++) {
        for (int stlpec = 0; stlpec < sirka; stlpec++) {
            if (riadok == 0 || riadok == vyska - 1 || stlpec == 0 || stlpec == sirka - 1) {
                mvprintw(stlpec, riadok, "#");
            }
        }
    }
}

void vygenerujPrekazku(int* prekazkaX, int* prekazkaY, Clanok* hlava, int sirka, int vyska)
{
    int jeHadik;
    do {
        jeHadik = 1;
        *prekazkaX = rand() % (sirka - 2) + 1;
        *prekazkaY = rand() % (vyska - 2) + 1;
        Clanok* aktualnyClanok = hlava;
        while (aktualnyClanok) {
            if (aktualnyClanok->x == *prekazkaX && aktualnyClanok->y == *prekazkaY) {
                jeHadik = 0;
                break;
            }
            aktualnyClanok = aktualnyClanok->nasledujuciClanok;
        }
    } while (!jeHadik);
}

void vygenerujOvocie(int* ovocieX, int* ovocieY, Clanok* hlava, int sirka, int vyska) {
    int jeHadik;
    do {
        jeHadik = 1;
        *ovocieX = rand() % (sirka - 2) + 1;
        *ovocieY = rand() % (vyska - 2) + 1;
        Clanok* aktualnyClanok = hlava;
        while (aktualnyClanok) {
            if (aktualnyClanok->x == *ovocieX && aktualnyClanok->y == *ovocieY) {
                jeHadik = 0;
                break;
            }
            aktualnyClanok = aktualnyClanok->nasledujuciClanok;
        }
    } while (!jeHadik);
}

void vypisSkore(int body)
{
    printf("Game Over! Skore:\n Hrac 1 - %d\n", body);
}

void hra(int typSvetu) // 0 - bez prekazok | 1 - s prekazkami
{
    int sirka = 30;
    int vyska = 30;

    int input, smer = KEY_RIGHT;
    int ovocieX, ovocieY, body = 0;
    Clanok* hadik = vytvorClanok(sirka / 2, vyska / 2);
    pridajClanok(hadik);
    pridajClanok(hadik);

    srand(time(0));
    vygenerujOvocie(&ovocieX, &ovocieY, hadik, sirka, vyska);

    int pocetPrekazok = 10;
    int prekazkaX[pocetPrekazok], prekazkaY[pocetPrekazok];

    //int polickaTotal = sirka * vyska;
    if (typSvetu == 1)
    {
        for (int i = 0; i < pocetPrekazok; ++i)
        {
            vygenerujPrekazku(&prekazkaX[i], &prekazkaY[i], hadik, sirka, vyska);
            //mvprintw(prekazkaX, prekazkaY, "#");
        }
    }

    initscr(); // Inicializacia ncurses
    keypad(stdscr, TRUE); // Povolenie vstupu z klávesnice pre okno | ak TRUE klavesy sa vratia ako kody (KEY_UP, ...)
    noecho(); // Zakáza opakovanie znakov na obrazovke
    curs_set(FALSE); // Skryje kurzor na terminal
    timeout(0); // Program moze pokracovat bez cakania na user input

    int delay = DELAY_SIRKA;

    while (1) {
        clear(); // Resetne kurzor, vycisti terminal

        vygenerujOkraje(sirka, vyska);
        mvprintw(ovocieY, ovocieX, "X");
        vykresliHadika(hadik);

        mvprintw(vyska, 0, "Skore: %d", body);

        for (int i = 0; i < pocetPrekazok; ++i)
        {
            mvprintw(prekazkaX[i], prekazkaY[i], "#");
        }

    	refresh(); // Update na terminaly aby boli viditelne zmeny
        // Input
        input = getch(); // input ako char
        if (input != ERR) {
            if ((input == KEY_UP && smer != KEY_DOWN) || (input == KEY_DOWN && smer != KEY_UP) ||
                (input == KEY_LEFT && smer != KEY_RIGHT) || (input == KEY_RIGHT && smer != KEY_LEFT)) {
                smer = input;
            }
        }

        int noveX = hadik->x;
        int noveY = hadik->y;

        if (smer == KEY_UP) {
            noveY--;
            delay = DELAY_VYSKA;
        }
        else if (smer == KEY_DOWN) {
            noveY++;
            delay = DELAY_VYSKA;
        }
        else if (smer == KEY_LEFT) {
            noveX--;
            delay = DELAY_SIRKA;
        }
        else if (smer == KEY_RIGHT) {
            noveX++;
            delay = DELAY_SIRKA;
        }

        // kolizia s okrajmi
        if (typSvetu == 0)
        {
            if (noveX <= 0)
            {
                noveX = sirka - 2;
            }
            else if (noveX >= sirka - 1)
            {
                noveX = 1;
            }
            else if (noveY <= 0)
            {
                noveY = vyska - 2;
            }
            else if (noveY >= vyska - 1)
            {
                noveY = 1;
            }
        } else if (typSvetu == 1)
        {
            if (noveX <= 0 || noveX >= sirka - 1 || noveY <= 0 || noveY >= vyska - 1) {
                break;
            }
        }

        // Kolizia sam so sebou
        if (kolizia(hadik, noveX, noveY)) {
            freeHadik(hadik);
            endwin();
            vypisSkore(body);
            //return 0;
        }

        // Posun
        Clanok* novyClanok = vytvorClanok(noveX, noveY);
        novyClanok->nasledujuciClanok = hadik;
        hadik = novyClanok;

        // Kolizia s ovocim
        if (noveX == ovocieX && noveY == ovocieY) {
            body++;
            vygenerujOvocie(&ovocieX, &ovocieY, hadik, sirka, vyska);
        }
        else {
            Clanok* tmp = hadik;
            while (tmp->nasledujuciClanok->nasledujuciClanok) {
                tmp = tmp->nasledujuciClanok;
            }
            free(tmp->nasledujuciClanok);
            tmp->nasledujuciClanok = NULL;
        }

        usleep(delay);
    }

    freeHadik(hadik);
    endwin(); // Ukoncuje ncurses
    vypisSkore(body);
    //return 0;
}