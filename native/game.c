/* VIER KI 004: nonrecursive tactics, multiplication-free wins, polled input. */
#ifdef __SDCC
#define CALL0 __sdcccall(0)
#else
#define CALL0
#endif
void fw_clear(void) CALL0;
void fw_text(const char *s, short x, short y) CALL0;
void fw_char(unsigned short ch, short x, short y) CALL0;
unsigned short fw_key(void) CALL0;
unsigned short fw_poll(void) CALL0;

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

static int span(const Game *g,int r,int c,int dr,int dc,int side)
{
    int n,rr,cc;
    n=1;rr=r+dr;cc=c+dc;
    while(rr>=0&&rr<6&&cc>=0&&cc<7) {
        if(g->cell[rr][cc]!=side) break;
        ++n;rr+=dr;cc+=dc;
    }
    rr=r-dr;cc=c-dc;
    while(rr>=0&&rr<6&&cc>=0&&cc<7) {
        if(g->cell[rr][cc]!=side) break;
        ++n;rr-=dr;cc-=dc;
    }
    return n;
}

static int winning(const Game *g,int r,int c,int side)
{
    return span(g,r,c,0,1,side)>=4||span(g,r,c,1,0,side)>=4||
           span(g,r,c,1,1,side)>=4||span(g,r,c,1,-1,side)>=4;
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

/* Negative command results: -2 quit, -3 new game, -4 level menu. */
static int ai_command(void)
{
    unsigned short key;
    key=fw_poll();
    if(key=='q'||key=='Q') return -2;
    if(key=='n'||key=='N') return -3;
    if(key=='s'||key=='S') return -4;
    return 0;
}

static int options(Game *g,int side,int stop)
{
    int c,r,n,command;
    n=0;
    for(c=0;c<7;++c) {
        command=ai_command();if(command) return command;
        r=landing(g,c);if(r<0) continue;
        g->cell[r][c]=(unsigned char)side;
        if(winning(g,r,c,side)) ++n;
        g->cell[r][c]=0;
        if(n>=stop) break;
    }
    return n;
}

static int connection(const Game *g,int r,int c)
{
    int d,dr,dc,n,score;
    score=0;
    for(d=0;d<4;++d) {
        dr=0;dc=1;
        if(d==1) {dr=1;dc=0;}
        if(d==2) {dr=1;dc=1;}
        if(d==3) {dr=1;dc=-1;}
        n=span(g,r,c,dr,dc,2);
        if(n==2) score+=12;
        if(n==3) score+=80;
    }
    return score;
}

static int choose_level(Game *g,int level)
{
    const char *order;
    int best,best_score,i,c,r,h,rh,human,own,command,danger,reply,bad,score,win;
    best=choose(g);
    if(best<0||level<=1) return best;
    command=ai_command();if(command) return command;
    r=landing(g,best);g->cell[r][best]=2;
    win=winning(g,r,best,2);g->cell[r][best]=0;
    if(win) return best;
    human=0;own=0;
    for(r=0;r<6;++r) for(c=0;c<7;++c) {
        if(g->cell[r][c]==1) ++human;
        if(g->cell[r][c]==2) ++own;
    }
    order="4352617";best_score=-16000;
    for(i=0;i<7;++i) {
        command=ai_command();if(command) return command;
        c=order[i]-'1';r=landing(g,c);if(r<0) continue;
        g->cell[r][c]=2;
        danger=human>=3?options(g,1,1):0;
        if(danger<0) {g->cell[r][c]=0;return danger;}
        score=c<3?c:6-c;bad=0;
        if(danger) score=-12000;
        else {
            if(human>=2) for(h=0;h<7;++h) {
                command=ai_command();
                if(command) {g->cell[r][c]=0;return command;}
                rh=landing(g,h);if(rh<0) continue;
                g->cell[rh][h]=1;
                reply=own>=2?options(g,2,1):0;
                danger=reply?0:options(g,1,2);
                if(reply<0||danger<0) {
                    g->cell[rh][h]=0;g->cell[r][c]=0;
                    return reply<0?reply:danger;
                }
                if(danger>=2) ++bad;
                g->cell[rh][h]=0;
            }
            if(bad) score=-6000-100*bad;
            else if(level>=3) {
                reply=own>=2?options(g,2,2):0;
                if(reply<0) {g->cell[r][c]=0;return reply;}
                if(reply==1) score+=500;
                if(reply>=2) score+=5000;
                score+=connection(g,r,c);
            }
        }
        g->cell[r][c]=0;
        if(score>best_score) {best_score=score;best=c;}
        fw_char((unsigned short)('1'+i),216,128);
    }
    return best;
}

static const char *level_name(int level)
{
    if(level==1) return "Leicht";
    if(level==3) return "Schwer";
    return "Mittel";
}

static int select_level(int previous)
{
    unsigned short key;
    for(;;) {
        fw_clear();
        fw_text("VIER KI 004",4,4);
        fw_text("Schwierigkeit",4,28);
        fw_text("1 Leicht",4,52);
        fw_text("2 Mittel",4,72);
        fw_text("3 Schwer",4,92);
        fw_text("Bisher:",4,116);
        fw_text(level_name(previous),76,116);
        fw_text("1-3 Wahl Enter OK Q Ende",4,144);
        key=fw_key();
        if(key=='q'||key=='Q') return 0;
        if(key>='1'&&key<='3') return (int)(key-'0');
        if(key==13||key==10) return previous;
    }
}

static void draw(const Game *g,const char *message,int level)
{
    int r,c;
    unsigned short ch;
    fw_clear();
    fw_text("VIER KI 004",4,4);
    fw_text(level_name(level),116,4);
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
    fw_text("N Neu S Stufe Q Ende",4,144);
}

void game_main(void)
{
    Game g;
    unsigned short key;
    int col,level;
    const char *message;
    level=select_level(2);
    if(!level) return;
    init(&g); message="Dein Zug: X";
    for(;;) {
        draw(&g,message,level);
        key=fw_key();
        if(key=='q'||key=='Q') return;
        if(key=='s'||key=='S') {
            level=select_level(level);
            if(!level) return;
            init(&g);message="Dein Zug: X";continue;
        }
        if(key=='n'||key=='N') {init(&g);message="Dein Zug: X";continue;}
        if(g.result||key<'1'||key>'7') continue;
        if(drop(&g,(int)(key-'1'),1)<0) {message="Spalte voll";continue;}
        if(!g.result) {
            draw(&g,"KI denkt: O",level);
            col=choose_level(&g,level);
            if(col==-2) return;
            if(col==-4) {level=select_level(level);if(!level) return;}
            if(col==-3||col==-4) {init(&g);message="Dein Zug: X";continue;}
            if(col>=0) drop(&g,col,2);
        }
        message="Dein Zug: X";
    }
}
