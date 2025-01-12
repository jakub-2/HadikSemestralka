#include "Menu.h"
#include "ClientBasic.h"

// main file for server
int main() {
    char* options[] = { "Connect to Game", "Exit" };
    int vyber;
    while (1)
    {
        vyber = menu(options, 3);
        if (vyber == 0)
        {
            start();
        }
        else
        {
            break;
        }
    }
    printf("Bye\n");
    return 0;
}
