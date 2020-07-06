#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif


#define PRITISNUTO(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define numberOffset 15  //Offset za poziciju brojeva kod odbrojavanja
#define angleTreshold 15 //Za odreðivanje koordinata izmeðu kojih æe se desiti kosi udarac
#define lineThickness 10 //Debljina linije prilikom iscrtavanja, koristi se i za razmak izmeðu kvadrata kod iscrtavanja igraèa
#define playerFromEdge 96 // Udaljenost igraèa od ivica ekrana
#define centerRadius 100 // Preènik kruga na centru igrališta
#define goalWidth 90 // Pola širine gola
#define checkOffset 10 // Offset, za provjeravanje koordinata, potreban, jer se koordinate nekad neće poklopiti
#define S 0x53  // S tipka na tastaturi
#define W 0x57  // W tipka na tastaturi
#define A 0x41  // A tipka na tastaturi
#define D 0x44  // D tipka na tastaturi

#define PLAY 1
#define SET 2
#define EXIT 3

#define PLAY_BUTTON 101
#define SET_BUTTON 102
#define EXIT_BUTTON 103
#define STATIC_BOX1 104
#define STATIC_BOX2 105
#define NAME1_BOX 106
#define NAME2_BOX 107
#define ENABLEDISABLE_BUTTON 108
#define OPTIONS_BUTTON 109
#define ABOUT_BUTTON 110

#define IDI_ICON 201

#define REDP1 301
#define REDP2 302
#define BLUEP1 303
#define BLUEP2 304
#define GREENP1 305
#define GREENP2 306
#define PURPLEP1 307
#define PURPLEP2 308
#define YELLOWP1 309
#define YELLOWP2 310
#define BLACKP1 311
#define BLACKP2 312
#define SURPRISEP1 313
#define SURPRISEP2 314
#define SET_COLOR 315

#define STATIC_POINTS 401
#define EDIT_POINTS 402
#define SET_POINTS 403

#define BALL_STATIC 501
#define BALL_EDIT 502
#define SET_SIZE 503

#define GROUP_COLORS1 601
#define GROUP_COLORS2 602

#define LAZY_BUTTON 701
#define EASY_BUTTON 702
#define NORMAL_BUTTON 703
#define HARD_BUTTON 704
#define INSANE_BUTTON 705
