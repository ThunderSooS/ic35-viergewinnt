#ifndef IC35_GRAPHICS_H
#define IC35_GRAPHICS_H
/* VIER KI 009 renderer. Include after Game and level_name(). */
void fw_fill(unsigned short color,short x,short y,short w,short h) CALL0;
#define GFX_X 3
#define GFX_Y 9
#define GFX_CELL 33
#define GFX_ROW 19

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
        for(c=0;c<=7;++c) fw_fill(1,(short)(GFX_X+GFX_CELL*c),GFX_Y,1,115);
        for(r=0;r<=6;++r) fw_fill(1,GFX_X,(short)(GFX_Y+GFX_ROW*r),232,1);
        gfx_box(1,7,236,119,1);
        fw_text("N Neu S Stufe Q Ende",4,144);
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
                x=GFX_X+GFX_CELL*c;y=GFX_Y+GFX_ROW*r;
                fw_fill(0,(short)(x+1),(short)(y+1),32,18);
                if(mask[r]&bit) fw_fill(1,(short)(x+1),(short)(y+1),32,18);
                gfx_piece(x+17,y+10,g->cell[r][c],(mask[r]&bit)?1:0);
                v->cells[i]=g->cell[r][c];
            }
            ++i;
        }
        v->win[r]=mask[r];
    }
    if(v->selected!=selected) {
        fw_fill(0,GFX_X,0,232,5);x=GFX_X+16+GFX_CELL*selected;
        for(i=0;i<5;++i) fw_fill(1,(short)(x-4+i),(short)i,(short)(9-i-i),1);
        v->selected=(unsigned char)selected;
    }
    if(v->level!=level) {
        fw_fill(0,172,144,48,16);fw_text(level_name(level),172,144);
        v->level=(unsigned char)level;
    }
    if(changed||v->message!=message) {
        shown=message;
        if(g->result==1) shown="Du gewinnst";
        if(g->result==2) shown="KI gewinnt";
        if(g->result==3) shown="Remis";
        fw_fill(0,0,126,239,18);
        if(g->result==1||g->result==2) gfx_box(1,127,236,17,1);
        fw_text(shown,6,128);
        if(g->result==1||g->result==2)
            fw_text(v->dir==0?"waagerecht":v->dir==1?"senkrecht":"diagonal",110,128);
        fw_fill(0,220,144,19,16);
        v->message=message;v->result=g->result;
    }
}

/* previous=0 redraws the menu; otherwise only its selection frame changes. */
static void gfx_menu(int selected,int previous)
{
    int y;
    if(!previous) {
        fw_clear();fw_text("VIER KI 009",4,4);
        fw_text("Schwierigkeit",4,24);
        fw_text("1 Leicht",26,46);fw_text("Einfach",132,46);
        fw_text("2 Mittel",26,68);fw_text("Taktisch",132,68);
        fw_text("3 Schwer",26,90);fw_text("Vorausdenkend",132,90);
        fw_text("Links/Rechts: Spalte",4,112);
        fw_text("Runter/Enter/Leer: Wurf",4,128);
        fw_text("Auf/Ab Enter Start Q Ende",4,144);
    } else {
        if(selected==previous) return;
        y=44+22*(previous-1);gfx_box(4,y,232,20,0);
        fw_fill(0,12,(short)(y+2),12,16);
    }
    y=44+22*(selected-1);gfx_box(4,y,232,20,1);
    fw_text(">",14,(short)(y+2));
}
#endif
