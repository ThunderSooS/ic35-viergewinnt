#include <assert.h>
#include <string.h>
#include "../native/game.c"
static const char *input="11q";
static int reads,clears,polls,poll_after;
static unsigned short poll_key;
#include "pixel_mock.h"
unsigned short fw_key(void)
{screen_decode();assert(reads<(int)strlen(input));return (unsigned char)input[reads++];}
unsigned short fw_poll(void)
{
    unsigned short key;
    ++polls;
    if(poll_key&&polls>=poll_after) {key=poll_key;poll_key=0;screen_decode();return key;}
    return 0;
}
static int threats(Game *g,int side)
{
    int c,r,n;
    n=0;
    for(c=0;c<7;++c) {
        r=landing(g,c);if(r<0) continue;
        g->cell[r][c]=(unsigned char)side;
        if(winning(g,r,c,side)) ++n;
        g->cell[r][c]=0;
    }
    return n;
}
int main(void)
{
    Game g,saved;
    int d,dr,dc,i,c,r,side,tries,games,moves,level,mirror,p,n;
    unsigned long seed;
    init(&g);saved=g;assert(choose(&g)==3);assert(!memcmp(&g,&saved,sizeof(g)));
    assert(drop(&g,-1,1)==-1&&drop(&g,7,1)==-1);
    for(i=0;i<6;++i) assert(drop(&g,0,1+i%2)>=0);
    saved=g;assert(drop(&g,0,1)==-1);assert(!memcmp(&g,&saved,sizeof(g)));
    for(d=0;d<4;++d) {
        dr=0;dc=1;if(d==1){dr=1;dc=0;}if(d==2){dr=1;dc=1;}if(d==3){dr=1;dc=-1;}
        for(r=0;r<6;++r) for(c=0;c<7;++c) {
            if(r+3*dr>=6||c+3*dc<0||c+3*dc>=7) continue;
            for(p=1;p<=2;++p) {
                init(&g);
                for(i=0;i<4;++i) g.cell[r+i*dr][c+i*dc]=(unsigned char)p;
                for(i=0;i<4;++i) assert(winning(&g,r+i*dr,c+i*dc,p));
                g.cell[r+3*dr][c+3*dc]=0;
                for(i=0;i<3;++i) assert(!winning(&g,r+i*dr,c+i*dc,p));
            }
        }
    }
    for(level=1;level<=3;++level) {
        init(&g);g.cell[5][0]=1;g.cell[5][1]=1;g.cell[5][2]=1;g.moves=3;
        saved=g;assert(choose_level(&g,level)==3);assert(!memcmp(&g,&saved,sizeof(g)));
        for(r=3;r<6;++r) g.cell[r][6]=2;
        g.moves=6;saved=g;assert(choose_level(&g,level)==6);assert(!memcmp(&g,&saved,sizeof(g)));
        assert(drop(&g,6,2)==2&&g.result==2);assert(choose_level(&g,level)==-1);
        assert(drop(&g,4,1)==-1);
    }
    init(&g);for(r=0;r<6;++r) g.cell[r][3]=(unsigned char)(1+r%2);
    assert(choose(&g)!=3);
    for(r=0;r<6;++r) for(c=0;c<7;++c) g.cell[r][c]=1;
    for(level=1;level<=3;++level) assert(choose_level(&g,level)==-1);
    init(&g);
    for(r=0;r<6;++r) for(c=0;c<7;++c) g.cell[r][c]=(unsigned char)(1+(r+c/2)%2);
    for(r=0;r<6;++r) for(c=0;c<7;++c) assert(!winning(&g,r,c,g.cell[r][c]));
    g.cell[0][0]=0;g.moves=41;
    for(level=1;level<=3;++level) {saved=g;assert(choose_level(&g,level)==0);assert(!memcmp(&g,&saved,sizeof(g)));}
    assert(drop(&g,0,1)==0&&g.result==3&&g.moves==42);
    assert(sizeof(Game)==44);
    for(level=2;level<=3;++level) for(mirror=0;mirror<2;++mirror) {
        init(&g);
        assert(drop(&g,3,1)>=0);assert(drop(&g,choose_level(&g,level),2)>=0);
        assert(drop(&g,mirror?4:2,1)>=0);assert(drop(&g,choose_level(&g,level),2)>=0);
        assert(drop(&g,mirror?2:4,1)>=0);assert(threats(&g,1)<2);
        if(!g.result) assert(drop(&g,choose_level(&g,level),2)>=0);
        if(!g.result) assert(drop(&g,mirror?1:5,1)>=0);
        assert(g.result!=1);
    }
    for(level=2;level<=3;++level) {
        init(&g);assert(drop(&g,3,1)>=0);assert(drop(&g,3,2)>=0);assert(drop(&g,2,1)>=0);
        saved=g;poll_key=0;polls=0;choose_level(&g,level);n=polls;
        for(p=0;p<3;++p) for(i=1;i<=n;++i) {
            poll_key=(unsigned short)(p==0?'Q':p==1?'N':'S');poll_after=i;polls=0;
            assert(choose_level(&g,level)==-2-p);
            assert(!poll_key&&!memcmp(&g,&saved,sizeof(g)));
        }
    }
    seed=19;
    for(level=1;level<=3;++level) for(games=0;games<(level==1?300:50);++games) {
        init(&g);side=1;
        for(moves=0;moves<42&&!g.result;++moves) {
            if(side==2) {saved=g;c=choose_level(&g,level);assert(!memcmp(&g,&saved,sizeof(g)));}
            else {
                seed=seed*1664525UL+1013904223UL;c=(int)((seed>>16)%7);tries=0;
                while(g.cell[0][c]&&tries<7){c=(c+1)%7;++tries;}
                assert(tries<7);
            }
            assert(c>=0&&c<7&&drop(&g,c,side)>=0);
            assert(g.moves==moves+1);side=3-side;
        }
        assert(g.result>=1&&g.result<=3);
    }
    input="11q";reads=0;clears=0;game_main();
    assert(reads==3&&clears==2&&board[5][0]=='X'&&board[5][3]=='O');
    input="11Nq";reads=0;game_main();
    for(r=0;r<6;++r) for(c=0;c<7;++c) assert(board[r][c]=='.');
    input="xQ";reads=0;game_main();assert(reads==2);
    input="\rQ";reads=0;game_main();assert(!strcmp(shown_level,"Mittel"));
    input="3NQ";reads=0;game_main();assert(!strcmp(shown_level,"Schwer"));
    input="1S3NQ";reads=0;game_main();assert(!strcmp(shown_level,"Schwer"));
    input="1SQ";reads=0;game_main();assert(reads==3);
    input="1435617Q";reads=0;game_main();assert(!strcmp(status,"Du gewinnst"));
    assert(board[5][2]=='X'&&board[5][3]=='X'&&board[5][4]=='X'&&board[5][5]=='X');
    input="14356NQ";reads=0;game_main();assert(!strcmp(status,"Dein Zug: X"));
    for(r=0;r<6;++r) for(c=0;c<7;++c) assert(board[r][c]=='.');
    input="24";reads=0;polls=0;poll_after=1;poll_key='Q';game_main();assert(reads==2);
    input="24Q";reads=0;polls=0;poll_key='N';game_main();assert(!strcmp(status,"Dein Zug: X"));
    for(r=0;r<6;++r) for(c=0;c<7;++c) assert(board[r][c]=='.');
    input="241Q";reads=0;polls=0;poll_key='S';game_main();assert(!strcmp(shown_level,"Leicht"));
    return 0;
}
