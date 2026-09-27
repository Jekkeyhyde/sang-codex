/* Памятка SANG · Орландо — нативная версия (Win32) */
#include <windows.h>
#include <richedit.h>
#include <uxtheme.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>

/* ---------------- данные ---------------- */
typedef struct{WCHAR *t,*code,*num,*stars,*jur,*title,*pun,*note,*full,*kw,*doc;WCHAR**arts;int na;WCHAR**say;int ns;WCHAR**steps;int nst;WCHAR**warn;int nw;
  WCHAR*tL,*fL,*dL;WCHAR**kws;int*kwWords;int nk;int sit;}Item;
static Item*IT;static int NIT;

static WCHAR*u8(const char*s,int len){int n=MultiByteToWideChar(CP_UTF8,0,s,len,0,0);WCHAR*w=malloc((n+1)*sizeof(WCHAR));MultiByteToWideChar(CP_UTF8,0,s,len,w,n);w[n]=0;return w;}
static void lowerw(WCHAR*w){CharLowerBuffW(w,(DWORD)wcslen(w));for(;*w;w++)if(*w==0x451)*w=0x435;}
static WCHAR*lowdup(const WCHAR*s){WCHAR*d=_wcsdup(s);lowerw(d);return d;}
static WCHAR**splitw(WCHAR*s,WCHAR sep,int*n){int c=*s?1:0;for(WCHAR*p=s;*p;p++)if(*p==sep)c++;WCHAR**a=malloc(sizeof(WCHAR*)*(c+1));int k=0;if(*s){a[k++]=s;for(WCHAR*p=s;*p;p++)if(*p==sep){*p=0;a[k++]=p+1;}}*n=k;return a;}

static void loadDb(void){
  HRSRC r=FindResourceW(0,MAKEINTRESOURCEW(101),(LPCWSTR)RT_RCDATA);DWORD sz=SizeofResource(0,r);const char*d=LockResource(LoadResource(0,r));
  WCHAR*all=u8(d,(int)sz);int nr;WCHAR**recs=splitw(all,0x1e,&nr);IT=calloc(nr,sizeof(Item));NIT=nr;
  for(int i=0;i<nr;i++){int nf;WCHAR**f=splitw(recs[i],0x1f,&nf);Item*a=&IT[i];WCHAR*e=L"";
    a->t=nf>0?f[0]:e;a->code=nf>1?f[1]:e;a->num=nf>2?f[2]:e;a->stars=nf>3?f[3]:e;a->jur=nf>4?f[4]:e;a->title=nf>5?f[5]:e;a->pun=nf>6?f[6]:e;a->note=nf>7?f[7]:e;a->full=nf>8?f[8]:e;a->kw=nf>9?f[9]:e;a->doc=nf>10?f[10]:e;
    a->arts=splitw(nf>11?f[11]:e,0x1d,&a->na);a->say=splitw(nf>12?f[12]:e,0x1d,&a->ns);a->steps=splitw(nf>13?f[13]:e,0x1d,&a->nst);a->warn=splitw(nf>14?f[14]:e,0x1d,&a->nw);
    a->sit=wcscmp(a->t,L"sit")==0;a->tL=lowdup(a->title);a->fL=lowdup(a->full);a->dL=lowdup(a->doc);
    WCHAR*kwc=lowdup(a->kw);int nk;WCHAR**ks=splitw(kwc,L'|',&nk);a->kws=malloc(sizeof(WCHAR*)*(nk+1));a->kwWords=malloc(sizeof(int)*(nk+1));a->nk=0;
    for(int j=0;j<nk;j++){WCHAR*k=ks[j];while(*k==L' ')k++;int L=(int)wcslen(k);while(L&&k[L-1]==L' ')L--;if(!L)continue;
      int words=1;for(int x=0;x<L;x++)if(k[x]==L' ')words++;int keep=L;if(words==1&&L>6)keep=L-2;
      WCHAR*s=malloc((keep+2)*sizeof(WCHAR));s[0]=L' ';wcsncpy(s+1,k,keep);s[keep+1]=0;a->kws[a->nk]=s;a->kwWords[a->nk]=words;a->nk++;}}}
static Item*byNum(const WCHAR*n){for(int i=0;i<NIT;i++)if(!IT[i].sit&&wcscmp(IT[i].num,n)==0)return &IT[i];return 0;}

/* ---------------- UI состояние ---------------- */
enum{ID_Q=10,ID_MINUS,ID_PLUS,ID_FONT,ID_GEAR,ID_TOP,ID_MINI,ID_TSEARCH,ID_TCONS,ID_LIST,ID_DET,ID_RANK,ID_NAME,ID_LIM,ID_COPYALL,ID_CHAT,ID_CHIP0=100,ID_OPT0=300};
static HWND hMain,hQ,hMinus,hPlus,hFontB,hGear,hTop,hMini,hTS,hTC,hHint,hList,hDet,hRank,hName,hLim,hCopyAll,hChat,hCons;
static HWND chips[20];static int nchips;
static HFONT fnt,fntS,fntB,fntC;static HBRUSH brBg,brPanel;
static COLORREF cBg=RGB(15,23,41),cPanel=RGB(24,35,56),cTxt=RGB(236,240,246),cGold=RGB(255,210,110),cMuted=RGB(150,165,190),cSel=RGB(52,78,122),cOn=RGB(40,110,70),cChip=RGB(44,68,102),cBlue=RGB(143,211,255),cWarn=RGB(255,180,160),cOk=RGB(159,240,191),cNo=RGB(255,180,180);
static int fsize=15,fidx=0,dpi=96,topOn=1,mini=0,cons=0,gear=0,fullH=0;
static const WCHAR*FONTS[]={L"Segoe UI",L"Tahoma",L"Verdana",L"Arial",L"Calibri"};
static WCHAR ini[MAX_PATH];
static int shown[3000],nshown=0,grp=0;
static const WCHAR*HINT=L"Что случилось? · стрелки — листать · клик по фразе — копировать · Tab — задержание";
static int S(int x){return MulDiv(x,dpi,96);}

/* группы-фильтры */
static const WCHAR*GRPN[]={L"Всё",L"Ситуации",L"УК",L"АК",L"ПК",L"ДК",L"ДУ",L"УВС",L"КУ",L"Закон о НГ",L"Правила",L"Прочее"};
#define NGRP 12
static int inGrp(Item*a){const WCHAR*c=a->code;
  switch(grp){case 0:return 1;case 1:return a->sit;case 2:return !wcscmp(c,L"УК");case 3:return !wcscmp(c,L"АК");case 4:return !wcscmp(c,L"ПК");case 5:return !wcscmp(c,L"ДК");case 6:return !wcscmp(c,L"ДУ");case 7:return !wcscmp(c,L"УВС");case 8:return !wcscmp(c,L"КУ");
  case 9:return !wcscmp(c,L"НГ")||!wcscmp(c,L"Прил1");
  case 10:{const WCHAR*R[]={L"ПГО",L"Поставки",L"Кайо",L"Форт",L"Зоны",L"Банки",L"Бизнесы",L"ОиП",L"ВП",L"Общие",L"AirDrop",L"График"};for(int i=0;i<12;i++)if(!wcscmp(c,R[i]))return 1;return 0;}
  case 11:{const WCHAR*R[]={L"Конст",L"ГК",L"ТК",L"ЭК"};for(int i=0;i<4;i++)if(!wcscmp(c,R[i]))return 1;return 0;}}return 1;}

/* синонимы */
static const WCHAR*SYN[][2]={{L"стрел",L"огонь выстрел стрельб"},{L"шмаля",L"стрел огонь"},{L"пальн",L"стрел огонь"},{L"удар",L"насили напал бьет"},{L"бьет",L"удар насили"},{L"избил",L"удар насили побои"},
 {L"тачк",L"авто транспорт машин"},{L"машин",L"авто транспорт"},{L"угна",L"угон завладен"},{L"ствол",L"оружие"},{L"пушк",L"оружие"},{L"нарк",L"наркот"},{L"трав",L"наркот куст"},{L"бронь",L"бронежилет"},{L"броник",L"бронежилет"},
 {L"маск",L"маска лицо"},{L"мент",L"полиц"},{L"коп",L"полиц"},{L"гос",L"государствен"},{L"фз",L"форт"},{L"форт",L"занкудо"},{L"остров",L"кайо перико"},{L"кайо",L"остров"},{L"матер",L"мат оскорб"},{L"послал",L"оскорб"},{L"обзыва",L"оскорб"},
 {L"убег",L"скрыл уклон"},{L"сбеж",L"скрыл уклон"},{L"документ",L"удостовер паспорт"},{L"увал",L"увольнит"},{L"лиценз",L"лицензия оружие"},{L"серийн",L"серийник гос ствол"},{L"мусор",L"полиц"},{L"фэбээр",L"fib"},{L"фбр",L"fib"},{L"фибов",L"fib"},{L"лспд",L"lspd полиц"},{L"шериф",L"lscsd"},{L"вертушк",L"вертолет"},{L"тазер",L"шокер спецсредств"},{L"гоп",L"грабеж"},{L"барыг",L"сбыт наркот"},{L"закладк",L"сбыт наркот"},{L"мочкан",L"убил"},{L"завалил",L"убил"},{L"грохнул",L"убил"},{L"заковал",L"наручник"},{L"браслет",L"наручник"},{L"ксива",L"удостовер"},{L"корочк",L"удостовер"},{L"личк",L"личный транспорт"},{L"хаммер",L"военн машин"},{L"броневик",L"военн машин"},{L"база",L"форт"},{L"взлетк",L"впп"},{L"рация",L"рации доклад"},{L"афк",L"бездейств"},{L"самовол",L"самовольн"},{L"кпз",L"арест"},{L"ордер",L"обыск"},{L"наркота",L"наркот"}};

