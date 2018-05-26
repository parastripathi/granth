
/**** includes ***/
#include <unistd.h>//read(),STDIN_FILENO
#include <termios.h>
#include <stdlib.h>//atexit()
#include <ctype.h>//iscntrl()
#include <stdio.h>//printf()
#include <errno.h>
#include <sys/ioctl.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/
struct editorConfig {
	int screenrows;
	int screencols; 
	struct termios orig_termios;
};

struct editorConfig E;

/*** terminal ***/

void die(const char *s)
{	
	write(STDOUT_FILENO, "\x1b[2J",4);
	write(STDOUT_FILENO, "\x1b[H", 3);
	
	perror(s);
	exit(1);
}

void disableRawMode()
{
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) 
		die("tcsetattr");
	
}
void enableRawMode()
{
	if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1 ) 
		die("tcgetattr") ;
	atexit(disableRawMode);
	
	struct termios raw = E.orig_termios;
	
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
	raw.c_cc[VMIN] = 0;// min bytes required before read() can return
	raw.c_cc[VTIME] = 1; //in tenth of a second so 1/10th
	
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

char editorReadKey()
{
	int nread;
	char c;
	while((nread = read(STDIN_FILENO, &c, 1)) != 1)
	{
		if(nread == -1 && errno != EAGAIN) 
			die("read");
	}
	
	return c;
}

int getWindowSize(int *rows, int *cols)
{
	struct winsize ws;
	
	if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
	{
		return -1;
	}
	else
	{
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		
		return 0;
	}
}

/***output***/
void editorDrawRows()
{
	int i;
	for(i = 0; i < E.screenrows ; i++)
	{
		write(STDOUT_FILENO, "~", 1);
		
		if(i < E.screenrows - 1)
		{
			write(STDOUT_FILENO. "\r\n", 2);
		}
	}
}

void editorRefreshScreen()
{
	write(STDOUT_FILENO, "\x1b[2J",4);
	write(STDOUT_FILENO, "\x1b[H", 3);
	
	editorDrawRows();
	
	write(STDOUT_FILENO, "\x1b[H", 3);
}

/***input***/

void editorProcessKeypress()
{
	char c = editorReadKey();
	
	switch(c)
	{
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J",4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;	
	}
}

/*** init ***/

void initEditor() {
	if(getWindowSize(&E.screenrows, &E.screencols) == -1)
		die("getWindowSize");
}

int main()
{	
	enableRawMode();
	initEditor();
	
	while(1)
	{	
		editorRefreshScreen();
		editorProcessKeypress();
	}	
/*
	while(1)
	{
	char c = '\0';
		if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) 
			die("read");
		{
			if(iscntrl(c))
			{
				printf("%d\r\n",c);
			}
			else 
			{
				printf("%d ('%c')\r\n", c, c);	
			}	
		}
		
		if(c == CTRL_KEY('q')) break;
	}*/
	return 0;
}
