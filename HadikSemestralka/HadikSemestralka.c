//#include "Menu.h"
#include "Server.h"
#include "ClientBasic.h"
#include "Client.h"

#define DELAY_SIRKA 100000
#define DELAY_VYSKA 150000

int main() {
    //char* pole[] = { "Ahoj1", "Ahoj2" , "Ahoj3" , "Ahoj4" , "Ahoj5" };

    //int velkost = 2;
    //char* poleSvetov[] = { "Svet bez prekazok", "Svet s prekazkami"};

    //int vybranySvet = menu(poleSvetov, velkost);

    //test();

    char* options[] = { "New Game", "Connect to Game", "Exit" };
    int vyber;
    while (1)
    {
        //TODO pripojenie z pozastavenia
        
        vyber = menu(options, 3);
        if (vyber == 0)
        {
            create_session();
        } else if (vyber == 1)
        {
            start();
        }
        else
        {
	        break;
        }
    }

    return 0;
} 
