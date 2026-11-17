/* VIER KI 003: bounded AI search; explicit CALL0 firmware boundary. */
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

/* All search state is automatic; no global RAM or copied boards. */
typedef struct {
    unsigned short nodes,limit;
    int root_depth,best;
    unsigned char aborted;
} Search;

static int evaluate(const Game *g)
{
    int r,c,d,dr,dc,k,a,b,er,ec,v,score,t1,t2;
    unsigned short mask1,mask2;
    score=0; mask1=0; mask2=0;
    for(r=0;r<6;++r) {
        if(g->cell[r][3]==2) score+=6;
        if(g->cell[r][3]==1) score-=6;
        for(c=0;c<7;++c) for(d=0;d<4;++d) {
            dr=0;dc=1;
            if(d==1) {dr=1;dc=0;}
            if(d==2) {dr=1;dc=1;}
            if(d==3) {dr=1;dc=-1;}
            if(r+3*dr>=6||c+3*dc<0||c+3*dc>=7) continue;
            a=0;b=0;er=0;ec=0;
            for(k=0;k<4;++k) {
                v=g->cell[r+k*dr][c+k*dc];
                if(v==1) ++a;
                else if(v==2) ++b;
                else {er=r+k*dr;ec=c+k*dc;}
            }
            if(!a) {
                if(b==1) score+=3;
                if(b==2) score+=18;
                if(b==3) {
                    score+=100;
                    if(landing(g,ec)==er) mask2|=(unsigned short)(1<<ec);
                }
            }
            if(!b) {
                if(a==1) score-=3;
                if(a==2) score-=18;
                if(a==3) {
                    score-=100;
                    if(landing(g,ec)==er) mask1|=(unsigned short)(1<<ec);
                }
            }
        }
    }
    t1=0;t2=0;
    for(c=0;c<7;++c) {
        if(mask1&(1<<c)) ++t1;
        if(mask2&(1<<c)) ++t2;
    }
    score+=400*(t2-t1);
    if(t2>=2) score+=3000;
    if(t1>=2) score-=3000;
    if(score>8000) return 8000;
    if(score< -8000) return -8000;
    return score;
}

static int search(Game *g,int depth,int side,int alpha,int beta,Search *s)
{
    int i,c,r,value,best,found;
    const char *order;
    if(s->nodes>=s->limit) {s->aborted=1;return 0;}
    ++s->nodes;
    if(g->moves==42) return 0;
    if(!depth) return evaluate(g);
    order="4352617";found=0;
    best=side==2?-16000:16000;
    for(i=0;i<7;++i) {
        c=order[i]-'1';r=landing(g,c);
        if(r<0) continue;
        g->cell[r][c]=(unsigned char)side;++g->moves;
        if(winning(g,r,c,side)) value=side==2?12000+depth:-12000-depth;
        else value=search(g,depth-1,3-side,alpha,beta,s);
        --g->moves;g->cell[r][c]=0;
        if(s->aborted) return 0;
        if(!found||(side==2?value>best:value<best)) {
            best=value;
            if(depth==s->root_depth) s->best=c;
        }
        found=1;
        if(side==2) {if(best>alpha) alpha=best;}
        else {if(best<beta) beta=best;}
        if(alpha>=beta) break;
    }
    return found?best:0;
}

static int choose_level(Game *g,int level)
{
    Search s;
    int best,depth,max_depth;
    best=choose(g);
    if(best<0||level<=1) return best;
    s.nodes=0;s.limit=(unsigned short)(level==2?800:2400);
    s.aborted=0;max_depth=level==2?4:6;
    for(depth=1;depth<=max_depth;++depth) {
        s.root_depth=depth;s.best=best;
        search(g,depth,2,-16000,16000,&s);
        if(s.aborted) break;
        best=s.best;
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
        fw_text("VIER KI 003",4,4);
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
    fw_text("VIER KI 003",4,4);
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
            if(col>=0) drop(&g,col,2);
        }
        message="Dein Zug: X";
    }
}
