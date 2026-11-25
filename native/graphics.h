#ifndef IC35_GRAPHICS_H
#define IC35_GRAPHICS_H
/* VIER KI 008 renderer. Include after Game and level_name(). */
void fw_fill(unsigned short color,short x,short y,short w,short h) CALL0;
#define GFX_X 8
#define GFX_Y 14
#define GFX_CELL 20

typedef struct {
    unsigned char cells[42],win[6];
    unsigned char ready,selected,level,result,dir;
    const char *message;
} GfxView;

static void gfx_reset(GfxView *v) {v->ready=0;}

static void gfx_box(int x,int y,int w,int h,int color)
{
    fw_fill((unsigned short)color,(short)x,(short)y,(short)w,1);
    fw_fill((unsigned short)color,(short)x,(short)(y+h-1),(short)w,1);
    fw_fill((unsigned short)color,(short)x,(short)y,1,(short)h);
    fw_fill((unsigned short)color,(short)(x+w-1),(short)y,1,(short)h);
}

static void gfx_disc(int x,int y,int radius,int color)
{
    int dy,a,half;
    const char *widths;
    widths=radius==8?"888776542":"555432";
    for(dy=-radius;dy<=radius;++dy) {
        a=dy<0?-dy:dy;half=widths[a]-'0';
        fw_fill((unsigned short)color,(short)(x-half),(short)(y+dy),(short)(half+half+1),1);
    }
}

/* inv=1 draws the piece white on a black cell (winning stones). */
static void gfx_piece(int x,int y,int side,int inv)
{
    int fg,bg;
    fg=inv?0:1;bg=inv?1:0;
    if(!side) {
        fw_fill(1,(short)(x-1),(short)y,3,1);fw_fill(1,(short)x,(short)(y-1),1,3);
        return;
    }
    gfx_disc(x,y,8,fg);
    if(side==2) gfx_disc(x,y,5,bg);
    else fw_fill((unsigned short)bg,(short)(x-4),(short)(y-4),2,2);
}

static void gfx_win(const Game *g,unsigned char *mask,unsigned char *dir)
{
    int r,c,d,dr,dc,rr,cc,n,i;
    for(r=0;r<6;++r) mask[r]=0;
    *dir=0;
    if(g->result<1||g->result>2) return;
    for(r=0;r<6;++r) for(c=0;c<7;++c) {
        if(g->cell[r][c]!=g->result) continue;
        for(d=0;d<4;++d) {
            dr=0;dc=1;
            if(d==1) {dr=1;dc=0;}
            if(d==2) {dr=1;dc=1;}
            if(d==3) {dr=1;dc=-1;}
            rr=r;cc=c;n=0;
            for(i=0;i<4;++i) {
                if(rr<0||rr>=6||cc<0||cc>=7) break;
                if(g->cell[rr][cc]!=g->result) break;
                ++n;rr+=dr;cc+=dc;
            }
            if(n==4) {
                rr=r;cc=c;
                for(i=0;i<4;++i) {mask[rr]|=(unsigned char)(1<<cc);rr+=dr;cc+=dc;}
                *dir=(unsigned char)d;
                return;
            }
        }
    }
}

static void gfx_draw(const Game *g,const char *message,int level,int selected,GfxView *v)
{
    int r,c,i,x,y,bit,changed;
    unsigned char mask[6];
    const char *shown;
    if(!v->ready) {
        fw_clear();
        for(c=0;c<=7;++c) fw_fill(1,(short)(GFX_X+GFX_CELL*c),GFX_Y,1,121);
        for(r=0;r<=6;++r) fw_fill(1,GFX_X,(short)(GFX_Y+GFX_CELL*r),141,1);
        gfx_box(6,12,145,125,1);
        fw_text("Stufe",155,14);
        fw_text("N Neu",155,50);fw_text("S Stufe",155,64);fw_text("Q Ende",155,78);
        gfx_piece(163,100,1,0);fw_text("Du",179,94);
        gfx_piece(163,120,2,0);fw_text("KI",179,114);
        for(i=0;i<42;++i) v->cells[i]=255;
        for(r=0;r<6;++r) v->win[r]=0;
        v->dir=0;v->selected=255;v->level=0;v->result=255;v->message=0;v->ready=1;
    }
    changed=v->result!=g->result;
    if(changed) gfx_win(g,mask,&v->dir);
    else for(r=0;r<6;++r) mask[r]=v->win[r];
    i=0;
    for(r=0;r<6;++r) {
        for(c=0;c<7;++c) {
            bit=1<<c;
            if(v->cells[i]!=g->cell[r][c]||((mask[r]^v->win[r])&bit)) {
                x=GFX_X+GFX_CELL*c;y=GFX_Y+GFX_CELL*r;
                fw_fill(0,(short)(x+1),(short)(y+1),19,19);
                if(mask[r]&bit) fw_fill(1,(short)(x+1),(short)(y+1),19,19);
                gfx_piece(x+10,y+10,g->cell[r][c],(mask[r]&bit)?1:0);
                v->cells[i]=g->cell[r][c];
            }
            ++i;
        }
        v->win[r]=mask[r];
    }
    if(v->selected!=selected) {
        fw_fill(0,GFX_X,4,141,5);x=GFX_X+10+GFX_CELL*selected;
        for(i=0;i<5;++i) fw_fill(1,(short)(x-4+i),(short)(4+i),(short)(9-i-i),1);
        v->selected=(unsigned char)selected;
    }
    if(v->level!=level) {
        fw_fill(0,155,30,84,16);fw_text(level_name(level),155,30);
        v->level=(unsigned char)level;
    }
    if(changed||v->message!=message) {
        shown=message;
        if(g->result==1) shown="Du gewinnst";
        if(g->result==2) shown="KI gewinnt";
        if(g->result==3) shown="Remis";
        fw_fill(0,0,138,239,22);
        if(g->result==1||g->result==2) gfx_box(2,139,235,21,1);
        fw_text(shown,6,142);
        if(g->result==1||g->result==2)
            fw_text(v->dir==0?"waagerecht":v->dir==1?"senkrecht":"diagonal",110,142);
        fw_fill(0,200,122,39,16);
        v->message=message;v->result=g->result;
    }
}

/* previous=0 redraws the menu; otherwise only its selection frame changes. */
static void gfx_menu(int selected,int previous)
{
    int y;
    if(!previous) {
        fw_clear();fw_text("VIER KI 008",4,4);
        fw_text("Schwierigkeit",4,28);
        fw_text("1 Leicht",26,52);fw_text("2 Mittel",26,76);fw_text("3 Schwer",26,100);
        fw_text("Auf/Ab: Wahl Enter: Start",4,124);fw_text("Q Ende",4,144);
    } else {
        if(selected==previous) return;
        y=48+24*(previous-1);gfx_box(8,y,222,22,0);
        fw_fill(0,12,(short)(y+4),12,16);
    }
    y=48+24*(selected-1);gfx_box(8,y,222,22,1);
    fw_text(">",14,(short)(y+4));
}
#endif
