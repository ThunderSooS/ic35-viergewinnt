#ifndef IC35_PIXEL_MOCK_H
#define IC35_PIXEL_MOCK_H
/* Host-only framebuffer. The native APP does not allocate this memory. */
static unsigned char screen[160][239],board[6][7],header[7];
static char status[32],shown_level[16];
static int ai_frames;
void fw_clear(void) {++clears;memset(screen,0,sizeof(screen));}
void fw_fill(unsigned short color,short x,short y,short w,short h)
{
    int xx,yy;
    assert(color<=1&&x>=0&&y>=0&&w>0&&h>0&&x+w<=239&&y+h<=160);
    for(yy=y;yy<y+h;++yy)for(xx=x;xx<x+w;++xx)screen[yy][xx]=(unsigned char)color;
}
void fw_text(const char *s,short x,short y)
{
    assert(s&&x>=0&&y>=0&&y+16<=160&&x+8*(int)strlen(s)<=239);
    if(y==142&&x==6) {assert(strlen(s)<sizeof(status));strcpy(status,!strcmp(s,"Remis")?"Unentschieden":s);}
    if(!strcmp(s,"Leicht")||!strcmp(s,"Mittel")||!strcmp(s,"Schwer"))strcpy(shown_level,s);
    if(!strcmp(s,"KI denkt"))++ai_frames;
}
void fw_char(unsigned short ch,short x,short y)
{assert(ch&&x>=0&&x+16<=239&&y>=0&&y+16<=160);}
static void screen_decode(void)
{
    int r,c,x,y,inv,a,b;
    for(c=0;c<7;++c)header[c]=(unsigned char)(screen[4][18+20*c]?'v':'1'+c);
    for(r=0;r<6;++r)for(c=0;c<7;++c) {
        x=18+20*c;y=24+20*r;
        inv=screen[y-9][x-9];a=screen[y][x+3]^inv;b=screen[y][x+6]^inv;
        board[r][c]=(unsigned char)(a?'X':b?'O':'.');
    }
}
#endif