/* ---------------- опечатки ---------------- */
static WCHAR**VOC;static int NVOC;
static int vocHas(WCHAR**arr,int n,const WCHAR*w){for(int i=0;i<n;i++)if(!wcscmp(arr[i],w))return 1;return 0;}
static void buildVoc(void){VOC=malloc(sizeof(WCHAR*)*60000);NVOC=0;
  for(int i=0;i<NIT;i++){Item*a=&IT[i];
    for(int j=0;j<a->nk;j++){WCHAR tmp[128];wcsncpy(tmp,a->kws[j]+1,127);tmp[127]=0;WCHAR*p=tmp;while(*p){while(*p==L' ')p++;WCHAR*b=p;while(*p&&*p!=L' ')p++;WCHAR c=*p;*p=0;if(wcslen(b)>=4&&NVOC<60000&&!vocHas(VOC,NVOC,b))VOC[NVOC++]=_wcsdup(b);if(c)p++;}}
    WCHAR*t=a->tL;while(*t){while(*t&&!((*t>=L'а'&&*t<=L'я')||(*t>=L'a'&&*t<=L'z')))t++;WCHAR*b=t;while((*t>=L'а'&&*t<=L'я')||(*t>=L'a'&&*t<=L'z'))t++;int L=(int)(t-b);if(L>=4&&L<60&&NVOC<60000){WCHAR w[64];wcsncpy(w,b,L);w[L]=0;if(!vocHas(VOC,NVOC,w))VOC[NVOC++]=_wcsdup(w);}}}}
static int lev(const WCHAR*a,int m,const WCHAR*b,int n,int max){if(abs(m-n)>max)return max+1;int p[80],c[80];if(n>78)n=78;for(int j=0;j<=n;j++)p[j]=j;
  for(int i=1;i<=m;i++){c[0]=i;int mn=i;for(int j=1;j<=n;j++){int v=p[j]+1;if(c[j-1]+1<v)v=c[j-1]+1;int d=p[j-1]+(a[i-1]==b[j-1]?0:1);if(d<v)v=d;c[j]=v;if(v<mn)mn=v;}if(mn>max)return max+1;for(int j=0;j<=n;j++)p[j]=c[j];}return p[n];}
static int fixTypos(WCHAR*s){if(!VOC)buildVoc();int changed=0;WCHAR out[512]=L"";WCHAR*p=s;
  while(*p){while(*p==L' ')p++;WCHAR*b=p;while(*p&&*p!=L' ')p++;int L=(int)(p-b);if(!L)break;WCHAR t[64];if(L>63)L=63;wcsncpy(t,b,L);t[L]=0;
    int dig=0;for(int i=0;i<L;i++)if(t[i]>=L'0'&&t[i]<=L'9')dig=1;
    if(L>=4&&!dig){int ok=0;for(int i=0;i<NVOC&&!ok;i++){int vl=(int)wcslen(VOC[i]);if(!wcsncmp(VOC[i],t,L)||(vl<=L&&!wcsncmp(t,VOC[i],vl)))ok=1;}
      if(!ok){int max=L>=8?2:1,bd=max+1;WCHAR*best=0;for(int i=0;i<NVOC;i++){int vl=(int)wcslen(VOC[i]);int cl=vl>L+1?L:vl;int d=lev(t,L,VOC[i],cl,max);if(d<bd){bd=d;best=VOC[i];if(!d)break;}}
        if(best&&bd<=max){wcsncpy(t,best,63);t[63]=0;changed=1;}}}
    if(*out)wcscat(out,L" ");wcsncat(out,t,511-wcslen(out));}
  if(changed)wcscpy(s,out);return changed;}

/* ---------------- поиск ---------------- */
static int score[3000];
static void doSearch(void){
  WCHAR raw0[256];GetWindowTextW(hQ,raw0,256);WCHAR raw[512];int k0=0;
  for(int i=0;raw0[i]&&k0<500;i++){raw[k0++]=raw0[i];WCHAR a=raw0[i],b=raw0[i+1];int ad=a>=L'0'&&a<=L'9',bd=b>=L'0'&&b<=L'9';if(b&&b!=L' '&&a!=L' '&&a!=L'.'&&b!=L'.'&&ad!=bd)raw[k0++]=L' ';}raw[k0]=0;
  WCHAR*r=raw;while(*r==L' ')r++;int empty=!*r;
  if(!empty&&!(r[0]>=L'0'&&r[0]<=L'9')){WCHAR lw[512];wcscpy(lw,r);lowerw(lw);if(fixTypos(lw)){static WCHAR last[512];if(wcscmp(last,lw)){wcscpy(last,lw);WCHAR m[600];swprintf(m,600,L"Исправлено: %ls",lw);SetWindowTextW(hHint,m);SetTimer(hMain,1,2200,0);}wcscpy(raw,lw);r=raw;}}
  WCHAR qo[300];swprintf(qo,300,L" %ls ",r);lowerw(qo);
  WCHAR q[1200];wcscpy(q,qo);for(size_t i=0;i<sizeof(SYN)/sizeof(SYN[0]);i++)if(wcsstr(qo,SYN[i][0])){wcscat(q,L" ");wcscat(q,SYN[i][1]);}wcscat(q,L" ");
  WCHAR qn[256];int k=0;for(WCHAR*p=r;*p&&k<255;p++)if(*p!=L' ')qn[k++]=*p;qn[k]=0;lowerw(qn);
  /* токены */
  WCHAR tok[40][64];int nt=0;{WCHAR*p=qo;while(*p&&nt<40){while(*p==L' ')p++;int j=0;while(*p&&*p!=L' '&&j<63)tok[nt][j++]=*p++;tok[nt][j]=0;if(j)nt++;}}
  WCHAR qw[80][64];int nqw=0;{WCHAR*p=q;while(*p&&nqw<80){while(*p==L' ')p++;int j=0;while(*p&&*p!=L' '&&j<63)qw[nqw][j++]=*p++;qw[nqw][j]=0;if(j)nqw++;}}
  int top=0;
  for(int i=0;i<NIT;i++){Item*a=&IT[i];int s=0;
    if(!inGrp(a)){score[i]=0;continue;}
    if(empty){score[i]=(grp==0?a->sit:1);continue;}
    /* номер */
    if(qn[0]>=L'0'&&qn[0]<=L'9'){const WCHAR*sp=wcschr(a->num,L' ');if(sp){WCHAR nn[64];int j=0;for(const WCHAR*p=sp+1;*p&&j<63;p++)if(*p!=L' ')nn[j++]=*p;nn[j]=0;lowerw(nn);if(!wcsncmp(nn,qn,wcslen(qn)))s+=50;}}
    /* слова в заголовке / тексте */
    for(int w=0;w<nqw;w++){WCHAR*x=qw[w];int L=(int)wcslen(x);if(L<=2)continue;WCHAR pat[70];swprintf(pat,70,L" %ls ",x);int orig=wcsstr(qo,pat)!=0;
      WCHAR st[64];wcscpy(st,x);if(L>6)st[L-2]=0;int sl=(int)wcslen(st);
      if(!orig){if(wcsstr(a->tL,st))s+=2;continue;}
      if(wcsstr(a->tL,st))s+=a->sit?3:5;else if(sl>3&&wcsstr(a->fL,st))s+=1;
      if(sl>3&&wcsstr(a->dL,st))s+=4;}
    /* ключи */
    {int hit[40]={0},multi=0,exp=0;
     for(int j=0;j<a->nk;j++){WCHAR*kk=a->kws[j];
       if(a->kwWords[j]>1){if(wcsstr(qo,kk)){multi++;WCHAR tmp[128];wcsncpy(tmp,kk+1,127);tmp[127]=0;WCHAR*p=tmp;while(*p){while(*p==L' ')p++;WCHAR*b=p;while(*p&&*p!=L' ')p++;WCHAR c=*p;*p=0;int bl=(int)wcslen(b);if(bl)for(int t=0;t<nt;t++)if(wcslen(tok[t])>2&&!wcsncmp(tok[t],b,bl))hit[t]=1;if(c){*p=c;}}}}
       else{int f=0,bl=(int)wcslen(kk+1);for(int t=0;t<nt;t++)if(wcslen(tok[t])>2&&!wcsncmp(tok[t],kk+1,bl)){hit[t]=1;f=1;}if(!f&&wcsstr(q,kk))exp++;}}
     int hs=0;for(int t=0;t<nt;t++)hs+=hit[t];int base=a->sit?6:((!wcscmp(a->code,L"УК")||!wcscmp(a->code,L"АК"))?5:3);s+=hs*base+multi*3+(exp>2?2:exp)*(a->sit?2:1);}
    score[i]=s;if(s>top)top=s;}
  int minsc=empty?1:(top/3>1?top/3:1);int lim=grp==0?60:2000;
  nshown=0;SendMessageW(hList,WM_SETREDRAW,FALSE,0);SendMessageW(hList,LB_RESETCONTENT,0,0);
  while(nshown<lim){int best=-1,bs=minsc-1;for(int i=0;i<NIT;i++)if(score[i]>bs){bs=score[i];best=i;}if(best<0)break;shown[nshown++]=best;score[best]=0;SendMessageW(hList,LB_ADDSTRING,0,(LPARAM)L"");}
  SendMessageW(hList,WM_SETREDRAW,TRUE,0);InvalidateRect(hList,0,TRUE);
  void showDetail(int);void showMsg(const WCHAR*);
  if(nshown){SendMessageW(hList,LB_SETCURSEL,0,0);showDetail(shown[0]);}else showMsg(L"Ничего не нашёл. Опиши по-другому или введи номер статьи.");}

/* ---------------- rich text ---------------- */
static int sayStart[40],sayEnd[40],nsay=0;static WCHAR*sayTxt[40];
static WCHAR*speech[40];static int nspeech=0;
static void me(const WCHAR*src,WCHAR*dst,int max){WCHAR r[64],n[64];GetWindowTextW(hRank,r,64);GetWindowTextW(hName,n,64);
  const WCHAR*p=src;int k=0;while(*p&&k<max-1){if(!wcsncmp(p,L"[звание]",8)&&*r){for(WCHAR*x=r;*x&&k<max-1;)dst[k++]=*x++;p+=8;}else if(!wcsncmp(p,L"[фамилия]",9)&&*n){for(WCHAR*x=n;*x&&k<max-1;)dst[k++]=*x++;p+=9;}else dst[k++]=*p++;}dst[k]=0;}
