#include "Menu.h"

int menu(char** options, int numOfOptions)
 {
     initscr();              // Initialize the curses mode
     keypad(stdscr, TRUE);   // Enable keyboard input for the window
     noecho();               // Disable echoing of characters to the screen
     curs_set(0);            // Hide the cursor

     int selected = 0;       // Keeps track of the currently selected option
     bool selecting = true;  // True while waiting for the user to press Enter
     int ch;                 // Character input

     while (selecting) {
         // Clear the screen and display the menu
         clear();
         for (int i = 0; i < numOfOptions; i++) {
             if (i == selected) {
                 attron(A_REVERSE);  // Highlight the selected option
                 mvprintw(i, 0, "%s", options[i]);
                 attroff(A_REVERSE); // Turn off highlighting
             }
             else {
                 mvprintw(i, 0, "%s", options[i]);
             }
         }
         refresh();  // Refresh the screen to show updates

         ch = getch();  // Get user input
         switch (ch) {
         case KEY_UP:
             if (selected > 0) {
                 selected--;  // Move selection up
             }
             break;
         case KEY_DOWN:
             if (selected < numOfOptions - 1) {
                 selected++;  // Move selection down
             }
             break;
         case '\n':  // Enter key
             selecting = false;  // Exit the selection loop
             break;
         default:
             break;
         }
     }
     clear();
     endwin();  // End curses mode
     return selected;
}