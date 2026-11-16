/* VIER KI 001: stack-local state; firmware calls preserve general registers. */
#ifdef __SDCC
#define CALL0 __sdcccall(0)
#else
#define CALL0
#endif
void fw_clear(void) CALL0;
void fw_text(const char *s, short x, short y) CALL0;
void fw_char(unsigned short ch, short x, short y) CALL0;
unsigned short fw_key(void) CALL0;

typedef struct {
    unsigned char cell[6][7];
    unsigned char moves, result;
} Game;

static void init(Game *g)
{
    int r,c;
    for(r=0;r<6;++r) for(c=0;c<7;++c) g->cell[r][c]=0;
    g->moves=0; g->result=0;
}

static int winning(const Game *g,int r,int c,int side)
{
    int d,dr,dc,sign,rr,cc,n;
    for(d=0;d<4;++d) {
        dr=0; dc=1;
        if(d==1) {dr=1;dc=0;}
        if(d==2) {dr=1;dc=1;}
        if(d==3) {dr=1;dc=-1;}
        n=1;
        for(sign=-1;sign<=1;sign+=2) {
            rr=r+sign*dr; cc=c+sign*dc;
            while(rr>=0&&rr<6&&cc>=0&&cc<7) {
                if(g->cell[rr][cc]!=side) break;
                ++n; rr+=sign*dr; cc+=sign*dc;
            }
        }
        if(n>=4) return 1;
    }
    return 0;
}

static int landing(const Game *g,int col)
{
    int r;
    if(col<0||col>=7) return -1;
    r=5;
    while(r>=0&&g->cell[r][col]) --r;
    return r;
}

static int drop(Game *g,int col,int side)
{
    int r;
    if(g->result||side<1||side>2) return -1;
    r=landing(g,col);
    if(r<0) return -1;
    g->cell[r][col]=(unsigned char)side; ++g->moves;
    if(winning(g,r,col,side)) g->result=(unsigned char)side;
    else if(g->moves==42) g->result=3;
    return r;
}

static int choose(Game *g)
{
    const char *order;
    int side,i,c,r,win,fallback;
    if(g->result) return -1;
    order="4352617"; fallback=-1;
    for(side=2;side>=1;--side) {
        for(i=0;i<7;++i) {
            c=order[i]-'1'; r=landing(g,c);
            if(r<0) continue;
            if(fallback<0) fallback=c;
            g->cell[r][c]=(unsigned char)side;
            win=winning(g,r,c,side);
            g->cell[r][c]=0;
            if(win) return c;
        }
    }
    return fallback;
}

static void draw(const Game *g,const char *message)
{
    int r,c;
    unsigned short ch;
    fw_clear();
    fw_text("VIER KI 001",4,4);
    for(c=0;c<7;++c) fw_char((unsigned short)('1'+c),(short)(6+28*c),22);
    for(r=0;r<6;++r) {
        for(c=0;c<7;++c) {
            ch='.';
            if(g->cell[r][c]==1) ch='X';
            if(g->cell[r][c]==2) ch='O';
            fw_char(ch,(short)(6+28*c),(short)(40+14*r));
        }
    }
    if(g->result==1) message="Du gewinnst";
    else if(g->result==2) message="KI gewinnt";
    else if(g->result==3) message="Unentschieden";
    fw_text(message,4,128);
    fw_text("N Neu Q Ende",4,144);
}

void game_main(void)
{
    Game g;
    unsigned short key;
    int col;
    const char *message;
    init(&g); message="Dein Zug: X";
    for(;;) {
        draw(&g,message);
        key=fw_key();
        if(key=='q'||key=='Q') return;
        if(key=='n'||key=='N') {init(&g);message="Dein Zug: X";continue;}
        if(g.result||key<'1'||key>'7') continue;
        if(drop(&g,(int)(key-'1'),1)<0) {message="Spalte voll";continue;}
        if(!g.result) {
            draw(&g,"KI denkt: O");
            col=choose(&g);
            if(col>=0) drop(&g,col,2);
        }
        message="Dein Zug: X";
    }
}