static void reAdd(const WCHAR*t,COLORREF c,int bold,int ital,int dtw){
  CHARFORMAT2W cf={0};cf.cbSize=sizeof(cf);cf.dwMask=CFM_COLOR|CFM_BOLD|CFM_ITALIC|CFM_FACE|CFM_SIZE;cf.crTextColor=c;cf.dwEffects=(bold?CFE_BOLD:0)|(ital?CFE_ITALIC:0);cf.yHeight=fsize*15+dtw;wcscpy(cf.szFaceName,FONTS[fidx]);
  int n=GetWindowTextLengthW(hDet);SendMessageW(hDet,EM_SETSEL,n,n);SendMessageW(hDet,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf);SendMessageW(hDet,EM_REPLACESEL,FALSE,(LPARAM)t);}
static int reLen(void){GETTEXTLENGTHEX g={GTL_NUMCHARS|GTL_PRECISE,1200};return (int)SendMessageW(hDet,EM_GETTEXTLENGTHEX,(WPARAM)&g,0);}
static void reBegin(void){SendMessageW(hDet,WM_SETREDRAW,FALSE,0);SetWindowTextW(hDet,L"");nsay=0;for(int i=0;i<nspeech;i++)free(speech[i]);nspeech=0;}
static void reEnd(void){PARAFORMAT2 pf={0};pf.cbSize=sizeof(pf);pf.dwMask=PFM_LINESPACING|PFM_SPACEAFTER;pf.bLineSpacingRule=5;pf.dyLineSpacing=23;pf.dySpaceAfter=30;
  SendMessageW(hDet,EM_SETSEL,0,-1);SendMessageW(hDet,EM_SETPARAFORMAT,0,(LPARAM)&pf);SendMessageW(hDet,EM_SETSEL,0,0);SendMessageW(hDet,WM_SETREDRAW,TRUE,0);InvalidateRect(hDet,0,TRUE);
  ShowWindow(hCopyAll,nspeech?SW_SHOW:SW_HIDE);ShowWindow(hChat,nspeech?SW_SHOW:SW_HIDE);SetWindowTextW(hChat,L"В чат по частям");}
static void lab(const WCHAR*t){reAdd(L"\r",cTxt,0,0,-80);reAdd(t,cMuted,1,0,-45);reAdd(L"\r",cTxt,0,0,-60);}
static void sayLine(const WCHAR*s){WCHAR b[2048];me(s,b,2048);speech[nspeech++]=_wcsdup(b);WCHAR q[2100];swprintf(q,2100,L"«%ls»",b);
  int st=reLen();reAdd(L"  › ",cGold,1,0,0);reAdd(q,cTxt,0,0,0);if(nsay<40){sayStart[nsay]=st;sayEnd[nsay]=reLen();sayTxt[nsay]=speech[nspeech-1];nsay++;}reAdd(L"\r",cTxt,0,0,0);}
static void artLine(Item*b){WCHAR h[256];swprintf(h,256,L"%ls%ls%ls",b->num,*b->stars?L"  ":L"",b->stars);reAdd(h,cGold,1,0,0);
  reAdd(L" — ",cTxt,0,0,0);reAdd((wcslen(b->full)&&wcslen(b->full)<=320)?b->full:b->title,cTxt,0,0,0);reAdd(L"\r",cTxt,0,0,0);if(*b->pun){reAdd(b->pun,cGold,1,0,0);reAdd(L"\r",cTxt,0,0,0);}}
static void autoScript(Item*a,const WCHAR***say,int*ns,const WCHAR***st,int*nst){
  static WCHAR l4[1024],l0[1024],d0[512];static const WCHAR*S1[8],*T1[8];*ns=*nst=0;
  WCHAR sh[512];wcsncpy(sh,a->title,511);sh[511]=0;WCHAR*cut=wcsstr(sh,L" — ");if(cut)*cut=0;cut=wcsstr(sh,L": ");if(cut)*cut=0;cut=wcsstr(sh,L", то есть ");if(cut)*cut=0;if(sh[0])CharLowerBuffW(sh,1);
  const WCHAR*n=a->num+3;
  if(!wcscmp(a->code,L"УК")){int mil=wcsstr(a->jur,L"Военн")!=0;WCHAR nn[64];wcsncpy(nn,n,63);nn[63]=0;WCHAR*pp=wcsstr(nn,L" ч.");WCHAR nv[80];if(pp){*pp=0;swprintf(nv,80,L"%ls часть %ls",nn,pp+3);}else wcscpy(nv,nn);
    swprintf(l4,1024,L"Вы задержаны по статье %ls Уголовного кодекса — %ls.",nv,sh);
    S1[0]=L"Национальная гвардия! Стоять, руки вверх!";S1[1]=L"Вы задержаны. Руки за спину.";S1[2]=L"Национальная гвардия штата San-Andreas, [звание] [фамилия]. Моё удостоверение.";S1[3]=l4;S1[4]=L"Сейчас будет проведён первичный обыск.";S1[5]=L"Вы будете переданы сотрудникам правоохранительных органов.";*ns=6;
    if(mil){T1[0]=L"Военная статья — ваша компетенция (ПК 36)";T1[1]=L"Задерживаете сами: наручники → удостоверение → статья → Миранда (ПК 17)";T1[2]=L"Военнослужащий — вызвать MP и ОГП (ПК 19, ПГО 1.14)";T1[3]=L"Видео с начала";}
    else{T1[0]=L"Задерживать только на Форте/около, на Кайо-Перико или если нарушение против SANG (ПГО 7.3)";T1[1]=L"Наручники (от Сержанта) → удостоверение → статья вслух → первичный обыск";T1[2]=L"Передать FIB / LSPD / LSCSD";T1[3]=L"Видео с начала";}*nst=4;}
  else if(!wcscmp(a->code,L"АК")){swprintf(l0,1024,L"Прекратите. Это нарушение статьи %ls Административного кодекса — %ls.",n,sh);S1[0]=l0;S1[1]=L"Выполните законное требование, иначе будете задержаны по статье 17.6 Уголовного кодекса.";*ns=2;
    T1[0]=L"Штраф SANG не выписывает (АК 2.2) — только требование прекратить";T1[1]=L"Не выполнил — УК 17.6, задержание и передача";T1[2]=L"Или вызвать LSPD / LSCSD";*nst=3;}
  else if(!wcscmp(a->code,L"ДУ")){swprintf(d0,512,L"Боец, [фамилия], прекратить. Это нарушение пункта %ls Дисциплинарного устава.",n);S1[0]=d0;*ns=1;
    T1[0]=L"Свой отдел — взыскание сам (ДУ 1.4, от Майора)";T1[1]=L"Чужой отдел — его командиру или MP (ДУ 4.40)";T1[2]=L"Не выполнил приказ — ДУ 4.10";T1[3]=L"Наручники за нарушение устава — нельзя (закон о НГ 25 ч.4)";*nst=4;}
  *say=S1;*st=T1;}
void showMsg(const WCHAR*t){reBegin();reAdd(t,cMuted,0,0,0);reEnd();}
void showDetail(int i){Item*a=&IT[i];reBegin();WCHAR h[512];
  if(a->sit){swprintf(h,512,L"• %ls",a->title);reAdd(h,cGold,1,0,50);reAdd(L"\r",cTxt,0,0,0);
    lab(L"СТАТЬИ");for(int k=0;k<a->na;k++){Item*b=byNum(a->arts[k]);if(b)artLine(b);}
    lab(L"ГОВОРИТЬ");for(int k=0;k<a->ns;k++)sayLine(a->say[k]);
    if(a->nst){lab(L"ДЕЙСТВИЯ");for(int k=0;k<a->nst;k++){swprintf(h,512,L"%d. ",k+1);reAdd(h,cGold,1,0,0);reAdd(a->steps[k],cTxt,0,0,0);reAdd(L"\r",cTxt,0,0,0);}}
    if(a->nw){lab(L"ВАЖНО");for(int k=0;k<a->nw;k++){reAdd(a->warn[k],cWarn,0,0,0);reAdd(L"\r",cTxt,0,0,0);}}}
  else{swprintf(h,512,L"%ls%ls%ls",a->num,*a->stars?L"   ":L"",a->stars);reAdd(h,cGold,1,0,50);reAdd(L"\r",cTxt,0,0,0);
    if(*a->doc){reAdd(a->doc,cMuted,0,0,-30);reAdd(L"\r",cTxt,0,0,0);}if(*a->jur){reAdd(a->jur,cMuted,0,0,-30);reAdd(L"\r",cTxt,0,0,0);}
    reAdd(L"\r",cTxt,0,0,-80);reAdd(a->title,cTxt,0,0,0);reAdd(L"\r",cTxt,0,0,0);if(*a->pun){reAdd(a->pun,cGold,1,0,0);reAdd(L"\r",cTxt,0,0,0);}
    const WCHAR**sy;const WCHAR**stp;int ns,nst;autoScript(a,&sy,&ns,&stp,&nst);
    if(ns){lab(L"ГОВОРИТЬ");for(int k=0;k<ns;k++)sayLine(sy[k]);}
    if(nst){lab(L"ДЕЙСТВИЯ");for(int k=0;k<nst;k++){swprintf(h,512,L"%d. ",k+1);reAdd(h,cGold,1,0,0);reAdd(stp[k],cTxt,0,0,0);reAdd(L"\r",cTxt,0,0,0);}}
    if(*a->note){lab(L"ВАЖНО");reAdd(a->note,cWarn,0,0,0);reAdd(L"\r",cTxt,0,0,0);}
    if(*a->full){lab((!wcscmp(a->code,L"УК")||!wcscmp(a->code,L"АК"))?L"ПОЛНЫЙ ТЕКСТ (ЕСЛИ ПРОСИТ РАСШИФРОВАТЬ)":L"ПОЛНЫЙ ТЕКСТ");WCHAR*t=_wcsdup(a->full);for(WCHAR*p=t;*p;p++)if(*p==L'\n')*p=L'\r';reAdd(t,cTxt,0,1,-10);free(t);}}
  reEnd();}

