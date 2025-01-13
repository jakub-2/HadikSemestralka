#include "Client.h"

// main file for server
int main() {
    char* options[] = { "New Game", "Exit" };
    int vyber;
    while (1)
    {
        vyber = menu(options, 2);
        if (vyber == 0)
        {
            create_session();
        }
        else
        {
            break;
        }
    }
    printf("Bye\n");
    return 0;
}
