#include <assert.h>
#include <stdio.h>
#include <string.h>
#define CALL0
static unsigned char pixels[160][239],saved[160][239];
static int clears,fills,texts;
typedef struct {unsigned char cell[6][7];unsigned char moves,result;} Game;
void fw_clear(void) {++clears;memset(pixels,0,sizeof(pixels));}
void fw_text(const char *s,short x,short y)
{assert(s&&x>=0&&y>=0&&y+16<=160&&x+8*(int)strlen(s)<=239);++texts;}
static const char *level_name(int level)
{return level==1?"Leicht":level==3?"Schwer":"Mittel";}
#include "../native/graphics.h"
void fw_fill(unsigned short color,short x,short y,short w,short h)
{
    int xx,yy;
    assert(color<=1&&w>0&&h>0&&x>=0&&y>=0&&x+w<=239&&y+h<=160);
    ++fills;
    for(yy=y;yy<y+h;++yy) for(xx=x;xx<x+w;++xx) pixels[yy][xx]=(unsigned char)color;
}
static void same_board(void)
{int r;for(r=14;r<=134;++r)assert(!memcmp(pixels[r]+8,saved[r]+8,141));}
static int pieces(int side,int r,int c)
{
    int x,y;x=18+20*c;y=24+20*r;
    if(side==1)return pixels[y][x+3]&&pixels[y][x+6];
    if(side==2)return !pixels[y][x]&&!pixels[y][x+3]&&pixels[y][x+6];
    return pixels[y][x]&&!pixels[y][x+3]&&!pixels[y][x+6];
}
int main(void)
{
    Game g,before;GfxView v;
    unsigned char dir;int r,c,i,d,dr,dc,a,b,old_fills,old_texts,old_clears;
    unsigned char mask[6];
    memset(&g,0,sizeof(g));before=g;gfx_reset(&v);
    gfx_draw(&g,"Dein Zug",2,3,&v);
    assert(!memcmp(&g,&before,sizeof(g))&&clears==1);
    for(r=0;r<6;++r)for(c=0;c<7;++c)assert(pieces(0,r,c));
    old_fills=fills;old_texts=texts;
    gfx_draw(&g,"Dein Zug",2,3,&v);assert(fills==old_fills&&texts==old_texts&&clears==1);
    memcpy(saved,pixels,sizeof(saved));
    for(c=0;c<7;++c) {gfx_draw(&g,"Dein Zug",2,c,&v);same_board();}
    assert(clears==1);
    old_fills=fills;g.cell[5][2]=1;g.moves=1;
    gfx_draw(&g,"Dein Zug",2,6,&v);assert(fills-old_fills==19&&pieces(1,5,2));
    old_fills=fills;g.cell[5][3]=2;g.moves=2;
    gfx_draw(&g,"Dein Zug",2,6,&v);assert(fills-old_fills==29&&pieces(2,5,3));
    gfx_draw(&g,"KI denkt",2,6,&v);assert(clears==1);
    for(d=0;d<4;++d) {
        dr=0;dc=1;if(d==1){dr=1;dc=0;}if(d==2){dr=1;dc=1;}if(d==3){dr=1;dc=-1;}
        for(r=0;r<6;++r)for(c=0;c<7;++c) {
            if(r+3*dr>=6||c+3*dc<0||c+3*dc>=7)continue;
            for(a=1;a<=2;++a) {
                memset(&g,0,sizeof(g));g.result=(unsigned char)a;
                for(i=0;i<4;++i)g.cell[r+i*dr][c+i*dc]=(unsigned char)a;
                before=g;gfx_win(&g,mask,&dir);b=0;
                for(i=0;i<4;++i)assert(mask[r+i*dr]&(1<<(c+i*dc)));
                for(i=0;i<6;++i){int j;for(j=0;j<7;++j)if(mask[i]&(1<<j))++b;}
                assert(b==4&&dir==(d==3?3:d));gfx_reset(&v);gfx_draw(&g,"Dein Zug",3,3,&v);
                for(i=0;i<4;++i)assert(pixels[15+20*(r+i*dr)][9+20*(c+i*dc)]);
                assert(!memcmp(&g,&before,sizeof(g)));
            }
        }
    }
    memset(&g,0,sizeof(g));g.result=3;gfx_reset(&v);
    gfx_draw(&g,"Unentschieden",1,0,&v);
    for(r=0;r<6;++r)assert(v.win[r]==0);
    gfx_menu(2,0);old_clears=clears;old_fills=fills;
    gfx_menu(2,2);assert(clears==old_clears&&fills==old_fills);
    gfx_menu(1,2);gfx_menu(3,1);assert(clears==old_clears);
    printf("%d,%d,%d\n",clears,fills,texts);
    return 0;
}