/* ---------------- буфер обмена / чат ---------------- */
static void clip(const WCHAR*t){if(!OpenClipboard(hMain))return;EmptyClipboard();size_t n=(wcslen(t)+1)*sizeof(WCHAR);HGLOBAL g=GlobalAlloc(GMEM_MOVEABLE,n);memcpy(GlobalLock(g),t,n);GlobalUnlock(g);SetClipboardData(CF_UNICODETEXT,g);CloseClipboard();}
static void flash(const WCHAR*t){SetWindowTextW(hHint,t);SetTimer(hMain,1,2200,0);}
static WCHAR*chatParts[60];static int nchat=0,chatI=0;static WCHAR chatKey[4096];
static int chatLimit(void){WCHAR b[16];GetWindowTextW(hLim,b,16);int v=_wtoi(b);return (v>=40&&v<=500)?v:120;}
static void joinSpeech(WCHAR*out,int max){out[0]=0;for(int i=0;i<nspeech;i++){if(i)wcsncat(out,L" ",max-wcslen(out)-1);wcsncat(out,speech[i],max-wcslen(out)-1);}}
static void chatCopy(void){WCHAR all[4096];joinSpeech(all,4096);WCHAR key[4200];swprintf(key,4200,L"%ls|%d",all,chatLimit());
  if(wcscmp(key,chatKey)){for(int i=0;i<nchat;i++)free(chatParts[i]);nchat=0;chatI=0;wcsncpy(chatKey,key,4095);int lim=chatLimit();WCHAR cur[600]=L"";WCHAR*p=all;
    while(*p){while(*p==L' ')p++;WCHAR*b=p;while(*p&&*p!=L' ')p++;int wl=(int)(p-b);if(!wl)break;int cl=(int)wcslen(cur);
      if(cl&&cl+1+wl>lim){chatParts[nchat++]=_wcsdup(cur);cur[0]=0;cl=0;}if(cl){wcscat(cur,L" ");}wcsncat(cur,b,wl);if(nchat>=58)break;}
    if(cur[0])chatParts[nchat++]=_wcsdup(cur);}
  if(!nchat)return;clip(chatParts[chatI]);chatI++;WCHAR m[128];swprintf(m,128,L"В буфере часть %d из %d — вставь в чат (Ctrl+V)",chatI,nchat);flash(m);
  if(chatI>=nchat){SetWindowTextW(hChat,L"В чат — готово (заново)");chatI=0;}else{swprintf(m,128,L"В чат — часть %d/%d",chatI+1,nchat);SetWindowTextW(hChat,m);}}

/* ---------------- конструктор ---------------- */
typedef struct{const WCHAR*id,*label;}Opt;
static const Opt PL[]={{L"kayo",L"Кайо-Перико"},{L"air",L"Аэропорт Кайо"},{L"fort",L"Форт-Занкудо"},{L"carrier",L"Авианосец"},{L"white",L"Белая зона у Форта"},{L"blaine",L"Округ Блейн"},{L"city",L"Город Лос-Сантос"}};
static const Opt WH[]={{L"civ",L"Гражданский"},{L"gov",L"Госслужащий"},{L"sang",L"Военный SANG"}};
typedef struct{const WCHAR*id,*label,*art,*phr;}Fact;
static const Fact FA[]={{L"shot",L"Стрелял в военных",L"УК 17.1",L"посягательство на жизнь военнослужащего при исполнении"},{L"hit",L"Ударил / угрожал военному",L"УК 17.2",L"применение насилия в отношении военнослужащего при исполнении"},
 {L"insult",L"Оскорбил военного",L"УК 17.3",L"оскорбление представителя власти при исполнении"},{L"disobey",L"Не выполнил требование",L"УК 17.6",L"неповиновение законному требованию"},{L"run",L"Убегал / скрывался",L"УК 16.12",L"уклонение от задержания"},
 {L"grow",L"Выращивал / поливал",L"УК 13.1",L"незаконное кустарное производство наркотических веществ"},{L"steal",L"Угнал транспорт",L"УК 10.5",L"неправомерное завладение транспортным средством"},
 {L"stealgov",L"Угнал гос. транспорт",L"УК 10.5.1",L"неправомерное завладение государственным транспортным средством"},{L"rob",L"Грабёж / разбой с оружием",L"УК 10.4",L"разбой"},{L"hostage",L"Держал заложника",L"УК 7.1",L"похищение человека"}};
static const Fact MI[]={{L"m181",L"Не выполнил приказ (вред службе)",L"УК 18.1",L"неисполнение приказа начальника"},{L"m182",L"Самоволка / неявка",L"УК 18.2",L"самовольное оставление места службы"},{L"m183",L"Дезертирство",L"УК 18.3",L"дезертирство"}};
static const Opt WP[]={{L"none",L"Нет оружия"},{L"lic",L"С лицензией"},{L"nolic",L"Без лицензии"},{L"fake",L"Лицензия поддельная"},{L"gov",L"Гос. образца (серийник)"}};
static const Opt WX[]={{L"incar",L"Было в машине"},{L"drawn",L"Доставал на людях"},{L"govgear",L"Броня / спецсредства гос. образца"},{L"bomb",L"Взрывчатка"}};
static const Opt DR[]={{L"none",L"Нет"},{L"lt3",L"Меньше 3 г"},{L"ge3",L"От 3 г"},{L"gt25",L"Свыше 25 г / сбыт"}};
static const Opt FL[]={{L"event",L"Идёт нападение (мероприятие)"},{L"vp",L"ЧС / военное положение"},{L"access",L"Есть доступ / пропуск"},{L"request",L"Выехали по запросу LSPD/FIB"},{L"duty",L"Госслужащий при исполнении"}};
enum{G_PL,G_WH,G_FA,G_MI,G_WP,G_WX,G_DR,G_FL,NG};
static const WCHAR*GLAB[]={L"ГДЕ",L"КТО",L"ЧТО СДЕЛАЛ",L"",L"ОРУЖИЕ",L"",L"НАРКОТИКИ ПРИ СЕБЕ",L"ОБСТАНОВКА"};
static int cPlace=0,cWho=0,cWeap=0,cDrug=0;static int cFa[10],cMi[3],cWx[4],cFl[5]={0,0,0,0,1};
typedef struct{HWND h;int g,i;}OptBtn;static OptBtn OB[60];static int nob=0;static HWND glabs[NG];static int consScroll=0,consH=0;

static int optOn(int g,int i){switch(g){case G_PL:return cPlace==i;case G_WH:return cWho==i;case G_FA:return cFa[i];case G_MI:return cMi[i];case G_WP:return cWeap==i;case G_WX:return cWx[i];case G_DR:return cDrug==i;case G_FL:return cFl[i];}return 0;}
static const WCHAR*optLabel(int g,int i){switch(g){case G_PL:return PL[i].label;case G_WH:return WH[i].label;case G_FA:return FA[i].label;case G_MI:return MI[i].label;case G_WP:return WP[i].label;case G_WX:return WX[i].label;case G_DR:return DR[i].label;case G_FL:return FL[i].label;}return L"";}
static int gCount(int g){switch(g){case G_PL:return 7;case G_WH:return 3;case G_FA:return 10;case G_MI:return 3;case G_WP:return 5;case G_WX:return 4;case G_DR:return 4;case G_FL:return 5;}return 0;}

