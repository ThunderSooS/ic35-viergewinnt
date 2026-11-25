#include <assert.h>
#include <string.h>
#include "../src/game.c"
static const char *input;
static int reads,clears;
#include "pixel_mock.h"
unsigned short fw_key(void)
{screen_decode();assert(reads<(int)strlen(input));return (unsigned char)input[reads++];}
unsigned short fw_poll(void) {return 0;}
static void run(const char *s)
{input=s;reads=0;clears=0;ai_frames=0;game_main();assert(reads==(int)strlen(s));}
static int stones(void)
{int r,c,n;n=0;for(r=0;r<6;++r)for(c=0;c<7;++c)if(board[r][c]=='X'||board[r][c]=='O')++n;return n;}
static void marker(int selected)
{int c,n;n=0;for(c=0;c<7;++c)if(header[c]=='v')++n;assert(n==1&&header[selected]=='v');}
int main(void)
{
    Game g,saved;
    int c,i;
    const char *keys;
    for(c=0;c<7;++c) {
        assert(move_selection(c,KEY_LEFT)==(c?c-1:0));
        assert(move_selection(c,KEY_RIGHT)==(c<6?c+1:6));
        assert(move_selection(c,0x0101)==c);
    }
    for(i=0;i<65536;++i) assert(!!drop_key((unsigned short)i)==(i==4||i==10||i==13||i==32));
    input="\003\003\003\r";reads=0;assert(select_level(2)==1);
    input="\004\004\004\r";reads=0;assert(select_level(2)==3);
    input="\003\004\r";reads=0;assert(select_level(2)==2);
    run("1Q");assert(stones()==0);marker(3);
    run("1\001\002Q");assert(stones()==0&&ai_frames==0);marker(3);
    run("1\001\001\001\001 Q");assert(board[5][0]=='X'&&stones()==2);marker(0);
    run("1\002\002\002\002 Q");assert(board[5][6]=='X'&&stones()==2);marker(6);
    keys="\004\r\n ";
    for(i=0;i<4;++i) {
        char sequence[5];
        sequence[0]='1';sequence[1]=keys[i];sequence[2]='Q';sequence[3]=0;
        run(sequence);assert(board[5][3]=='X'&&stones()==2&&ai_frames==1);marker(3);
    }
    run("1\001\004\004Q");assert(board[5][2]=='X'&&board[4][2]=='X'&&stones()==4);marker(2);
    run("1    Q");assert(stones()==6&&ai_frames==3&&!strcmp(status,"Spalte voll"));marker(3);
    run("1    \002\rQ");assert(board[5][4]=='X'&&stones()==8);marker(4);
    run("1\001N Q");assert(board[5][3]=='X'&&board[5][2]=='.');marker(3);
    run("1\001S1 Q");assert(board[5][3]=='X'&&board[5][2]=='.');marker(3);
    run("14356\001\002\004\r Q");assert(stones()==7&&!strcmp(status,"Du gewinnst"));marker(5);
    run("11q");assert(board[5][0]=='X'&&board[5][3]=='O'&&clears==2);marker(0);
    run("2\001\004Q");assert(board[5][2]=='X'&&stones()==2);marker(2);
    init(&g);saved=g;assert(choose(&g)==3&&!memcmp(&g,&saved,sizeof(g)));
    return 0;
}
