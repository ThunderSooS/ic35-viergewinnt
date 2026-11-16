#include <assert.h>
#include <string.h>
#include "../native/game.c"
static const char *input="1q";
static int reads,clears;
static unsigned char board[6][7];
void fw_clear(void) {++clears;memset(board,0,sizeof(board));}
void fw_text(const char *s,short x,short y)
{assert(s&&x>=0&&x<239&&y>=0&&y+16<=160);}
void fw_char(unsigned short ch,short x,short y)
{
    int r,c;
    assert(x>=0&&x+16<=239&&y>=0&&y+16<=160);
    if(y>=40&&y<=110) {
        r=(y-40)/14;c=(x-6)/28;
        assert(r>=0&&r<6&&c>=0&&c<7);
        board[r][c]=(unsigned char)ch;
    }
}
unsigned short fw_key(void)
{assert(reads<(int)strlen(input));return (unsigned char)input[reads++];}
int main(void)
{
    Game g,saved;
    int d,dr,dc,i,c,r,side,tries,games,moves;
    unsigned long seed;
    init(&g);saved=g;assert(choose(&g)==3);assert(!memcmp(&g,&saved,sizeof(g)));
    assert(drop(&g,-1,1)==-1&&drop(&g,7,1)==-1);
    for(i=0;i<6;++i) assert(drop(&g,0,1+i%2)>=0);
    saved=g;assert(drop(&g,0,1)==-1);assert(!memcmp(&g,&saved,sizeof(g)));
    for(d=0;d<4;++d) {
        dr=0;dc=1;if(d==1){dr=1;dc=0;}if(d==2){dr=1;dc=1;}if(d==3){dr=1;dc=-1;}
        init(&g);
        for(i=0;i<4;++i) g.cell[1+i*dr][3+i*dc]=1;
        for(i=0;i<4;++i) assert(winning(&g,1+i*dr,3+i*dc,1));
    }
    init(&g);g.cell[5][0]=1;g.cell[5][1]=1;g.cell[5][2]=1;g.moves=3;
    saved=g;assert(choose(&g)==3);assert(!memcmp(&g,&saved,sizeof(g)));
    for(r=3;r<6;++r) g.cell[r][6]=2;
    saved=g;assert(choose(&g)==6);assert(!memcmp(&g,&saved,sizeof(g)));
    assert(drop(&g,6,2)==2&&g.result==2);assert(drop(&g,4,1)==-1);
    init(&g);for(r=0;r<6;++r) g.cell[r][3]=(unsigned char)(1+r%2);
    assert(choose(&g)!=3);
    for(r=0;r<6;++r) for(c=0;c<7;++c) g.cell[r][c]=1;
    assert(choose(&g)==-1);
    init(&g);
    for(r=0;r<6;++r) for(c=0;c<7;++c) g.cell[r][c]=(unsigned char)(1+(r+c/2)%2);
    for(r=0;r<6;++r) for(c=0;c<7;++c) assert(!winning(&g,r,c,g.cell[r][c]));
    g.cell[0][0]=0;g.moves=41;
    assert(drop(&g,0,1)==0&&g.result==3&&g.moves==42);
    assert(sizeof(Game)==44);
    seed=19;
    for(games=0;games<300;++games) {
        init(&g);side=1;
        for(moves=0;moves<42&&!g.result;++moves) {
            if(side==2) {
                saved=g;c=choose(&g);assert(!memcmp(&g,&saved,sizeof(g)));
            } else {
                seed=seed*1664525UL+1013904223UL;c=(int)((seed>>16)%7);tries=0;
                while(g.cell[0][c]&&tries<7){c=(c+1)%7;++tries;}
                assert(tries<7);
            }
            assert(c>=0&&c<7&&drop(&g,c,side)>=0);
            assert(g.moves==moves+1);side=3-side;
        }
        assert(g.result>=1&&g.result<=3);
    }
    input="1q";reads=0;clears=0;game_main();
    assert(reads==2&&clears==3&&board[5][0]=='X'&&board[5][3]=='O');
    input="1Nq";reads=0;game_main();
    for(r=0;r<6;++r) for(c=0;c<7;++c) assert(board[r][c]=='.');
    input="xQ";reads=0;game_main();assert(reads==2);
    return 0;
}