typedef struct{const WCHAR*n;WCHAR p[256];}CA;
static void consResult(void){
  CA arts[30];int na=0;WCHAR*warn[20];int nw=0;
  #define ADD(N,P) do{int _d=0;for(int _i=0;_i<na;_i++)if(!wcscmp(arts[_i].n,N))_d=1;if(!_d&&na<30){arts[na].n=N;wcsncpy(arts[na].p,P,255);na++;}}while(0)
  #define WARN(T) do{if(nw<20)warn[nw++]=(WCHAR*)(T);}while(0)
  const WCHAR*place=PL[cPlace].id;int isGov=cWho==1,isM=cWho==2,civ=cWho==0;int event=cFl[0],vp=cFl[1],access=cFl[2],request=cFl[3];
  int guarded=!wcscmp(place,L"kayo")||!wcscmp(place,L"fort")||!wcscmp(place,L"carrier");
  int govDutyKayo=isGov&&cFl[4]&&!wcscmp(place,L"kayo");
  if(guarded&&!access&&!govDutyKayo&&!(!wcscmp(place,L"kayo")&&event)){if(isGov)ADD(L"УК 12.7 ч.3",L"проникновение на особо охраняемый объект должностным лицом");else if(!isM)ADD(L"УК 12.7 ч.2",L"незаконное нахождение на особо охраняемом объекте");}
  for(int i=0;i<10;i++)if(cFa[i])ADD(FA[i].art,FA[i].phr);
  if(isM)for(int i=0;i<3;i++)if(cMi[i])ADD(MI[i].art,MI[i].phr);
  const WCHAR*wp=WP[cWeap].id;
  if(!wcscmp(wp,L"nolic")){ADD(L"УК 12.8 ч.2",L"незаконное ношение оружия");if(cWx[0])ADD(L"УК 12.8 ч.1",L"незаконная перевозка оружия в транспортном средстве");}
  if(!wcscmp(wp,L"fake")){ADD(L"УК 17.8",L"использование поддельной лицензии");ADD(L"УК 12.8 ч.2",L"незаконное ношение оружия");if(cWx[0])ADD(L"УК 12.8 ч.1",L"незаконная перевозка оружия в транспортном средстве");}
  if(!wcscmp(wp,L"gov")&&civ)ADD(L"УК 12.8.1",L"ношение гражданским оружия государственного образца");
  if(cWx[2]&&civ)ADD(L"УК 12.8.1",L"ношение гражданским спецсредств государственного образца");
  if(cWx[3])ADD(L"УК 12.9",L"незаконное ношение взрывчатки");
  if(cWx[1]&&cWeap){if(civ)ADD(L"АК 5.4 ч.1",L"обнажение оружия гражданским лицом");else ADD(L"АК 5.4 ч.2",L"обнажение служебного оружия без законных оснований");}
  if(cDrug==1)ADD(L"АК 8.1",L"незаконное хранение наркотических веществ менее 3 грамм");
  if(cDrug==2){if(isGov||isM)ADD(L"УК 13.4",L"наркотики у сотрудника государственной структуры");else ADD(L"УК 13.2",L"незаконное хранение наркотических средств");}
  if(cDrug==3){if(isGov||isM)ADD(L"УК 13.4",L"наркотики у сотрудника государственной структуры");else ADD(L"УК 13.2.1",L"хранение наркотических средств в крупном размере или с целью сбыта");}
  int against=cFa[0]||cFa[1]||cFa[2];int milCrime=isM&&(cMi[0]||cMi[1]||cMi[2]);
  int zone=!wcscmp(place,L"kayo")||!wcscmp(place,L"fort")||!wcscmp(place,L"white")||!wcscmp(place,L"carrier");
  int nuk=0;for(int i=0;i<na;i++)if(!wcsncmp(arts[i].n,L"УК",2))nuk++;
  WCHAR verdict[400];COLORREF vc=cOk;int can=0;
  if(!na){wcscpy(verdict,L"НЕТ ОСНОВАНИЙ — отметь, что он сделал.");vc=cNo;}
  else if(!nuk){wcscpy(verdict,L"ТОЛЬКО АК — задерживать нельзя, штраф выписывает полиция (АК 2.2). Требование прекратить, иначе 17.6.");vc=cGold;}
  else if(zone||against||milCrime||vp){can=1;swprintf(verdict,400,L"МОЖНО ЗАДЕРЖИВАТЬ%ls%ls",!wcscmp(place,L"carrier")?L" (Авианосец — дислокация НГ по закону, в ПГО 7.3 прямо не назван)":L"",
      zone?L"":against?L" — нарушение против SANG (ПГО 7.3)":milCrime?L" — военное преступление (ПГО 7.3, исключение)":L" — ЧС / ВП (ПГО 7.3, исключение)");}
  else{wcscpy(verdict,L"ЗАДЕРЖИВАТЬ НЕЛЬЗЯ: не зона SANG и нарушение не против вас (ПГО 7.3, Demorgan 50–90). Вызови LSPD / LSCSD / FIB и передай данные.");vc=cNo;}
  if(!wcscmp(place,L"kayo")&&event)WARN(L"Идёт нападение на Кайо-Перико: наручники и тепловизор запрещены (Кайо 2.1, Demorgan 60–90). Задержать причастных — только после нападения.");
  if(!wcscmp(place,L"air"))WARN(L"Аэропорт Кайо-Перико — открытая территория, 12.7 нет (Приложение №1 ст. 3).");
  if(!wcscmp(place,L"white"))WARN(L"Белая зона до КПП — свободна для граждан (ПГО 7.3). 12.7 нет; можно спросить цель и документы (Прил. №1 ст. 7). Не ушёл по требованию — 17.6.");
  if(!wcscmp(place,L"city")&&!request&&!cFa[4])WARN(L"В городе без основания (запрос, погоня, доставка задержанного) — АК 9.5, штраф 20.000$ (закон о НГ ст. 2).");
  if(!wcscmp(place,L"city"))WARN(L"Город — зона LSPD: вызвать их по /dep.");
  if(govDutyKayo)WARN(L"Госслужащий при исполнении имеет доступ на Кайо-Перико — 12.7 нет (Прил. №1 ст. 4 «ж»).");
  if(isGov&&cFl[4]&&(!wcscmp(place,L"fort")||!wcscmp(place,L"carrier")))WARN(L"На Форт и Авианосец госслужащим доступ только в указанных случаях (спецоперация по защите Форта, закреплённые ОГП, EMS на проверке, поставки с Авианосца — Прил. №1 ст. 4). Если так — отметь «Есть доступ».");
  if(isGov)WARN(L"Госслужащий: вызвать его руководство и ОГП, ждать до 15 мин (ПК 19). В КПЗ — только после решения прокурора (ПГО 1.14).");
  if(isM)WARN(L"Свой военный: наручники — только за преступление по УК (закон о НГ 25 ч.4). Нарушение устава — взыскание, не задержание. Вызвать MP.");
  if(cWeap==1)WARN(L"Лицензия есть — ношение законно, 12.8 нет. Проверь, что лицензия действующая и оружие не гос. образца.");
  if(cWeap)WARN(L"Спроси лицензию на оружие или коллекционера (КУ 3.12). Изъятие — только через функционал (ПГО 1.6), на видео.");
  if(cFa[0])WARN(L"Стреляют по вам — ответный огонь сразу, без предупреждения (закон о НГ 20.1). В толпе — нельзя (20.2).");
  if(!wcscmp(place,L"kayo")&&!event&&(cWeap||cFa[0]))WARN(L"На Кайо-Перико огонь по вооружённым — без предупреждения (Правила зон 2.3).");
  if(cFa[4])WARN(L"Не догнал — задержать позже без розыска нельзя (ПГО 8.7). Боло SANG не ставит — передай данные LSPD / FIB.");
  int cuffs=!(!wcscmp(place,L"kayo")&&event);
  reBegin();reAdd(verdict,vc,1,0,20);reAdd(L"\r",cTxt,0,0,0);
  if(na){int mx=0;for(int i=0;i<na;i++){Item*b=byNum(arts[i].n);if(b){int l=(int)wcslen(b->stars);if(l>mx)mx=l;}}
    WCHAR lb[64];if(mx){wcscpy(lb,L"СТАТЬИ · РОЗЫСК ДО ");for(int i=0;i<mx;i++)wcscat(lb,L"*");}else wcscpy(lb,L"СТАТЬИ");lab(lb);
    for(int i=0;i<na;i++){Item*b=byNum(arts[i].n);if(!b)continue;WCHAR h[256];swprintf(h,256,L"%ls%ls%ls",b->num,*b->stars?L"  ":L"",b->stars);reAdd(h,cGold,1,0,0);reAdd(L" — ",cTxt,0,0,0);reAdd(b->title,cTxt,0,0,0);reAdd(L"\r",cTxt,0,0,0);reAdd(b->pun,cGold,1,0,0);reAdd(L"\r",cTxt,0,0,0);}
    reAdd(L"Несколько статей — поглощение более строгим; при задержании можно сложить до 10 лет (УК 5.2).\r",cMuted,0,0,-30);}
  if(can){lab(L"ГОВОРИТЬ");sayLine(L"Национальная гвардия! Стоять! Руки вверх!");if(cuffs)sayLine(L"Вы задержаны. Руки за спину.");sayLine(L"Национальная гвардия штата San-Andreas, [звание] [фамилия]. Моё удостоверение.");
    WCHAR big[3000]=L"Вы задержаны по ";int first=1;for(int i=0;i<na;i++){if(wcsncmp(arts[i].n,L"УК",2))continue;WCHAR nn[64];wcsncpy(nn,arts[i].n+3,63);nn[63]=0;WCHAR*pp=wcsstr(nn,L" ч.");WCHAR nv[80];if(pp){*pp=0;swprintf(nv,80,L"%ls часть %ls",nn,pp+3);}else wcscpy(nv,nn);
      WCHAR part[400];swprintf(part,400,L"%lsстатье %ls Уголовного кодекса — %ls",first?L"":L", по ",nv,arts[i].p);wcsncat(big,part,2999-wcslen(big));first=0;}wcscat(big,L".");sayLine(big);
    sayLine(L"Сейчас будет проведён первичный обыск. Есть при себе оружие, колюще-режущие предметы, наркотики?");if(cWeap)sayLine(L"Предъявите лицензию на оружие или лицензию коллекционера.");
    sayLine(isM?L"Вы будете переданы Military Police.":L"Вы будете доставлены и переданы сотрудникам правоохранительных органов.");
    lab(L"ДЕЙСТВИЯ");const WCHAR*st[8];int ns=0;st[ns++]=L"Видео — с первого шага (ПК 32, ПГО 8.4)";if(cuffs)st[ns++]=L"Наручники — от Сержанта (ДУ 4.42)";st[ns++]=L"Удостоверение, статьи вслух полностью (ПГО 1.8)";st[ns++]=L"Первичный обыск, изъятие через функционал (ПГО 1.6)";
    if(isGov)st[ns++]=L"Вызвать руководство задержанного и ОГП (ПК 19)";st[ns++]=isM?L"Вызвать MP; военная статья — компетенция SANG (ПК 36)":L"Вызвать FIB / LSPD / LSCSD по /dep или везти самим (ПГО 7.3), не затягивать (ПГО 1.4)";st[ns++]=L"Час задержания идёт с наручников (ПК 20)";
    for(int i=0;i<ns;i++){WCHAR h[16];swprintf(h,16,L"%d. ",i+1);reAdd(h,cGold,1,0,0);reAdd(st[i],cTxt,0,0,0);reAdd(L"\r",cTxt,0,0,0);}}
  else if(na){lab(L"ГОВОРИТЬ");WCHAR big[3000]=L"Прекратите. Вы нарушаете ";for(int i=0;i<na;i++){WCHAR part[400];int ak=!wcsncmp(arts[i].n,L"АК",2);swprintf(part,400,L"%lsстатью %ls %ls — %ls",i?L", ":L"",arts[i].n+3,ak?L"Административного кодекса":L"Уголовного кодекса",arts[i].p);wcsncat(big,part,2999-wcslen(big));}wcscat(big,L".");sayLine(big);
    sayLine(L"Выполните законное требование, иначе будете задержаны по статье 17.6 Уголовного кодекса.");lab(L"ДЕЙСТВИЯ");reAdd(L"1. ",cGold,1,0,0);reAdd(L"Требование прекратить\r",cTxt,0,0,0);reAdd(L"2. ",cGold,1,0,0);reAdd(L"Вызвать LSPD / LSCSD / FIB и передать данные\r",cTxt,0,0,0);}
  if(nw){lab(L"ВАЖНО");for(int i=0;i<nw;i++){reAdd(warn[i],cWarn,0,0,0);reAdd(L"\r",cTxt,0,0,0);}}
  if(can){int any=0;for(int i=0;i<na;i++){Item*b=byNum(arts[i].n);if(b&&*b->full){if(!any){lab(L"ЕСЛИ ПРОСИТ РАСШИФРОВАТЬ");any=1;}reAdd(b->num,cGold,1,0,0);reAdd(L" — «",cTxt,0,1,0);WCHAR*t=_wcsdup(b->full);for(WCHAR*p=t;*p;p++)if(*p==L'\n'||*p==L'\r')*p=L' ';reAdd(t,cTxt,0,1,-10);free(t);reAdd(L"»\r",cTxt,0,1,0);}}}
  reEnd();}

