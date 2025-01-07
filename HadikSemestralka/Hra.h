#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#include "Hadik.h"

void vygenerujOkraje(int sirka, int vyska);
void vygenerujPrekazku(int* prekazkaX, int* prekazkaY, Clanok* hlava, int sirka, int vyska);
void vygenerujOvocie(int* ovocieX, int* ovocieY, Clanok* hlava, int sirka, int vyska);
void vypisSkore(int body);
void hra(int typSvetu);