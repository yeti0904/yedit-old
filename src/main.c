#include <ncurses.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "int.h"
#include "file.h"
#define ctrl(x)    ((x) & 0x1f)
// lenny from programmer's den made the macro above ^

void refreshAll(WINDOW * windows[], int windowc) {
	refresh();
	for (int i = 0; i<windowc; ++i) {
		wrefresh(windows[i]);
	}
}

void clearAll(WINDOW * windows[], int windowc) {
	for (int i = 0; i<windowc; ++i) {
		wrefresh(windows[i]);
	}
}

int main(int argc, char* argv[]) {
	// time variables
	time_t rawtime;
	struct tm * timeinfo;
	// make/read file
	char* file;
	char filename[64];
	ui32 filesize;
	if ((argc > 1) && (fexists(argv[1]))) {
		strcpy(filename, argv[1]);
		file = readfile(argv[1]);
		filesize = getfilesize(argv[1]);
	}
	else if (!fexists(argv[1]) && (argc > 1)) {
		strcpy(filename, argv[1]);
		file = (char*) malloc(1);
		filesize = 1;
		strcpy(file, "");
	}
	else {
		strcpy(filename, "Untitled");
		file = (char*) malloc(1);
		filesize = 1;
		strcpy(file, "");
	}
	// init ncurses
	initscr();            // Start curses mode
	raw();                // 
	noecho();             //
	start_color();        //
	use_default_colors(); //
	//curs_set(0);          // disable cursor (im gonna fake the cursor)
	// variables
	int scrX, scrY;               // terminal width and height
	getmaxyx(stdscr, scrY, scrX); // get terminal width and height
	bool run = true;              // bool for run loop
	int input;                    // input in editor
	unsigned int scrollY = 0;     // scroll Y (offset in lines)
	int col, ln;                  // cursor position
	int shortfilesize;            // shortened file size (measured in different things)
	char sfsmeasure[3];           // shortened file size measure
	char alert[64] = "";          // alert content
	ui8 alertclock = 0;           //
	bool inString;                // if in string (when printing)
	bool inDirective;             // used for C syntax highlighting
	bool inComment;               // used in syntax highlighting
	init_pair(1, COLOR_BLACK, COLOR_WHITE);
	init_pair(2, COLOR_WHITE, COLOR_BLUE);
	init_pair(3, COLOR_BLACK, COLOR_GREEN);
	// syntax highlighting
	init_pair(4, COLOR_CYAN, COLOR_BLUE); // integer / numbers
	init_pair(5, COLOR_GREEN, COLOR_BLUE); // string
	init_pair(6, COLOR_MAGENTA, COLOR_BLUE); // directive (C)
	init_pair(7, COLOR_YELLOW, COLOR_BLUE); // comment
	// windows
	WINDOW* titlebar = newwin(1, scrX, 0, 0);
	WINDOW* editor   = newwin(scrY - 1, scrX, 1, 0);
	WINDOW* windows[]  = {titlebar, editor};
	keypad(stdscr, true);
	// setup colour
	wbkgd(titlebar, COLOR_PAIR(1));
	wbkgd(editor, COLOR_PAIR(2));
	// run loop
	while (run) {
		col = 0;
		ln = 0;
		if (alertclock != 0) {
			-- alertclock;
		}
		if (alertclock = 0) {
			strcpy(alert, "");
		}
		wclear(titlebar);
		wclear(editor);
		wprintw(titlebar, "Yeti's Editor b5 || ");
		wattron(titlebar, COLOR_PAIR(3));
		wprintw(titlebar,  "%s", alert);
		wattroff(titlebar, COLOR_PAIR(3));
		// print on editor
		wprintw(editor, "\n ");
		for (int i = 0; i<filesize; ++i) {
			if (!(file[i] == 10)) {
				++ col;
				if (ln >= scrollY) {
					if ((col == 0) && inComment) {
						inComment = false;
					}
					if ((file[i] == '\"') && !inDirective) inString = !inString;
					else if ((file[i] == '#') && (col == 0)) inDirective = !inDirective;
					else if ((file[i] == '/') && (file[i+1] == '/') && !inDirective) inComment = !inComment;
					if (inString || (file[i] == '"')) {
						wattron(editor, COLOR_PAIR(5));
					}
					else if ((file[i] >= '0') && (file[i] <= '9')) {
						wattron(editor, COLOR_PAIR(4));
					}
					else if (inDirective) {
						wattron(editor, COLOR_PAIR(6));
					}
					else if (inComment) {
						wattron(editor, COLOR_PAIR(7));
					}
					else {
						wattroff(editor, COLOR_PAIR(4));
						wattroff(editor, COLOR_PAIR(5));
						wattroff(editor, COLOR_PAIR(6));
						wattroff(editor, COLOR_PAIR(7));
					}
					wprintw(editor, "%c", file[i]);
				}
			}
			if (file[i] == 10) {
				col = 1;
				++ ln;
				if (ln >= scrollY) {
					wprintw(editor, "\n ");
				}
			}
		}
		wattroff(editor, COLOR_PAIR(4));
		// resize in case of terminal resize
		getmaxyx(stdscr, scrY, scrX);
		wresize(editor, scrY - 1, scrX);
		box(editor, 0, 0);
		// box title and footer
		wmove(editor, 0, 2);
		wprintw(editor, "%s", filename); // editor title (filename)
		wmove(editor, scrY - 2, 2);
		// filesize measure
		shortfilesize = filesize;
		strcpy(sfsmeasure, "B");
		if (shortfilesize >= 1024) {
			shortfilesize = (int)round(shortfilesize/1024);
			strcpy(sfsmeasure, "KB");
			if (shortfilesize >= 1024) {
				shortfilesize = (int)round(shortfilesize/1024);
				strcpy(sfsmeasure, "MB");			
			}
		}
		wprintw(editor, "Mem: %i%s", shortfilesize, sfsmeasure);
		// time
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		wmove(titlebar, 0, scrX - 16);
		wattron(titlebar, COLOR_PAIR(2));
		wprintw(titlebar, "%s", asctime(timeinfo));
		wattroff(titlebar, COLOR_PAIR(2));
		// move back
		wmove(editor, ln + 1 - scrollY, col);
		wprintw(editor, "");
		refreshAll(windows, 2);
		// input
		input = getch();
		switch (input) {
			case 10: {
				file = (char*) realloc(file, filesize+1);
				++filesize;
				file[filesize - 1] = input;
				if (scrollY != 0) {
					scrollY += 1;
				}
			}
			case KEY_UP: {
				if (scrollY > 0) {
					-- scrollY;
				}
				break;
			}
			case KEY_DOWN: {
				++ scrollY;
				break;
			}
			case KEY_BACKSPACE: {
				if (filesize != 1) {
					file = (char*) realloc(file, filesize - 1);
					--filesize;
				}
				break;
			}
			case KEY_STAB: {
				file = (char*) realloc(file, filesize+4);
				filesize += 4;
				for (int i = 1; i<4; ++i) {
					file[filesize-i] = ' ';
				}
				break;
			}
			case ctrl('C'): {
				run = 0;
				break;
			}
			case ctrl('S'): {
				writefile(filename, file);
				sprintf(alert, "File %s saved to disk", filename);
				alertclock = 2;
				break;
			}
			default: {
				if (input < 32 || input > 126) break;
				file = (char*) realloc(file, filesize+1);
				++filesize;
				file[filesize - 1] = input;
				break;
			}
		}
	}
	// end program
	free(file);
	endwin();
	return 0;
}