/* ---------------- раскладка ---------------- */
static void layoutCons(void){RECT r;GetClientRect(hCons,&r);int W=r.right,x=S(6),y=S(4)-consScroll,rowH=S(20),g=S(3);HDC dc=GetDC(hCons);SelectObject(dc,fntC);HDWP dw=BeginDeferWindowPos(nob+NG);
  for(int G=0;G<NG;G++){if(G==G_MI&&cWho!=2){for(int i=0;i<nob;i++)if(OB[i].g==G)ShowWindow(OB[i].h,SW_HIDE);continue;}
    if(*GLAB[G]){if(x>S(6))y+=rowH+g;x=S(6);dw=DeferWindowPos(dw,glabs[G],0,x,y,W-2*x,S(14),SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);y+=S(15);}
    else if(G!=G_MI){y+=rowH+g;x=S(6);}
    for(int i=0;i<nob;i++){if(OB[i].g!=G)continue;SIZE sz;const WCHAR*t=optLabel(G,OB[i].i);GetTextExtentPoint32W(dc,t,(int)wcslen(t),&sz);int w=sz.cx+S(14);
      if(x+w>W-S(6)&&x>S(6)){x=S(6);y+=rowH+g;}dw=DeferWindowPos(dw,OB[i].h,0,x,y,w,rowH,SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);x+=w+g;}}
  EndDeferWindowPos(dw);
  ReleaseDC(hCons,dc);consH=y+rowH+S(8)+consScroll;RedrawWindow(hCons,0,0,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN|RDW_UPDATENOW);
  SCROLLINFO si={sizeof(si),SIF_RANGE|SIF_PAGE|SIF_POS,0,consH,(UINT)r.bottom,consScroll};SetScrollInfo(hCons,SB_VERT,&si,TRUE);}
static void layout(void){RECT r;GetClientRect(hMain,&r);int W=r.right,H=r.bottom,m=S(8),g=S(4);int eh=S(fsize+14),bw=S(30);
  int topW=S(70);MoveWindow(hQ,m,m,W-2*m-5*(bw+g)-topW-g,eh,TRUE);int x=W-m-topW-5*(bw+g)+g;
  MoveWindow(hMinus,x,m,bw,eh,TRUE);x+=bw+g;MoveWindow(hPlus,x,m,bw,eh,TRUE);x+=bw+g;MoveWindow(hFontB,x,m,bw,eh,TRUE);x+=bw+g;MoveWindow(hGear,x,m,bw,eh,TRUE);x+=bw+g;MoveWindow(hTop,x,m,topW,eh,TRUE);x+=topW+g;MoveWindow(hMini,x,m,bw,eh,TRUE);
  int y=m+eh+g;if(mini){return;}
  int th=S(26);MoveWindow(hTS,m,y,(W-2*m-g)/2,th,TRUE);MoveWindow(hTC,m+(W-2*m-g)/2+g,y,(W-2*m-g)/2,th,TRUE);y+=th+g;
  ShowWindow(hRank,gear?SW_SHOW:SW_HIDE);ShowWindow(hName,gear?SW_SHOW:SW_HIDE);ShowWindow(hLim,gear?SW_SHOW:SW_HIDE);
  if(gear){int lw=S(70);int w=(W-2*m-2*g-lw)/2;MoveWindow(hRank,m,y,w,eh,TRUE);MoveWindow(hName,m+w+g,y,w,eh,TRUE);MoveWindow(hLim,m+2*(w+g),y,lw,eh,TRUE);y+=eh+g;}
  /* чипы-фильтры */
  int cx=m;int ch=S(20);HDC dc=GetDC(hMain);SelectObject(dc,fntS);
  for(int i=0;i<nchips;i++){if(cons){ShowWindow(chips[i],SW_HIDE);continue;}ShowWindow(chips[i],SW_SHOW);SIZE sz;GetTextExtentPoint32W(dc,GRPN[i],(int)wcslen(GRPN[i]),&sz);int w=sz.cx+S(14);if(cx+w>W-m){cx=m;y+=ch+g;}MoveWindow(chips[i],cx,y,w,ch,TRUE);cx+=w+g;}
  ReleaseDC(hMain,dc);if(!cons)y+=ch+g;
  ShowWindow(hHint,SW_SHOW);MoveWindow(hHint,m,y,W-2*m,S(16),TRUE);y+=S(18);
  int bh=S(26),by=H-m-bh;int avail=(nspeech?by-g:H-m)-y;
  if(cons){ShowWindow(hList,SW_HIDE);ShowWindow(hCons,SW_SHOW);int ch2=avail*55/100;MoveWindow(hCons,m,y,W-2*m,ch2,TRUE);MoveWindow(hDet,m,y+ch2+g,W-2*m,avail-ch2-g,TRUE);layoutCons();}
  else{ShowWindow(hCons,SW_HIDE);ShowWindow(hList,SW_SHOW);int rowh=S(fsize+10);int want=nshown*rowh+S(4);int mx=avail*40/100;int lh=want<mx?want:mx;if(lh<rowh)lh=rowh;MoveWindow(hList,m,y,W-2*m,lh,TRUE);MoveWindow(hDet,m,y+lh+g,W-2*m,avail-lh-g,TRUE);}
  MoveWindow(hCopyAll,m,by,S(150),bh,TRUE);MoveWindow(hChat,m+S(150)+g,by,S(170),bh,TRUE);}

/* ---------------- настройки ---------------- */
static void saveCfg(void){WCHAR b[64];swprintf(b,64,L"%d",fsize);WritePrivateProfileStringW(L"ui",L"size",b,ini);swprintf(b,64,L"%d",fidx);WritePrivateProfileStringW(L"ui",L"font",b,ini);
  GetWindowTextW(hRank,b,64);WritePrivateProfileStringW(L"ui",L"rank",b,ini);GetWindowTextW(hName,b,64);WritePrivateProfileStringW(L"ui",L"name",b,ini);GetWindowTextW(hLim,b,64);WritePrivateProfileStringW(L"ui",L"limit",b,ini);}
static void applyFont(void){if(fnt)DeleteObject(fnt);if(fntS)DeleteObject(fntS);if(fntB)DeleteObject(fntB);
  fnt=CreateFontW(-S(fsize),0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,FONTS[fidx]);fntB=CreateFontW(-S(fsize),0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,FONTS[fidx]);
  fntS=CreateFontW(-S(12),0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Tahoma");
  if(fntC)DeleteObject(fntC);fntC=CreateFontW(-S(10),0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Tahoma");
  SendMessageW(hQ,WM_SETFONT,(WPARAM)fnt,1);SendMessageW(hList,WM_SETFONT,(WPARAM)fnt,1);SendMessageW(hHint,WM_SETFONT,(WPARAM)fntS,1);
  {HWND hs3[3]={hRank,hName,hLim};for(int i=0;i<3;i++)SendMessageW(hs3[i],WM_SETFONT,(WPARAM)fntS,1);}
  for(int i=0;i<NG;i++)if(glabs[i])SendMessageW(glabs[i],WM_SETFONT,(WPARAM)fntC,1);
  SendMessageW(hList,LB_SETITEMHEIGHT,0,S(fsize+10));layout();int s=(int)SendMessageW(hList,LB_GETCURSEL,0,0);
  if(cons)consResult();else if(s>=0&&s<nshown)showDetail(shown[s]);InvalidateRect(hMain,0,TRUE);saveCfg();}

static void setMini(int on){mini=on;RECT r;GetWindowRect(hMain,&r);
  int show=on?SW_HIDE:SW_SHOW;HWND hs[]={hTS,hTC,hHint,hList,hDet,hCons,hCopyAll,hChat,hRank,hName,hLim};for(int i=0;i<11;i++)ShowWindow(hs[i],show);for(int i=0;i<nchips;i++)ShowWindow(chips[i],show);
  if(on){fullH=r.bottom-r.top;RECT c={0,0,0,S(8)*2+S(fsize+14)};AdjustWindowRectExForDpi(&c,WS_OVERLAPPEDWINDOW,FALSE,WS_EX_TOPMOST,dpi);SetWindowPos(hMain,0,0,0,r.right-r.left,c.bottom-c.top,SWP_NOMOVE|SWP_NOZORDER);}
  else{SetWindowPos(hMain,0,0,0,r.right-r.left,fullH?fullH:S(640),SWP_NOMOVE|SWP_NOZORDER);if(!cons)ShowWindow(hCons,SW_HIDE);else ShowWindow(hList,SW_HIDE);layout();}
  SetWindowTextW(hMini,on?L"□":L"_");}
static void setTab(int c){cons=c;InvalidateRect(hMain,0,TRUE);InvalidateRect(hTS,0,TRUE);InvalidateRect(hTC,0,TRUE);layout();if(cons)consResult();else{int s=(int)SendMessageW(hList,LB_GETCURSEL,0,0);if(s>=0&&s<nshown)showDetail(shown[s]);SetFocus(hQ);}layout();RedrawWindow(hMain,0,0,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN);}

/* ---------------- окна ---------------- */
static WNDPROC oldDet;
static LRESULT CALLBACK DetProc(HWND h,UINT m,WPARAM w,LPARAM l){LRESULT r=CallWindowProcW(oldDet,h,m,w,l);
  if(m==WM_LBUTTONUP){CHARRANGE cr;SendMessageW(h,EM_EXGETSEL,0,(LPARAM)&cr);for(int i=0;i<nsay;i++)if(cr.cpMin>=sayStart[i]&&cr.cpMin<=sayEnd[i]){clip(sayTxt[i]);flash(L"Фраза скопирована");break;}}return r;}
static LRESULT CALLBACK ConsProc(HWND h,UINT m,WPARAM w,LPARAM l){
  switch(m){
  case WM_COMMAND:{int id=LOWORD(w);if(HIWORD(w)!=BN_CLICKED)return 0;if(id>=ID_OPT0&&id<ID_OPT0+nob){OptBtn*o=&OB[id-ID_OPT0];switch(o->g){case G_PL:cPlace=o->i;break;case G_WH:cWho=o->i;if(cWho!=2)memset(cMi,0,sizeof(cMi));break;case G_FA:cFa[o->i]^=1;break;case G_MI:cMi[o->i]^=1;break;case G_WP:cWeap=o->i;break;case G_WX:cWx[o->i]^=1;break;case G_DR:cDrug=o->i;break;case G_FL:cFl[o->i]^=1;break;}
      for(int i=0;i<nob;i++)InvalidateRect(OB[i].h,0,TRUE);layoutCons();consResult();layout();}return 0;}
  case WM_DRAWITEM:{DRAWITEMSTRUCT*d=(DRAWITEMSTRUCT*)l;OptBtn*o=&OB[d->CtlID-ID_OPT0];int on=optOn(o->g,o->i);FillRect(d->hDC,&d->rcItem,brPanel);HBRUSH b=CreateSolidBrush(on?cChip:RGB(15,26,46));HPEN pn=CreatePen(PS_SOLID,1,on?RGB(74,106,165):RGB(37,51,80));HGDIOBJ ob=SelectObject(d->hDC,b),op=SelectObject(d->hDC,pn);
      RoundRect(d->hDC,d->rcItem.left,d->rcItem.top,d->rcItem.right,d->rcItem.bottom,S(12),S(12));SelectObject(d->hDC,ob);SelectObject(d->hDC,op);DeleteObject(b);DeleteObject(pn);
      SetBkMode(d->hDC,TRANSPARENT);SetTextColor(d->hDC,on?RGB(255,255,255):cTxt);SelectObject(d->hDC,fntC);DrawTextW(d->hDC,optLabel(o->g,o->i),-1,&d->rcItem,DT_SINGLELINE|DT_VCENTER|DT_CENTER);return TRUE;}
  case WM_CTLCOLORSTATIC:SetTextColor((HDC)w,cMuted);SetBkColor((HDC)w,cPanel);return (LRESULT)brPanel;
  case WM_ERASEBKGND:{RECT r;GetClientRect(h,&r);FillRect((HDC)w,&r,brPanel);return 1;}
  case WM_MOUSEWHEEL:{RECT r;GetClientRect(h,&r);int d=-GET_WHEEL_DELTA_WPARAM(w)/120*S(36);consScroll+=d;int mx=consH-r.bottom;if(consScroll>mx)consScroll=mx;if(consScroll<0)consScroll=0;layoutCons();InvalidateRect(h,0,TRUE);return 0;}
  case WM_VSCROLL:{RECT r;GetClientRect(h,&r);SCROLLINFO si={sizeof(si),SIF_ALL};GetScrollInfo(h,SB_VERT,&si);int p=si.nPos;switch(LOWORD(w)){case SB_LINEUP:p-=S(30);break;case SB_LINEDOWN:p+=S(30);break;case SB_PAGEUP:p-=r.bottom;break;case SB_PAGEDOWN:p+=r.bottom;break;case SB_THUMBTRACK:case SB_THUMBPOSITION:p=si.nTrackPos;break;}
      int mx=consH-r.bottom;if(p>mx)p=mx;if(p<0)p=0;consScroll=p;layoutCons();InvalidateRect(h,0,TRUE);return 0;}
  case WM_SIZE:layoutCons();return 0;}
  return DefWindowProcW(h,m,w,l);}

static WNDPROC oldBtn;
static LRESULT CALLBACK BtnProc(HWND h,UINT m,WPARAM w,LPARAM l){if(m==WM_LBUTTONDBLCLK)m=WM_LBUTTONDOWN;return CallWindowProcW(oldBtn,h,m,w,l);}
static HWND mkBtn(const WCHAR*t,int id,HWND par){HWND b=CreateWindowExW(0,L"BUTTON",t,WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,0,0,0,0,par,(HMENU)(INT_PTR)id,0,0);WNDPROC o=(WNDPROC)SetWindowLongPtrW(b,GWLP_WNDPROC,(LONG_PTR)BtnProc);if(!oldBtn)oldBtn=o;return b;}
static HWND mkEdit(int id,const WCHAR*cue){HWND e=CreateWindowExW(0,L"EDIT",L"",WS_CHILD|ES_AUTOHSCROLL,0,0,0,0,hMain,(HMENU)(INT_PTR)id,0,0);SendMessageW(e,0x1501/*EM_SETCUEBANNER*/,TRUE,(LPARAM)cue);return e;}
typedef UINT (WINAPI*PGDFW)(HWND);static int getDpi(HWND h){static PGDFW f=0;static int t=0;if(!t){t=1;f=(PGDFW)GetProcAddress(GetModuleHandleW(L"user32.dll"),"GetDpiForWindow");}if(f){UINT d=f(h);if(d)return d;}HDC dc=GetDC(h);int d=GetDeviceCaps(dc,LOGPIXELSY);ReleaseDC(h,dc);return d?d:96;}

LRESULT CALLBACK Wnd(HWND h,UINT m,WPARAM w,LPARAM l){
  switch(m){
  case WM_CREATE:{hMain=h;dpi=getDpi(h);brBg=CreateSolidBrush(cBg);brPanel=CreateSolidBrush(cPanel);
    hQ=CreateWindowExW(0,L"EDIT",L"",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,0,0,0,0,h,(HMENU)ID_Q,0,0);SendMessageW(hQ,0x1501,TRUE,(LPARAM)L"Что случилось? угон, стрелял, остров, 17.1…");
    hMinus=mkBtn(L"A−",ID_MINUS,h);hPlus=mkBtn(L"A+",ID_PLUS,h);hFontB=mkBtn(L"Аа",ID_FONT,h);hGear=mkBtn(L"Я",ID_GEAR,h);hTop=mkBtn(L"Поверх",ID_TOP,h);hMini=mkBtn(L"_",ID_MINI,h);
    hTS=mkBtn(L"Поиск",ID_TSEARCH,h);hTC=mkBtn(L"Задержание",ID_TCONS,h);
    for(int i=0;i<NGRP;i++)chips[i]=mkBtn(GRPN[i],ID_CHIP0+i,h);nchips=NGRP;
    hHint=CreateWindowExW(0,L"STATIC",HINT,WS_CHILD|WS_VISIBLE|SS_ENDELLIPSIS,0,0,0,0,h,0,0,0);
    hRank=mkEdit(ID_RANK,L"Звание");hName=mkEdit(ID_NAME,L"Фамилия");hLim=mkEdit(ID_LIM,L"Лимит чата");
    {WCHAR b[64];GetPrivateProfileStringW(L"ui",L"rank",L"",b,64,ini);SetWindowTextW(hRank,b);GetPrivateProfileStringW(L"ui",L"name",L"",b,64,ini);SetWindowTextW(hName,b);GetPrivateProfileStringW(L"ui",L"limit",L"120",b,64,ini);SetWindowTextW(hLim,b);}
    hList=CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|LBS_OWNERDRAWFIXED,0,0,0,0,h,(HMENU)ID_LIST,0,0);
    hDet=CreateWindowExW(0,MSFTEDIT_CLASS,L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL,0,0,0,0,h,(HMENU)ID_DET,0,0);
    SendMessageW(hDet,EM_SETBKGNDCOLOR,0,cPanel);oldDet=(WNDPROC)SetWindowLongPtrW(hDet,GWLP_WNDPROC,(LONG_PTR)DetProc);
    hCopyAll=mkBtn(L"Скопировать речь",ID_COPYALL,h);hChat=mkBtn(L"В чат по частям",ID_CHAT,h);
    WNDCLASSW wc={0};wc.lpfnWndProc=ConsProc;wc.hInstance=GetModuleHandleW(0);wc.lpszClassName=L"PSConsPanel";wc.hCursor=LoadCursor(0,IDC_ARROW);RegisterClassW(&wc);
    hCons=CreateWindowExW(0,L"PSConsPanel",L"",WS_CHILD|WS_VSCROLL|WS_CLIPCHILDREN,0,0,0,0,h,0,0,0);
    for(int G=0;G<NG;G++){glabs[G]=CreateWindowExW(0,L"STATIC",GLAB[G],WS_CHILD,0,0,0,0,hCons,0,0,0);for(int i=0;i<gCount(G);i++){OB[nob].g=G;OB[nob].i=i;OB[nob].h=mkBtn(L"",ID_OPT0+nob,hCons);ShowWindow(OB[nob].h,SW_HIDE);nob++;}}
    SetWindowTheme(hList,L"DarkMode_Explorer",0);SetWindowTheme(hDet,L"DarkMode_Explorer",0);SetWindowTheme(hCons,L"DarkMode_Explorer",0);
    applyFont();doSearch();layout();SetFocus(hQ);return 0;}
  case WM_SIZE:if(hQ&&!mini)layout();return 0;
  case WM_DPICHANGED:{dpi=HIWORD(w);RECT*r=(RECT*)l;SetWindowPos(h,0,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);applyFont();return 0;}
  case WM_GETMINMAXINFO:{MINMAXINFO*mi=(MINMAXINFO*)l;mi->ptMinTrackSize.x=S(360);if(!mini)mi->ptMinTrackSize.y=S(360);return 0;}
  case WM_TIMER:KillTimer(h,1);SetWindowTextW(hHint,HINT);return 0;
  case WM_MEASUREITEM:((MEASUREITEMSTRUCT*)l)->itemHeight=S(fsize+10);return TRUE;
  case WM_DRAWITEM:{DRAWITEMSTRUCT*d=(DRAWITEMSTRUCT*)l;
    if(d->CtlID==ID_LIST){if(d->itemID==(UINT)-1||(int)d->itemID>=nshown)return TRUE;Item*a=&IT[shown[d->itemID]];int sel=d->itemState&ODS_SELECTED;
      HBRUSH b=CreateSolidBrush(sel?cSel:cPanel);FillRect(d->hDC,&d->rcItem,b);DeleteObject(b);SetBkMode(d->hDC,TRANSPARENT);SelectObject(d->hDC,fnt);RECT r=d->rcItem;r.left+=S(8);r.right-=S(4);
      WCHAR h[128];if(a->sit){wcscpy(h,L"•");SetTextColor(d->hDC,cBlue);}else{swprintf(h,128,L"%ls%ls%ls",a->num,*a->stars?L"  ":L"",a->stars);SetTextColor(d->hDC,cGold);}
      DrawTextW(d->hDC,h,-1,&r,DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX);SIZE sz;GetTextExtentPoint32W(d->hDC,h,(int)wcslen(h),&sz);r.left+=sz.cx+S(10);SetTextColor(d->hDC,cTxt);DrawTextW(d->hDC,a->title,-1,&r,DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS);return TRUE;}
    {COLORREF bg=cPanel,fg=cTxt;int id=d->CtlID;if(id==ID_TOP&&topOn)bg=cOn;if(id==ID_GEAR&&gear)bg=cChip;if((id==ID_TSEARCH&&!cons)||(id==ID_TCONS&&cons))bg=cSel;if(id==ID_TSEARCH&&cons)fg=cMuted;if(id==ID_TCONS&&!cons)fg=cMuted;
     if(id==ID_COPYALL)bg=cOn;if(id==ID_CHAT)bg=cChip;if(id>=ID_CHIP0&&id<ID_CHIP0+NGRP){bg=(id-ID_CHIP0==grp)?cChip:cPanel;fg=(id-ID_CHIP0==grp)?RGB(255,255,255):cMuted;}
     if(d->itemState&ODS_SELECTED)bg=cSel;HBRUSH b=CreateSolidBrush(bg);FillRect(d->hDC,&d->rcItem,b);DeleteObject(b);
     WCHAR t[64];GetWindowTextW(d->hwndItem,t,64);SetBkMode(d->hDC,TRANSPARENT);SetTextColor(d->hDC,fg);SelectObject(d->hDC,fntS);DrawTextW(d->hDC,t,-1,&d->rcItem,DT_SINGLELINE|DT_VCENTER|DT_CENTER);return TRUE;}}
  case WM_COMMAND:{int id=LOWORD(w);
    if(id==ID_Q&&HIWORD(w)==EN_CHANGE){if(cons)setTab(0);if(mini){WCHAR b[4];GetWindowTextW(hQ,b,4);if(*b)setMini(0);}doSearch();layout();return 0;}
    if(id==ID_LIST){if(HIWORD(w)==LBN_SELCHANGE){int s=(int)SendMessageW(hList,LB_GETCURSEL,0,0);if(s>=0&&s<nshown)showDetail(shown[s]);layout();}return 0;}
    if(HIWORD(w)!=BN_CLICKED&&id!=ID_RANK&&id!=ID_NAME&&id!=ID_LIM)return 0;
    if(id>=ID_CHIP0&&id<ID_CHIP0+NGRP){grp=id-ID_CHIP0;for(int i=0;i<nchips;i++)InvalidateRect(chips[i],0,TRUE);doSearch();layout();SetFocus(hQ);return 0;}
    if((id==ID_RANK||id==ID_NAME||id==ID_LIM)&&HIWORD(w)==EN_CHANGE){saveCfg();if(cons)consResult();else{int s=(int)SendMessageW(hList,LB_GETCURSEL,0,0);if(s>=0&&s<nshown)showDetail(shown[s]);}return 0;}
    switch(id){
    case ID_MINUS:if(fsize>10){fsize--;applyFont();}break;case ID_PLUS:if(fsize<30){fsize++;applyFont();}break;
    case ID_FONT:{fidx=(fidx+1)%5;applyFont();WCHAR b[64];swprintf(b,64,L"Шрифт: %ls",FONTS[fidx]);flash(b);break;}
    case ID_GEAR:gear=!gear;InvalidateRect(hGear,0,TRUE);layout();break;
    case ID_TOP:topOn=!topOn;SetWindowPos(h,topOn?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);InvalidateRect(hTop,0,TRUE);flash(topOn?L"Поверх окон: вкл":L"Поверх окон: выкл");break;
    case ID_MINI:setMini(!mini);break;
    case ID_TSEARCH:setTab(0);break;case ID_TCONS:setTab(1);break;
    case ID_COPYALL:{WCHAR all[4096];joinSpeech(all,4096);WCHAR*nl=malloc(8192*sizeof(WCHAR));nl[0]=0;for(int i=0;i<nspeech;i++){wcscat(nl,speech[i]);wcscat(nl,L"\r\n");}clip(nl);free(nl);flash(L"Речь скопирована");break;}
    case ID_CHAT:chatCopy();break;}
    if(id!=ID_RANK&&id!=ID_NAME&&id!=ID_LIM&&!cons)SetFocus(hQ);return 0;}
  case WM_CTLCOLOREDIT:case WM_CTLCOLORLISTBOX:SetTextColor((HDC)w,cTxt);SetBkColor((HDC)w,cPanel);return (LRESULT)brPanel;
  case WM_CTLCOLORSTATIC:SetTextColor((HDC)w,cMuted);SetBkColor((HDC)w,cBg);return (LRESULT)brBg;
  case WM_ERASEBKGND:{RECT r;GetClientRect(h,&r);FillRect((HDC)w,&r,brBg);return 1;}
  case WM_DESTROY:saveCfg();PostQuitMessage(0);return 0;}
  return DefWindowProcW(h,m,w,l);}

int WINAPI wWinMain(HINSTANCE hi,HINSTANCE p,PWSTR c,int n){
  LoadLibraryW(L"Msftedit.dll");
  WCHAR ap[MAX_PATH];if(GetEnvironmentVariableW(L"APPDATA",ap,MAX_PATH)){wcscpy(ini,ap);wcscat(ini,L"\\PamyatkaSANG.ini");}else wcscpy(ini,L"PamyatkaSANG.ini");
  fsize=GetPrivateProfileIntW(L"ui",L"size",15,ini);fidx=GetPrivateProfileIntW(L"ui",L"font",0,ini);if(fsize<10||fsize>30)fsize=15;if(fidx<0||fidx>4)fidx=0;
  loadDb();
  WNDCLASSW wc={0};wc.lpfnWndProc=Wnd;wc.hInstance=hi;wc.lpszClassName=L"PamyatkaSANG";wc.hCursor=LoadCursor(0,IDC_ARROW);wc.hIcon=LoadIconW(hi,MAKEINTRESOURCEW(1));RegisterClassW(&wc);
  HWND h=CreateWindowExW(WS_EX_TOPMOST,L"PamyatkaSANG",L"SANG Codex",WS_OVERLAPPEDWINDOW,40,40,480,720,0,0,hi,0);
  int d=getDpi(h);if(d!=96)SetWindowPos(h,0,0,0,MulDiv(480,d,96),MulDiv(720,d,96),SWP_NOMOVE|SWP_NOZORDER);
#ifdef PS_TEST
  {FILE*f=fopen("q.txt","rb");if(f){char b[512]={0};fread(b,1,511,f);fclose(f);WCHAR*w=u8(b,-1);if(!strncmp(b,"CONS",4)){setTab(1);}else SetWindowTextW(hQ,w);}}
#endif
  SendMessageW(h,WM_SETICON,ICON_SMALL,(LPARAM)LoadImageW(hi,MAKEINTRESOURCEW(1),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0));
  SendMessageW(h,WM_SETICON,ICON_BIG,(LPARAM)LoadImageW(hi,MAKEINTRESOURCEW(1),IMAGE_ICON,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON),0));
  ShowWindow(h,n);MSG msg;
  while(GetMessageW(&msg,0,0,0)){
    if(msg.message==WM_KEYDOWN&&msg.wParam==VK_TAB&&msg.hwnd!=hRank&&msg.hwnd!=hName&&msg.hwnd!=hLim){setTab(!cons);continue;}
    if(msg.message==WM_KEYDOWN&&msg.hwnd==hQ){
      if(msg.wParam==VK_ESCAPE){SetWindowTextW(hQ,L"");continue;}
      if(msg.wParam==VK_DOWN||msg.wParam==VK_UP){int s=(int)SendMessageW(hList,LB_GETCURSEL,0,0)+(msg.wParam==VK_DOWN?1:-1);if(s>=0&&s<nshown){SendMessageW(hList,LB_SETCURSEL,s,0);showDetail(shown[s]);layout();}continue;}}
    if(msg.message==WM_MOUSEWHEEL&&(GET_KEYSTATE_WPARAM(msg.wParam)&MK_CONTROL)){int dd=GET_WHEEL_DELTA_WPARAM(msg.wParam)>0?1:-1;if(fsize+dd>=10&&fsize+dd<=30){fsize+=dd;applyFont();}continue;}
    TranslateMessage(&msg);DispatchMessageW(&msg);}
  return 0;}
