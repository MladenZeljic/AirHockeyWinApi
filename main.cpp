#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <windows.h>
#include <tchar.h>
#include <time.h>
#include <stdio.h>
#include <mmsystem.h>
#include <iostream>
#include <math.h>
#include "resource.h"

using namespace std;

static unsigned int _frameDelay=30;
static char goal_char [2];
static bool mainDraw=false,numberOver=false,drawGoalOne=false,drawGoalTwo;
static int i=0,j=0;
HFONT oldFont,font;
COLORREF colorP1,color,colorP2,color1,color2;
static int playerWidth=0,playerHeight=0,start=0,ballDiameter=0,scorePlayerOne=0,scorePlayerTwo=0,checkedP1=0,checkedP2=0,ballRadius=32,playPoints=5,accelerationGain=6,playerAcceleration=4;
static POINT playerOnePosition,playerTwoPosition,ballPosition;
char playerOneName[20], playerTwoName[20],playerOneNameBuffer[20], playerTwoNameBuffer[20];
char poruka1[20], poruka2[20];
bool check_if_in_range(float,float,float);
void drawGoal(HDC);
void play_shot();
void shot();
void countdown();
void countdown_end();
void shot_big();
void shot_small();
void goal_bar_shot();
void goal();
void congrats();
void bg_music(HWND);
void stop_music(HWND);
void drawNumbers(HDC,HDC);
int CheckRadioP1(HWND,HWND);
int CheckRadioP2(HWND,HWND);
int CheckDifficulty(HWND,HWND);

BITMAP bmp;
HDC hdcBuffer,hdcMem;
HBITMAP number,numberMask;
HBITMAP hbmBuffer,hbmMem;
HBRUSH oldBrush,ballBrush,whiteBrush;
HPEN oldPen,currentPen,ballPen;
static HWND hEnDis;
int window = 1;
bool sound = true;
bool create = false,gameActive=false,isPlayerSet=false;

void DrawBkg(HDC, RECT*);
void setPlayerOneShot(void);
void setPlayerTwoShot(void);


void UncheckSelectedButton(HWND, HWND);
void ChangeButtonStateAndSendTheMessageToTheParent(HWND, HWND, int);

typedef struct ObjectInfo
{
    int width;
    int height;
    int x;
    int y;

    int dx;
    int dy;
} Object;

Object bkg,field,numberInfo;
HBITMAP hbmBkg,hbmField;

class Vector2D
{
    public:
        Vector2D();
        virtual ~Vector2D();
        Vector2D(double x, double y);
        void SetAngle(double angle);
        double GetAngle();
        void SetLength(double length);
        double GetLength();
        Vector2D Add(const Vector2D& other);
        Vector2D Substract(const Vector2D& other);
        Vector2D Multiply(int value);
        Vector2D Divide(int value);
        void AddTo(const Vector2D& other);
        void SubstractFrom(const Vector2D& other);
        void MultiplyBy(int value);
        void DivideBy(int value);
        void SetX(double value);
        void SetY(double value);
        double GetX();
        double GetY();
    private:
        double _x;
        double _y;
};

Vector2D::Vector2D(double x,double y){
        SetX(x);
        SetY(y);
}

Vector2D::Vector2D(){}

Vector2D:: ~Vector2D(){}

void Vector2D::SetX(double value){
    _x=value;
}

void Vector2D::SetY(double value){
    _y=value;
}

double Vector2D::GetX(){
    return _x;
}

double Vector2D::GetY(){
    return _y;
}

double Vector2D::GetLength(){
    return sqrt(pow(GetX(),2)+pow(GetY(),2));
}

void Vector2D::SetAngle(double angle){
    double length=GetLength();
    SetY(length*sin(angle));
    SetX(sqrt(pow(length,2)-pow(GetY(),2)));
}

double Vector2D::GetAngle(){
    double length=sqrt(pow(GetX(),2)+pow(GetY(),2));
    return 180+(asin(GetY()/length)*180)/M_PI;
}

void Vector2D::SetLength(double length){
    double old_length=GetLength();
    double sin_alpha=(GetY()/old_length)*(M_PI/180);
    SetY(length*sin_alpha);
    SetX(sqrt(pow(length,2)*(1-pow(sin_alpha,2))));
}

Vector2D Vector2D::Add(const Vector2D& value){
    _x=_x+value._x;
    _y=_y+value._y;
    return Vector2D(_x,_y);
}

Vector2D Vector2D::Substract(const Vector2D& value){
    _x=_x-value._x;
    _y=_y-value._y;
    return Vector2D(_x,_y);
}

Vector2D Vector2D::Multiply(int value){
    _x=_x*value;
    _y=_y*value;
    return Vector2D(_x,_y);
}

Vector2D Vector2D::Divide(int value){
    _x=_x/value;
    _y=_y/value;
    return Vector2D(_x,_y);
}

void Vector2D::AddTo(const Vector2D& other){
    _x+=other._x;
    _y+=other._y;

}

void Vector2D::SubstractFrom(const Vector2D& other){
    _x=_x+other._x;
    _y=_y+other._y;
}

void Vector2D::MultiplyBy(int value){
    _x=_x*value;
    _y=_y*value;
}

void Vector2D::DivideBy(int value){
    _x=_x/value;
    _y=_y/value;
}

class GameObject
{
    public:
        GameObject()=default;
        virtual ~GameObject();
        virtual void update() = 0;
        virtual void render(HDC hdc) = 0;
        virtual void checkInput() = 0;
        void SetGameObjectPosition(Vector2D& object);
        void SetGameObjectVelocity(Vector2D& object);
        void SetGameObjectAccelerization(Vector2D& object);
        Vector2D& GetGameObjectPosition();
        Vector2D& GetGameObjectVelocity();
        Vector2D& GetGameObjectAcceleration();
    protected:
        Vector2D _position;
        Vector2D _velocity;
        Vector2D _acceleration;
};

GameObject::~GameObject(){}

Vector2D& GameObject::GetGameObjectPosition(){
    return _position;
}

Vector2D& GameObject::GetGameObjectVelocity(){
    return _velocity;
}

Vector2D& GameObject::GetGameObjectAcceleration(){
    return _acceleration;
}

void GameObject::SetGameObjectPosition(Vector2D& object){
    GetGameObjectPosition().SetX(object.GetX());
    GetGameObjectPosition().SetY(object.GetY());
}

void GameObject::SetGameObjectVelocity(Vector2D& object){
    GetGameObjectVelocity().SetX(object.GetX());
    GetGameObjectVelocity().SetY(object.GetY());
}

void GameObject::SetGameObjectAccelerization(Vector2D& object){
    GetGameObjectAcceleration().SetX(object.GetX());
    GetGameObjectAcceleration().SetY(object.GetY());
}


class CircleObject:public GameObject{
    public:
        CircleObject(double r,Vector2D pos) {
            _diameter = r;
            _position = pos;
        }
        CircleObject() = default;

        double GetDiameter();
        void SetDiameter(double d);

        void setPosition(Vector2D);

        void checkInput(){
            HWND hwnd=GetFocus();
            if(hwnd){//Dodatak, samo ovaj if
                RECT r;
                GetClientRect(hwnd,&r);
                int p1x1=playerOnePosition.x - playerWidth/2;   //Gornja x koordinata prvog igraèa
                int p1y1=playerOnePosition.y - playerHeight/2;  //Gornja y koordinata prvog igraèa
                int p1x2=playerOnePosition.x + playerWidth/2;   //Donja x koordinata prvog igraèa
                int p1y2=playerOnePosition.y + playerHeight/2;  //Donja y koordinata prvog igraèa

                int p2x1=playerTwoPosition.x - playerWidth/2;   //Gornja x koordinata drugog igraèa
                int p2y1=playerTwoPosition.y - playerHeight/2;  //Gornja y koordinata drugog igraèa
                int p2x2=playerTwoPosition.x + playerWidth/2;   //Donja x koordinata drugog igraèa
                int p2y2=playerTwoPosition.y + playerHeight/2;  //Donja y koordinata drugog igraèa

                int cx1=_position.GetX() - _diameter/2; //Gornja x koordinata loptice
                int cy1=_position.GetY() - _diameter/2; //Gornja y koordinata loptice
                int cx2=_position.GetX() + _diameter/2; //Donja x koordinata loptice
                int cy2=_position.GetY() + _diameter/2; //Donja y koordinata loptice

                if(start==0){
                    if(PRITISNUTO(VK_SPACE)||PRITISNUTO(VK_RETURN)){    //Ako je pritisnut SPACE ili ENTER i ako je start 0 (da se ne bi sluèajno smjer loptice promijenio ako se ona npr. trenutno spušta) pokreni lopticu na gore
                        _acceleration.SetY(-1*accelerationGain);
                        start++;
                    }
                }

                else{
                    if(check_if_in_range(cy2,p1y1-checkOffset,p1y1+checkOffset)){//Gornji kraj prvog igraèa
                        if(cx2>p1x1+angleTreshold &&cx1<p1x2-angleTreshold){//"Prav" udarac
                            if(sound == true){
                                play_shot();
                            } //Pusti zvuk udarca
                            start=1; // Loptica je udarila, nema potrebe da se sad opet klikæe na ENTER ili SPACE
                            _acceleration.SetY(-1*accelerationGain); //Postavi Y na gore
                        }
                        else if(check_if_in_range(cx2,p1x1,p1x1+angleTreshold)){//Kosi udarac na lijevo
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(-1*accelerationGain);//Postavi X na lijevo
                            _acceleration.SetY(-1*accelerationGain);//Postavi Y na gore što rezultuje kosim udarcem na lijevo
                        }
                        else if(check_if_in_range(cx1,p1x2-angleTreshold,p1x2)){//Kosi udarac na desno
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(accelerationGain);
                            _acceleration.SetY(-1*accelerationGain);
                        }
                        else{
                            _acceleration.SetX(_acceleration.GetX());
                            _acceleration.SetY(_acceleration.GetY());
                        }
                    }

                    if(check_if_in_range(cy1,p1y2-checkOffset,p1y2+checkOffset)){//Donji kraj prvog igraèa
                        if (cx2>=p1x1+angleTreshold &&cx1<=p1x2-angleTreshold){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetY(accelerationGain);
                        }
                        else if(check_if_in_range(cx2,p1x1,p1x1+angleTreshold)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(-1*accelerationGain);
                            _acceleration.SetY(accelerationGain);
                        }
                        else if(check_if_in_range(cx1,p1x2-angleTreshold,p1x2)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(-1*accelerationGain);
                            _acceleration.SetY(accelerationGain);
                        }
                        else{
                            _acceleration.SetX(_acceleration.GetX());
                            _acceleration.SetY(_acceleration.GetY());
                        }
                    }

                    if(check_if_in_range(cx2,p1x1-checkOffset,p1x1+checkOffset)){//Lijevi kraj prvog igraèa
                        if(cy2>=p1y1+angleTreshold&& cy1<=p1y2-angleTreshold){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(-1*accelerationGain);
                        }
                        else if(check_if_in_range(cy2,p1y1,p1y1+angleTreshold)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(-1*accelerationGain);
                            _acceleration.SetY(-1*accelerationGain);
                        }
                        else if(check_if_in_range(cy1,p1y2-angleTreshold,p1y2)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(-1*accelerationGain);
                            _acceleration.SetY(accelerationGain);
                        }
                        else{
                            _acceleration.SetX(_acceleration.GetX());
                            _acceleration.SetY(_acceleration.GetY());
                        }
                    }

                    if(check_if_in_range(cx1,p1x2-checkOffset,p1x2+checkOffset)){//Desni kraj prvog igraèa
                        if(cy2>=p1y1+angleTreshold&& cy1<=p1y2-angleTreshold){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(accelerationGain);
                        }
                        else if(check_if_in_range(cy2,p1y1,p1y1+angleTreshold)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(accelerationGain);
                            _acceleration.SetY(-1*accelerationGain);
                        }
                        else if(check_if_in_range(cy1,p1y2-angleTreshold,p1y2)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(accelerationGain);
                            _acceleration.SetY(accelerationGain);
                        }
                        else{
                            _acceleration.SetX(_acceleration.GetX());
                            _acceleration.SetY(_acceleration.GetY());
                        }
                    }

                    if(check_if_in_range(cy1,p2y2-checkOffset,p2y2+checkOffset)){//Donji kraj drugog igraèa
                        if(cx2>p2x1+angleTreshold &&cx1<p2x2-angleTreshold){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetY(accelerationGain);
                        }
                        else if(check_if_in_range(cx2,p2x1,p2x1+angleTreshold)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(-1*accelerationGain);
                            _acceleration.SetY(accelerationGain);
                        }
                        else if(check_if_in_range(cx1,p2x2-angleTreshold,p2x2)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(accelerationGain);
                            _acceleration.SetY(accelerationGain);
                        }
                        else{
                            _acceleration.SetX(_acceleration.GetX());
                            _acceleration.SetY(_acceleration.GetY());
                        }
                    }

                    if(check_if_in_range(cy2,p2y1-checkOffset,p2y1+checkOffset)){//Gornji kraj drugog igraèa
                        if(cx2>p2x1+angleTreshold &&cx1<p2x2-angleTreshold){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetY(-1*accelerationGain);
                        }
                        else if(check_if_in_range(cx2,p2x1,p2x1+angleTreshold)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(-1*accelerationGain);
                            _acceleration.SetY(-1*accelerationGain);
                        }
                        else if(check_if_in_range(cx1,p2x2-angleTreshold,p2x2)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(accelerationGain);
                            _acceleration.SetY(-1*accelerationGain);
                        }
                        else{
                            _acceleration.SetX(_acceleration.GetX());
                            _acceleration.SetY(_acceleration.GetY());
                        }
                    }

                    if(check_if_in_range(cx2,p2x1-checkOffset,p2x1+checkOffset)){//Lijevi kraj drugog igraèa
                        if(cy2>=p2y1+angleTreshold&& cy1<=p2y2-angleTreshold){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(-1*accelerationGain);
                        }
                        else if(check_if_in_range(cy2,p2y1,p2y1+angleTreshold)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(-1*accelerationGain);
                            _acceleration.SetY(-1*accelerationGain);
                        }
                        else if(check_if_in_range(cy1,p2y2-angleTreshold,p2y2)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(-1*accelerationGain);
                            _acceleration.SetY(accelerationGain);
                        }
                        else{
                            _acceleration.SetX(_acceleration.GetX());
                            _acceleration.SetY(_acceleration.GetY());
                        }
                    }

                    if(check_if_in_range(cx1,p2x2-checkOffset,p2x2+checkOffset)){//Desni kraj drugog igraèa
                        if(cy2>=p2y1+angleTreshold&& cy1<=p2y2-angleTreshold){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(accelerationGain);
                        }
                        else if(check_if_in_range(cy2,p2y1,p2y1+angleTreshold)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(accelerationGain);
                            _acceleration.SetY(-1*accelerationGain);
                        }
                        else if(check_if_in_range(cy1,p2y2-angleTreshold,p2y2)){
                            if(sound == true){
                                play_shot();
                            }
                            start=1;
                            _acceleration.SetX(accelerationGain);
                            _acceleration.SetY(-1*accelerationGain);
                        }
                        else{
                            _acceleration.SetX(_acceleration.GetX());
                            _acceleration.SetY(_acceleration.GetY());
                        }
                    }
                }
            }
        }

        void update(){
            HWND hwnd=GetFocus();
            HDC hdc=GetDC(hwnd);
            if(hwnd){
                RECT r,rect;
                GetClientRect(hwnd,&r);
                GetClientRect(hwnd,&rect);
                if(_position.GetX()-_diameter/2<r.left+lineThickness/2||_position.GetX()+_diameter/2>r.right-lineThickness/2){
                    if(sound == true){
                        play_shot();
                    }
                    _acceleration.SetX(-1*_acceleration.GetX());
                    _position.AddTo(_acceleration);
                    }

                if(_position.GetY()-_diameter/2>r.bottom||_position.GetY()+_diameter/2<r.top){

                    if(_position.GetY()+_diameter/2<r.top){
                        drawGoalTwo=true;
                        scorePlayerOne = scorePlayerOne + 1;
                    }
                    if(_position.GetY()-_diameter/2>r.bottom){
                        drawGoalOne=true;

                        scorePlayerTwo = scorePlayerTwo + 1;

                    }
                    if(scorePlayerOne<playPoints && scorePlayerTwo<playPoints){
                        _position.SetX((r.right-r.left)/2);
                        _position.SetY((r.bottom-r.top)/2);
                        _acceleration.SetX(0);
                        _acceleration.SetY(0);
                        start=0;
                        if(sound == true){
                            goal();
                        }
                    }
                    if(scorePlayerOne==playPoints || scorePlayerTwo==playPoints){
                        _position.SetX((r.right-r.left)/2);
                        _position.SetY((r.bottom-r.top)/2);
                        _acceleration.SetX(0);
                        _acceleration.SetY(0);
                        if(sound == true){
                            congrats();
                        }
                    }

                }
                /*IZMJENA*/
                if(_position.GetY()+_diameter/2>=r.bottom-lineThickness/2){
                    if(_position.GetX()-_diameter/2>=(r.right-r.left)/2-goalWidth&&_position.GetX()+_diameter/2<=(r.right-r.left)/2+goalWidth){
                        _position.AddTo(_acceleration);
                    }
                    else if(_position.GetX()-_diameter/2<=(r.right-r.left)/2-goalWidth&&_position.GetX()+_diameter/2>=(r.right-r.left)/2-goalWidth){//Lijevi ugao gola
                        if(sound == true){
                            goal_bar_shot();
                        }
                        _acceleration.SetX(accelerationGain);
                        _acceleration.SetY(-1*accelerationGain);
                        _position.AddTo(_acceleration);
                    }
                    else if(_position.GetX()+_diameter/2>=(r.right-r.left)/2+goalWidth&&_position.GetX()-_diameter/2<=(r.right-r.left)/2+goalWidth){//Desni ugao gola
                        if(sound == true){
                            goal_bar_shot();
                        }
                        _acceleration.SetX(-1*accelerationGain);
                        _acceleration.SetY(-1*accelerationGain);
                        _position.AddTo(_acceleration);
                    }
                    else{
                        if(sound == true){
                            play_shot();
                        }
                        _acceleration.SetY(-1*_acceleration.GetY());
                        _position.AddTo(_acceleration);
                    }
                }

                if(_position.GetY()-_diameter/2<=r.top+lineThickness/2){
                    if(_position.GetX()-_diameter/2>=(r.right-r.left)/2-goalWidth&&_position.GetX()+_diameter/2<=(r.right-r.left)/2+goalWidth){
                        _position.AddTo(_acceleration);
                    }
                    else if(_position.GetX()-_diameter/2<=(r.right-r.left)/2-goalWidth&&_position.GetX()+_diameter/2>=(r.right-r.left)/2-goalWidth){//Lijevi ugao gola
                        if(sound == true){
                            goal_bar_shot();
                        }
                        _acceleration.SetX(accelerationGain);
                        _acceleration.SetY(accelerationGain);
                        _position.AddTo(_acceleration);
                    }
                    else if(_position.GetX()+_diameter/2>=(r.right-r.left)/2+goalWidth&&_position.GetX()-_diameter/2<=(r.right-r.left)/2+goalWidth){//Desni ugao gola
                        if(sound == true){
                            goal_bar_shot();
                        }
                        _acceleration.SetX(-1*accelerationGain);
                        _acceleration.SetY(accelerationGain);
                        _position.AddTo(_acceleration);
                    }
                    else{
                        if(sound == true){
                            play_shot();
                        }
                        _acceleration.SetY(-1*_acceleration.GetY());
                        _position.AddTo(_acceleration);
                    }
                }
                /*------*/
                _position.AddTo(_acceleration);
            }
            ReleaseDC(hwnd,hdc);
        }

        void render(HDC hdc){
            if(scorePlayerOne==playPoints || scorePlayerTwo==playPoints)
            {
                ballBrush=(HBRUSH)GetStockObject(WHITE_BRUSH);
                ballPen=(HPEN)GetStockObject(WHITE_PEN);
            }
            else
            {
                ballPen=(HPEN)GetStockObject(BLACK_PEN);
                ballBrush=(HBRUSH)GetStockObject(BLACK_BRUSH);
            }
            oldBrush= (HBRUSH)SelectObject(hdc,ballBrush);
            oldPen= (HPEN)SelectObject(hdc,ballPen);
            Ellipse(hdc,_position.GetX()-_diameter/2,_position.GetY()-_diameter/2,_position.GetX()+_diameter/2,_position.GetY()+_diameter/2);

            SelectObject(hdc,oldPen);
            SelectObject(hdc,oldBrush);
            DeleteObject(ballBrush);
        }

    protected:
        double _diameter;
};

double CircleObject::GetDiameter(){
    return _diameter;
}

void CircleObject::SetDiameter(double d){
    _diameter=d;
}

void CircleObject::setPosition(Vector2D position){
    _position = position;
}

class PlayerObject:public GameObject{
    public:
        PlayerObject(RECT* rect,Vector2D pos,double number) {
            SetRect(&_playerObject,rect->left,rect->top,rect->right,rect->bottom);
            _position = pos;
            _playerNo = number;
            _width = rect->right-rect->left;
            _height = rect->bottom - rect->top;
        }
        PlayerObject() = default;

        double GetHeight();
        RECT& GetPlayerObject();
        double GetWidth();
        void SetWidth(double);
        void SetHeight(double);
        void SetPlayerObject(RECT&);
        double GetPlayerNumber();
        void SetPlayerNumber(double);
        void setPosition(Vector2D);

        void checkInput(){
            HWND hwnd=GetFocus();
            if(hwnd){
                RECT r;
                GetClientRect(hwnd,&r);

                if(_playerNo==1){ //Ako je u pitanju prvi igraè
                    if(PRITISNUTO(VK_UP)){// Ako je pritisnuta strelica gore
                        if(_position.GetX()+_width/2>=ballPosition.x-ballDiameter/2&&_position.GetX()-_width/2<=ballPosition.x+ballDiameter/2){// Ako je igraè kod loptice
                            if(_position.GetY()-_width/2-ballDiameter>=r.bottom/2){// Ako ide loptica pa igraè (odozgo ka dole) i nisu odmah uz gornju polovinu igrališta
                                _position.SetY(_position.GetY()-playerAcceleration);//igraè ide gore
                            }
                            if(ballPosition.y+ballDiameter/2>_position.GetY()+_width/2&&_position.GetY()-lineThickness/2-_width/2>r.bottom/2){// Ako je prvo igraè pa loptica igraè se može pomjeriti do sredine
                                _position.SetY(_position.GetY()-playerAcceleration);
                            }
                        }

                        else//u suprotnom izvrši normalno kretanje ka gore
                        {
                             if(_position.GetY()-lineThickness-_width/2>r.bottom/2){
                                _position.SetY(_position.GetY()-playerAcceleration);
                            }
                        }

                    }

                    else if(PRITISNUTO(VK_DOWN)){
                        if(_position.GetX()+_width/2>=ballPosition.x-ballDiameter/2&&_position.GetX()-_width/2<=ballPosition.x+ballDiameter/2){
                            if(_position.GetY()+_width/2+ballDiameter+lineThickness/2<=r.bottom){
                                _position.SetY(_position.GetY()+playerAcceleration);
                            }
                            if(ballPosition.y+ballDiameter/2<_position.GetY()-_width/2&&_position.GetY()+lineThickness/2+_width/2<r.bottom){
                                _position.SetY(_position.GetY()+playerAcceleration);
                            }
                        }
                        else
                        {
                             if(_position.GetY()+lineThickness/2+_width/2<r.bottom){
                                _position.SetY(_position.GetY()+playerAcceleration);
                             }
                        }
                    }

                    else if(PRITISNUTO(VK_LEFT)){//Ako je pritisnuta strelica na lijevo
                        if(_position.GetY()-_width/2<=ballPosition.y+ballDiameter/2&&_position.GetY()+_width/2>=ballPosition.y-ballDiameter/2){// Ako je igraè kod loptice
                            if(_position.GetX()-_width/2-ballDiameter-lineThickness/2>=r.left){// Ako ide loptica pa igraè (s ijeva na desno) i nisu odmah uz lijevi zid
                                _position.SetX(_position.GetX()-playerAcceleration); //Poveæaj poziciju igraèa za 4 u lijevo
                            }
                            if(ballPosition.x+ballDiameter/2>_position.GetX()+_width/2&&_position.GetX()-lineThickness/2-_width/2>r.left){// Ako je prvo igraè pa loptica igraè se može pomjeriti do zida
                                _position.SetX(_position.GetX()-playerAcceleration);
                            }
                        }
                        else// u sprotnom ide normalno pomjeranje u lijevo
                        {
                             if(_position.GetX()-lineThickness/2-_width/2>r.left){
                                _position.SetX(_position.GetX()-playerAcceleration);
                            }
                        }

                    }

                    else if(PRITISNUTO(VK_RIGHT)){
                        if(_position.GetY()-_width/2<=ballPosition.y+ballDiameter/2&&_position.GetY()+_width/2>=ballPosition.y-ballDiameter/2){
                            if(_position.GetX()+_width/2+ballDiameter+lineThickness/2<=r.right){
                                _position.SetX(_position.GetX()+playerAcceleration);
                            }
                            if(ballPosition.x+ballDiameter/2<_position.GetX()-_width/2&&_position.GetX()+lineThickness/2+_width/2<r.right){
                                _position.SetX(_position.GetX()+playerAcceleration);
                            }
                        }
                        else
                        {
                             if(_position.GetX()+lineThickness/2+_width/2<r.right){
                                _position.SetX(_position.GetX()+playerAcceleration);
                             }
                        }

                    }

                    else{
                        _position.SetX(_position.GetX());
                        _position.SetY(_position.GetY());
                    }
                }

                else if(_playerNo==2){
                    if(PRITISNUTO(W)){
                         if(_position.GetX()+_width/2>=ballPosition.x-ballDiameter/2&&_position.GetX()-_width/2<=ballPosition.x+ballDiameter/2){
                            if(_position.GetY()-_width/2-ballDiameter-lineThickness/2>=r.top){
                                _position.SetY(_position.GetY()-playerAcceleration);
                            }
                            if(ballPosition.y+ballDiameter/2>_position.GetY()+_width/2&&_position.GetY()-lineThickness/2-_width/2>r.top){
                                _position.SetY(_position.GetY()-playerAcceleration);
                            }
                        }
                        else
                        {
                             if(_position.GetY()-lineThickness/2-_width/2>r.top){
                                _position.SetY(_position.GetY()-playerAcceleration);
                            }
                        }
                    }

                    else if(PRITISNUTO(S)){
                        if(_position.GetX()+_width/2>=ballPosition.x-ballDiameter/2&&_position.GetX()-_width/2<=ballPosition.x+ballDiameter/2){
                            if(_position.GetY()+_width/2+ballDiameter<=r.bottom/2){
                                _position.SetY(_position.GetY()+playerAcceleration);
                            }
                            if(ballPosition.y+ballDiameter/2<_position.GetY()-_width/2&&_position.GetY()+lineThickness/2+_width/2<r.bottom/2){
                                _position.SetY(_position.GetY()+playerAcceleration);
                            }
                        }
                        else{
                            if(_position.GetY()+lineThickness+_width/2<r.bottom/2){
                                _position.SetY(_position.GetY()+playerAcceleration);
                            }
                        }
                    }

                    else if(PRITISNUTO(A)){
                        if(_position.GetY()-_width/2<=ballPosition.y+ballDiameter/2&&_position.GetY()+_width/2>=ballPosition.y-ballDiameter/2){
                            if(_position.GetX()-_width/2-ballDiameter-lineThickness/2>=r.left){
                                _position.SetX(_position.GetX()-playerAcceleration);
                            }
                            if(ballPosition.x+ballDiameter/2>_position.GetX()+_width/2&&_position.GetX()-lineThickness/2-_width/2>r.left){
                                _position.SetX(_position.GetX()-playerAcceleration);
                            }
                        }
                        else
                        {
                             if(_position.GetX()-lineThickness/2-_width/2>r.left){
                                _position.SetX(_position.GetX()-playerAcceleration);
                            }
                        }
                    }

                    else if(PRITISNUTO(D)){
                        if(_position.GetY()-_width/2<=ballPosition.y+ballDiameter/2&&_position.GetY()+_width/2>=ballPosition.y-ballDiameter/2){
                            if(_position.GetX()+_width/2+ballDiameter+lineThickness/2<=r.right){
                                _position.SetX(_position.GetX()+playerAcceleration);
                            }
                            if(ballPosition.x+ballDiameter/2<_position.GetX()-_width/2&&_position.GetX()+lineThickness/2+_width/2<r.right){
                                _position.SetX(_position.GetX()+playerAcceleration);
                            }
                        }
                        else
                        {
                             if(_position.GetX()+lineThickness/2+_width/2<r.right){
                                _position.SetX(_position.GetX()+playerAcceleration);
                             }
                        }
                    }

                    else{
                        _position.SetX(_position.GetX());
                        _position.SetY(_position.GetY());
                    }
                }
            }
        }

        void render(HDC hdc){
            HBRUSH insideBrush=(HBRUSH)GetStockObject(HOLLOW_BRUSH);
            oldBrush= (HBRUSH)SelectObject(hdc,insideBrush);
            if(_playerNo==1){
                color=colorP1;
            }
            else{
                color=colorP2;
            }
            currentPen=CreatePen(PS_SOLID,1,color);
            oldPen=(HPEN)SelectObject(hdc,currentPen);
            Rectangle(hdcMem,_position.GetX()-_width/2,_position.GetY()-_height/2,_position.GetX() +_width/2,_position.GetY()+_height/2);
            Rectangle(hdcMem,_position.GetX()-_width/2 +lineThickness/2,_position.GetY()-_height/2+lineThickness/2,_position.GetX()+_width/2-lineThickness/2,_position.GetY()+_height/2-lineThickness/2);

            SelectObject(hdc,oldBrush);

            DeleteObject(insideBrush);
            insideBrush=(HBRUSH)CreateSolidBrush(color);
            oldBrush= (HBRUSH)SelectObject(hdc,insideBrush);
            Rectangle(hdcMem,_position.GetX()-_width/2+lineThickness,_position.GetY()-_height/2+lineThickness,_position.GetX()+_width/2 -lineThickness,_position.GetY()+_height/2-lineThickness);

            SelectObject(hdc,oldPen);
            SelectObject(hdc,oldBrush);

            DeleteObject(insideBrush);
            DeleteObject(currentPen);
        }


        void update(){



        }

    protected:
        RECT _playerObject;
        double _width;
        double _height;
        double _playerNo;
};

RECT& PlayerObject::GetPlayerObject(){
    return _playerObject;
}

double PlayerObject::GetWidth(){
    return _width;
}

double PlayerObject::GetHeight(){
    return _height;
}

double PlayerObject::GetPlayerNumber(){
    return _playerNo;
}

void PlayerObject::SetWidth(double width){
    _width=width;
}

void PlayerObject::SetHeight(double height){
    _height=height;
}

void PlayerObject::SetPlayerObject(RECT& r){
    _playerObject=r;
}

void PlayerObject::SetPlayerNumber(double number){
    _playerNo=number;
}

void PlayerObject::setPosition(Vector2D position){
    _position = position;
}

void renderField(HWND hwnd,HDC hdcMem){//nacrtaj polje
    RECT rect;
    GetClientRect(hwnd,&rect);
    currentPen=(HPEN)CreatePen(PS_SOLID,lineThickness,RGB(128,128,128));
    oldPen= (HPEN)SelectObject(hdcMem,currentPen);
    MoveToEx(hdcMem,rect.left,(rect.bottom-rect.top)/2,NULL);
    LineTo(hdcMem,rect.right,(rect.bottom-rect.top)/2);
    Ellipse(hdcMem,(rect.right-rect.left)/2-centerRadius/2,(rect.bottom-rect.top)/2-centerRadius/2,(rect.right-rect.left)/2+centerRadius/2,(rect.bottom-rect.top)/2+centerRadius/2);
    SelectObject(hdcMem,oldPen);
    DeleteObject(currentPen);

    currentPen=(HPEN)CreatePen(PS_SOLID,lineThickness,RGB(255,0,0));
    oldPen= (HPEN)SelectObject(hdcMem,currentPen);
    MoveToEx(hdcMem,(rect.right-rect.left)/2-goalWidth,rect.top,NULL);
    LineTo(hdcMem,rect.left,rect.top);
    LineTo(hdcMem,rect.left,(rect.bottom-rect.top)/2);
    SelectObject(hdcMem,oldPen);
    DeleteObject(currentPen);

    currentPen=(HPEN)CreatePen(PS_SOLID,lineThickness,RGB(0,0,255));
    oldPen= (HPEN)SelectObject(hdcMem,currentPen);
    LineTo(hdcMem,rect.left,rect.bottom);
    LineTo(hdcMem,(rect.right-rect.left)/2-goalWidth,rect.bottom);
    SelectObject(hdcMem,oldPen);
    DeleteObject(currentPen);

    currentPen=(HPEN)CreatePen(PS_SOLID,lineThickness,RGB(0,255,0));
    oldPen= (HPEN)SelectObject(hdcMem,currentPen);
    MoveToEx(hdcMem,(rect.right-rect.left)/2+goalWidth,rect.bottom,NULL);
    LineTo(hdcMem,rect.right,rect.bottom);
    LineTo(hdcMem,rect.right,rect.bottom/2);
    SelectObject(hdcMem,oldPen);
    DeleteObject(currentPen);

    currentPen=(HPEN)CreatePen(PS_SOLID,lineThickness,RGB(255,255,0));
    oldPen= (HPEN)SelectObject(hdcMem,currentPen);
    LineTo(hdcMem,rect.right,rect.top);
    LineTo(hdcMem,(rect.right-rect.left)/2+goalWidth,rect.top);
    SelectObject(hdcMem,oldPen);
    DeleteObject(currentPen);
    drawGoal(hdcMem);


}

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WindowProcedureMainWindow (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WindowProcedureAboutWindow (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WindowProcedureOptionsWindow (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");

HWND hwndMenu;
HWND hwndPlay;
HWND hwndAbout;
HWND hwndOptions;
int play;

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND mainHwnd;
    HWND aboutHwnd;
    HWND optionsHwnd;
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = "Hockey";
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (hThisInstance, MAKEINTRESOURCE(IDI_ICON));
    wincl.hIconSm = LoadIcon (NULL, MAKEINTRESOURCE(IDI_ICON));
    wincl.hCursor = LoadCursorFromFile("hockey.cur");
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    wincl.lpfnWndProc = WindowProcedureMainWindow;
    wincl.lpszClassName = "Menu";
    wincl.hCursor = LoadCursorFromFile("hockey.cur");;

    if(!RegisterClassEx(&wincl))
        return 0;

    wincl.lpfnWndProc = WindowProcedureAboutWindow;
    wincl.lpszClassName = "About";

    if(!RegisterClassEx(&wincl))
        return 0;

    wincl.lpfnWndProc = WindowProcedureOptionsWindow;
    wincl.lpszClassName = "Options";

    if(!RegisterClassEx(&wincl))
        return 0;

    /* The class is registered, let's create the program*/

    mainHwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           "Menu",         /* Classname */
           _T("Menu"),       /* Title Text */
           WS_CAPTION | WS_SYSMENU, /* default window */
           500,       /* Windows decides the position */
           20,       /* where the window ends up on the screen */
           528,                 /* The programs width */
           683,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    hwndMenu = mainHwnd;
    ShowWindow (mainHwnd, nCmdShow);


    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           "Hockey",         /* Classname */
           _T("Hockey"),       /* Title Text */
           WS_CAPTION | WS_SYSMENU, /* default window */
           500,       /* Windows decides the position */
           20,       /* where the window ends up on the screen */
           528,                 /* The programs width */
           683,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           (HINSTANCE)GetWindowLong(hwndMenu,GWLP_HINSTANCE),       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    hwndPlay = hwnd;
    play = nCmdShow;

    aboutHwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           "About",         /* Classname */
           _T("About"),       /* Title Text */
           WS_CAPTION | WS_SYSMENU, /* default window */
           504,       /* Windows decides the position */
           15,       /* where the window ends up on the screen */
           520,                 /* The programs width */
           700,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           (HINSTANCE)GetWindowLong(hwndMenu,GWLP_HINSTANCE),       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    hwndAbout = aboutHwnd;

    optionsHwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           "Options",         /* Classname */
           _T("Options"),       /* Title Text */
           WS_CAPTION | WS_SYSMENU, /* default window */
           595,       /* Windows decides the position */
           5,       /* where the window ends up on the screen */
           332,                 /* The programs width */
           720,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           (HINSTANCE)GetWindowLong(hwndMenu,GWLP_HINSTANCE),       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    hwndOptions = optionsHwnd;
    /* Run the message loop. It will run until GetMessage() returns 0 */

    RECT client;

    GetClientRect(hwnd,&client);

    Vector2D pozicijaLoptice((client.right-client.left)/2,(client.bottom-client.top)/2);
    Vector2D pozicijaPrvogIgraca((client.right-client.left)/2,client.bottom-playerFromEdge);
    Vector2D pozicijaDrugogIgraca((client.right-client.left)/2,client.top+playerFromEdge-2);
    Vector2D ubrzanje(0,0);
    RECT rect;
    SetRect(&rect,20,20,60,60);
    CircleObject loptica;
    PlayerObject igrac1(&rect,pozicijaPrvogIgraca,1);
    PlayerObject igrac2(&rect,pozicijaDrugogIgraca,2);
    bg_music(hwnd);


    while (TRUE)
    {
        DWORD start_time;
        if(PeekMessage(&messages, NULL, 0, 0, PM_REMOVE))
        {
            if(messages.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&messages);
            DispatchMessage(&messages);
        }

        start_time = GetTickCount();


        RECT clientRectangle;
        if(mainDraw){
            HDC hdc = GetDC(mainHwnd);
            GetClientRect(mainHwnd, &clientRectangle);
            DrawBkg(hdc, &clientRectangle);
            ReleaseDC(mainHwnd, hdc);
            mainDraw=false;
        }
        if(create == true){
            igrac1.setPosition(pozicijaPrvogIgraca);
            igrac2.setPosition(pozicijaDrugogIgraca);
            loptica.SetDiameter(ballRadius);
            loptica.SetGameObjectPosition(pozicijaLoptice);
            loptica.SetGameObjectAccelerization(ubrzanje);
            loptica.setPosition(pozicijaLoptice);
            loptica.SetGameObjectAccelerization(ubrzanje);
            start = 0;
            scorePlayerOne = 0;
            scorePlayerTwo = 0;

            create = false;
        }

    //Iscrtavanje polja, kretanje loptice, kretanja igraèa
    /**/
        if(scorePlayerOne < playPoints && scorePlayerTwo < playPoints){
            if(gameActive){
                HDC hdc = GetDC(hwnd);
                    if(numberOver){

                    igrac1.checkInput();

                    igrac2.checkInput();

                    igrac1.update();
                    igrac2.update();
                    playerOnePosition.x=igrac1.GetGameObjectPosition().GetX();
                    playerOnePosition.y=igrac1.GetGameObjectPosition().GetY();
                    playerTwoPosition.x=igrac2.GetGameObjectPosition().GetX();
                    playerTwoPosition.y=igrac2.GetGameObjectPosition().GetY();
                    playerWidth=igrac1.GetWidth();
                    playerHeight=igrac1.GetHeight();
                    loptica.checkInput();

                    loptica.update();
                    ballPosition.x=loptica.GetGameObjectPosition().GetX();
                    ballPosition.y=loptica.GetGameObjectPosition().GetY();
                    ballDiameter=loptica.GetDiameter();
                    }
                    RECT rect,rectDraw;
                    hdcMem=CreateCompatibleDC(hdc);
                    hdcBuffer=CreateCompatibleDC(hdc);
                    HWND hwnd = WindowFromDC(hdc);
                    GetClientRect(hwnd,&rect);
                    GetClientRect(hwnd,&rectDraw);

                    hbmBuffer=CreateCompatibleBitmap(hdc,rect.right,rect.bottom);
                    hbmMem=CreateCompatibleBitmap(hdc,rectDraw.right,rectDraw.bottom);

                    HBITMAP hbmOldBuffer=(HBITMAP)SelectObject(hdcBuffer,(HBITMAP)hbmBuffer);
                    HBITMAP hbmOldMem=(HBITMAP)SelectObject(hdcMem,(HBITMAP)hbmMem);

                    FillRect(hdcBuffer,&rect,(HBRUSH)GetStockObject(WHITE_BRUSH));
                    FillRect(hdcMem,&rectDraw,(HBRUSH)GetStockObject(WHITE_BRUSH));



                    if(!numberOver)
                    {
                        drawNumbers(hdcMem,hdcBuffer);
                    }
                    else
                    {
                        renderField(hwnd,hdcMem);

                        igrac1.render(hdcMem);

                        igrac2.render(hdcMem);
                        loptica.render(hdcMem);

                        memset(poruka1, 0, sizeof(poruka1));
                        memset(poruka2, 0, sizeof(poruka2));
                        sprintf(poruka1, "%s: %d\n", playerOneName, scorePlayerOne);
                        sprintf(poruka2, "%s: %d", playerTwoName, scorePlayerTwo);
                        SetTextColor(hdcMem,colorP1);
                        SetBkMode(hdcMem,TRANSPARENT);
                        SelectObject(hdcMem, oldFont);
                        DeleteObject(font);
                        TextOut(hdcMem, 400, 10, poruka1, 15);
                        SetTextColor(hdcMem,colorP2);
                        TextOut(hdcMem, 400, 30, poruka2, 15);
                        SetTextColor(hdcMem,RGB(0,0,0));
                        SelectObject(hdcMem,oldBrush);
                        SelectObject(hdcMem,oldPen);
                        BitBlt(hdcBuffer,rect.left,rect.top,rect.right,rect.bottom,hdcMem,0,0,SRCCOPY);

                    }

                    BitBlt(hdc,0,0,rect.right,rect.bottom,hdcBuffer,0,0,SRCCOPY);
                    SelectObject(hdcMem,hbmOldMem);
                    SelectObject(hdcMem,hbmOldBuffer);
                    DeleteObject(hbmBuffer);
                    DeleteObject(hbmMem);
                    DeleteDC(hdcBuffer);
                    DeleteDC(hdcMem);

                    ReleaseDC(hwnd,hdc);

            }


        }
        else if(scorePlayerOne == playPoints || scorePlayerTwo == playPoints){
           char poruka3[30];

           RECT clientRect;
           HDC hdc = GetDC(hwnd);
           GetClientRect(hwnd, &clientRect);
           if(scorePlayerOne == playPoints){
                SetTextColor(hdc,colorP1);
                sprintf(poruka3, "%s is the Winner!", playerOneName);
           }
           if(scorePlayerTwo == playPoints){
                SetTextColor(hdc,colorP2);
                sprintf(poruka3, "%s is the Winner!", playerTwoName);
           }
           HFONT font = CreateFont(35, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
           SelectObject(hdc, font);
           SetBkMode(hdc, TRANSPARENT);
           DrawText(hdc, poruka3, strlen(poruka3), &clientRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
           memset(poruka3, 0, sizeof(poruka3));
           SetTextColor(hdc,RGB(0,0,0));
           ReleaseDC(hwnd, hdc);
        }

        /**/
        DWORD frame_time = GetTickCount() - start_time;
        if(frame_time < _frameDelay)
        {
            Sleep(_frameDelay - frame_time);
        }
    }

    return messages.wParam;
}

bool check_if_in_range(float a, float min, float max){// Funkcija koja provjerava da li se dati broj nalazi u nekom skupu
    if (a <= max && !(a < min)) {
        return true;
    }

    return false;
}

void bg_music(HWND hwnd){
    mciSendString("open happy_bg.mp3 type mpegvideo alias myFile", NULL, 0, 0);
    mciSendString("play myFile repeat", NULL, 0, hwnd );
    sound=true;
}

void stop_music(HWND hwnd){
    mciSendString("close happy_bg.mp3 type mpegvideo alias myFile", NULL, 0, 0);
    mciSendString("stop myFile", NULL, 0, hwnd );
    sound=false;
}

void shot(){
    PlaySound((LPCSTR) "boing.wav", NULL, SND_FILENAME | SND_ASYNC);
}

void shot_big(){
    PlaySound((LPCSTR) "boing_big.wav", NULL, SND_FILENAME | SND_ASYNC);
}

void shot_small(){
    PlaySound((LPCSTR) "boing_small.wav", NULL, SND_FILENAME | SND_ASYNC);
}

void play_shot(){
    if(ballRadius>=15&&ballRadius<26){
        shot_small();
    }
    else if(ballRadius>=26&&ballRadius<37){
        shot();
    }
    else{
        shot_big();
    }
}

void countdown(){
    PlaySound((LPCSTR) "countdown.wav", NULL, SND_FILENAME | SND_ASYNC);
}

void countdown_end(){
    PlaySound((LPCSTR) "countdownend.wav", NULL, SND_FILENAME | SND_ASYNC);
}

void goal_bar_shot(){
    PlaySound((LPCSTR) "goal_bar_boing.wav", NULL, SND_FILENAME | SND_ASYNC);
}

void congrats(){
    PlaySound((LPCSTR) "congrats.wav", NULL, SND_FILENAME | SND_ASYNC);
}

void goal(){
    PlaySound((LPCSTR) "goal.wav", NULL, SND_FILENAME | SND_ASYNC);
}

void DrawBkg(HDC hdc, RECT* prect)
{
    HDC hdcBuffer = CreateCompatibleDC(hdc);
    HBITMAP hbmBuffer = CreateCompatibleBitmap(hdc, prect->right, prect->bottom);
    HBITMAP hbmOldBuffer = (HBITMAP)SelectObject(hdcBuffer,hbmBuffer);

    HDC hdcMem = CreateCompatibleDC(hdc);

    HBITMAP hbmOld1 = (HBITMAP)SelectObject(hdcMem, hbmBkg);
    BitBlt(hdcBuffer, bkg.x, bkg.y, bkg.width, bkg.height, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdc, hbmOld1);

    BitBlt(hdc, 0, 0, prect->right, prect->bottom, hdcBuffer, 0, 0, SRCCOPY);

    SelectObject(hdcMem, hbmOld1);
    DeleteObject(hdcMem);
    DeleteObject(hbmBuffer);

    SelectObject(hdcBuffer, hbmOldBuffer);
    DeleteObject(hdcBuffer);
    DeleteObject(hbmOldBuffer);

}

void SetPlayerColors(){

    srand(time(NULL));
    int randomP1=rand()%4+1;
    int randomP2=rand()%4+1;

    while(randomP1==randomP2){
        randomP2=rand()%4;
    }
    if(randomP1==1){
        colorP1=RGB(255,0,0);
        color1=colorP1;
    }
    else if(randomP1==2){
        colorP1=RGB(0,255,0);
        color1=colorP1;
    }
    else if(randomP1==3){
        colorP1=RGB(255,255,0);
        color1=colorP1;
    }
    else{
        colorP1=RGB(0,0,255);
        color1=colorP1;
    }
    if(randomP2==1){
        colorP2=RGB(255,0,0);
        color2=colorP2;
    }
    else if(randomP2==2){
        colorP2=RGB(0,255,0);
        color2=colorP2;
    }
    else if(randomP2==3){
        colorP2=RGB(255,255,0);
        color2=colorP2;
    }
    else{
        colorP2=RGB(0,0,255);
        color2=colorP2;
    }
}

/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedureMainWindow (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
        case WM_CREATE:
        {
            SetPlayerColors();

			BITMAP bitmap;
            hbmBkg = (HBITMAP)LoadImage(NULL, "iceHockey.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

            GetObject(hbmBkg, sizeof(BITMAP), &bitmap);

            bkg.width=bitmap.bmWidth;
            bkg.height=bitmap.bmHeight;
            bkg.dx=0;
            bkg.dy=0;

            mainDraw=true;

            HINSTANCE hInst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
            CreateWindow("button", "Play", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_PUSHLIKE,
                         350, 230, 150, 50, hwnd, (HMENU)PLAY_BUTTON, hInst, NULL);
            CreateWindow("button", "Options", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_PUSHLIKE,
                         350, 290, 150, 50, hwnd, (HMENU)OPTIONS_BUTTON, hInst, NULL);
            CreateWindow("button", "About", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_PUSHLIKE,
                         350, 350, 150, 50, hwnd, (HMENU)ABOUT_BUTTON, hInst, NULL);
            CreateWindow("button", "Exit", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_PUSHLIKE,
                         350, 410, 150, 50, hwnd, (HMENU)EXIT_BUTTON, hInst, NULL);
            break;
        }
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case PLAY_BUTTON:
                {
                    if(isPlayerSet){
                        UncheckSelectedButton(hwnd, (HWND)lParam);
                        create = true;
                        ShowWindow (hwndPlay, play);
                        gameActive=true;
                    }
                    else{
                        MessageBox(NULL,(LPCSTR)"Player names are not set! You must enter them in Player One and Player Two fields in options, and then you can try again!",(LPCSTR)"Player Names Not Set",MB_ICONWARNING | MB_OK | MB_DEFBUTTON1);
                    }

                    break;
                }

                case OPTIONS_BUTTON:
                {
                    UncheckSelectedButton(hwnd, (HWND)lParam);
                    ShowWindow (hwndOptions, play);
                    break;
                }

                case ABOUT_BUTTON:
                {
                    UncheckSelectedButton(hwnd, (HWND)lParam);
                    ShowWindow (hwndAbout, play);
                    break;
                }
                case EXIT_BUTTON:
                {
                    DestroyWindow(hwnd);
                    break;
                }
            }
        break;
        }
        case WM_CLOSE:{
            memset(playerOneName, 0, sizeof(playerOneName));
            memset(playerOneNameBuffer, 0, sizeof(playerOneNameBuffer));
            memset(playerTwoName, 0, sizeof(playerTwoName));
            memset(playerTwoNameBuffer, 0, sizeof(playerTwoNameBuffer));
            DestroyWindow(hwnd);
            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_DESTROY:
		{
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
           break;
		}
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
void drawGoal(HDC hdcMem){//Funkcija za iscrtavanje poruke G O A L na ekranu

    RECT r;
    GetClientRect(hwndPlay,&r); //Dohvati dimenzije prozora u r

    if(drawGoalOne&&scorePlayerTwo>0&&scorePlayerTwo<playPoints){ //Ako se treba iscrtati G O A L na golu prvog igrača i ako su osvojeni poeni drugog igrača manji od podešenog broja i veći od 0 (koji se može namjestit u opcijama a pri pokretanju igre, ako se tamo ne postavi bude 5)
        SetBkMode(hdcMem, TRANSPARENT); // Pozadina teksta treba biti providna
        r.top+=550; //Pomjeri vrh i lijevu stranu prozora za iscrtavanje u kojem ce se iscrtat poruka za navedene vrijednosti
        r.left+=180;
        j++;    // Povecaj za 1 pomocnu varijablu
        font = CreateFont(60, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial"); //Podesi font i njegovu velicinu
        oldFont=(HFONT)SelectObject(hdcMem, font); // Ubaci ga u hdcMem ali ostavi stari koji je bio, da se poslije moze vratit
        if(j>0&&j<=81){ //Dok j izbroji od 0 do 81
            SetTextColor(hdcMem,RGB(0,255,0)); //Zelena boja teksta
            sprintf(goal_char,"G"); //Upisi u goal_char slovo G
            DrawText(hdcMem, goal_char, strlen(goal_char), &r, DT_LEFT); //Ispisi ga takvog na hdc
        }
        if(j>21&&j<=81){// dok j izbroji od 21 do 81
            r.left+=45; //pomjeri prozor za iscrtavanje ulijevo za 45
            SetTextColor(hdcMem,RGB(255,255,0)); // Oboji zuto tekst i ostalo je slicno
            sprintf(goal_char,"O");
            DrawText(hdcMem, goal_char, strlen(goal_char), &r, DT_LEFT);
        }
        if(j>41&&j<=81){
            r.left+=45;
            SetTextColor(hdcMem,RGB(255,0,0));
            sprintf(goal_char,"A");
            DrawText(hdcMem, goal_char, strlen(goal_char), &r, DT_LEFT);
        }
        if(j>61&&j<=81){
            r.left+=45;
            SetTextColor(hdcMem,RGB(0,0,255));
            sprintf(goal_char,"L");
            DrawText(hdcMem, goal_char, strlen(goal_char), &r, DT_LEFT);
        }
        if(j>81){
            j=0;
            SetTextColor(hdcMem,RGB(0,0,0));
            drawGoalOne=false;
        }
    }
    if(drawGoalTwo&&scorePlayerOne>0&&scorePlayerOne<playPoints){
        SetBkMode(hdcMem, TRANSPARENT);
        r.top+=50;
        r.left+=180;
        j++;
        font = CreateFont(60, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
        oldFont=(HFONT)SelectObject(hdcMem, font);
        if(j>0&&j<=81){
            SetTextColor(hdcMem,RGB(0,255,0));
            sprintf(goal_char,"G");
            DrawText(hdcMem, goal_char, strlen(goal_char), &r, DT_LEFT);
        }
        if(j>21&&j<=81){
            r.left+=45;
            SetTextColor(hdcMem,RGB(255,255,0));
            sprintf(goal_char,"O");
            DrawText(hdcMem, goal_char, strlen(goal_char), &r, DT_LEFT);
        }
        if(j>41&&j<=81){
            r.left+=45;
            SetTextColor(hdcMem,RGB(255,0,0));
            sprintf(goal_char,"A");
            DrawText(hdcMem, goal_char, strlen(goal_char), &r, DT_LEFT);
        }
        if(j>61&&j<=81){
            r.left+=45;
            SetTextColor(hdcMem,RGB(0,0,255));
            sprintf(goal_char,"L");
            DrawText(hdcMem, goal_char, strlen(goal_char), &r, DT_LEFT);
        }
        if(j>81){
            j=0;
            SetTextColor(hdcMem,RGB(0,0,0));
            drawGoalTwo=false;
        }

    }
}

void drawNumbers(HDC hdcMem,HDC hdcBuffer){

    i++;
    if(i>=1&&i<30){

        hbmField = (HBITMAP)LoadImage(NULL, "field.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        number=(HBITMAP)LoadImage(NULL,"three.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
        numberMask=(HBITMAP)LoadImage(NULL,"threeMask.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
        if(i==1)
        {
            if(sound==true)
            {
                countdown();
            }
        }
    }
    else if(i>=31&&i<60){

        hbmField = (HBITMAP)LoadImage(NULL, "field2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        number=(HBITMAP)LoadImage(NULL,"two.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
        numberMask=(HBITMAP)LoadImage(NULL,"twoMask.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
        if(i==31)
        {
            if(sound==true)
            {
                countdown();
            }
        }
    }
    else if(i>=61&&i<90)
    {
        hbmField = (HBITMAP)LoadImage(NULL, "field3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        number=(HBITMAP)LoadImage(NULL,"one.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
        numberMask=(HBITMAP)LoadImage(NULL,"oneMask.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
        if(i==61)
        {
            if(sound==true)
            {
                countdown();
            }
        }
    }
    else if(i>=91&&i<120)
    {
        hbmField = (HBITMAP)LoadImage(NULL, "field4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        numberMask=(HBITMAP)LoadImage(NULL,"zeroMask.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
        number=(HBITMAP)LoadImage(NULL,"zero.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
        if(i==91)
        {
            if(sound==true)
            {
                countdown();
            }
        }
    }
    else
    {
        if(i==121)
        {
            i=0;
            if(sound==true)
            {
                countdown_end();
            }
            hbmField = (HBITMAP)LoadImage(NULL, "field.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
            numberOver=true;
        }
    }

    RECT client;
    GetClientRect(hwndPlay,&client);

    GetObject(hbmField, sizeof(BITMAP), &bmp);

    field.width=bmp.bmWidth;
    field.height=bmp.bmHeight;
    field.dx=0;
    field.dy=0;


    HBITMAP hbmOld1 = (HBITMAP)SelectObject(hdcMem, hbmField);
    BitBlt(hdcBuffer, field.x, field.y, field.width, field.height, hdcMem, 0, 0, SRCCOPY);


    GetObjectA(number,sizeof(BITMAP),&bmp);
    numberInfo.width=bmp.bmWidth;
    numberInfo.height=bmp.bmHeight;
    numberInfo.x=((client.right-client.left)/2-bmp.bmHeight/2)+numberOffset;
    numberInfo.y=((client.bottom-client.top)/2-bmp.bmWidth/2)-numberOffset;
    numberInfo.dx=0;
    numberInfo.dy=0;

    (HBITMAP)SelectObject(hdcMem,numberMask);
    BitBlt(hdcBuffer,numberInfo.x,numberInfo.y,numberInfo.width,numberInfo.height,hdcMem,0,0,SRCPAINT);
    (HBITMAP)SelectObject(hdcMem,number);
    BitBlt(hdcBuffer,numberInfo.x,numberInfo.y,numberInfo.width,numberInfo.height,hdcMem,0,0,SRCAND);

    SelectObject(hdcMem, hbmOld1);
}

void UncheckSelectedButton(HWND hwnd, HWND hwndOfCurrentButton)
{
    HWND hwndPlayButton = GetDlgItem(hwnd, PLAY_BUTTON);
    HWND hwndSetButton = GetDlgItem(hwnd, SET_BUTTON);

    if(hwndPlayButton != hwndOfCurrentButton)
    {
        if(SendMessage(hwndPlayButton, BM_GETCHECK, 0, 0))
        {
            SendMessage(hwndPlayButton, BM_SETCHECK, !SendMessage(hwndPlayButton, BM_GETCHECK, 0, 0), 0);
        }
    }

    if(hwndSetButton != hwndOfCurrentButton)
    {
        if(SendMessage(hwndSetButton, BM_GETCHECK, 0, 0))
        {
            SendMessage(hwndSetButton, BM_SETCHECK, !SendMessage(hwndSetButton, BM_GETCHECK, 0, 0), 0);
        }
    }
}

int CheckDifficulty(HWND hwnd,HWND hwndControl)
{
    HWND hwndLazy= GetDlgItem(hwnd, LAZY_BUTTON);
    HWND hwndEasy= GetDlgItem(hwnd, EASY_BUTTON);
    HWND hwndNormal= GetDlgItem(hwnd, NORMAL_BUTTON);
    HWND hwndHard= GetDlgItem(hwnd, HARD_BUTTON);
    HWND hwndInsane= GetDlgItem(hwnd, INSANE_BUTTON);

    if(hwndLazy==hwndControl&&SendMessage(hwndLazy, BM_GETCHECK,0,0)){
        SendMessage(hwndLazy, BM_SETCHECK, !SendMessage(hwndLazy, BM_GETCHECK,0,0),0);
    }
    if(hwndEasy==hwndControl&&SendMessage(hwndEasy, BM_GETCHECK,0,0)){
        SendMessage(hwndEasy, BM_SETCHECK, !SendMessage(hwndEasy, BM_GETCHECK,0,0),0);
    }
    if(hwndNormal==hwndControl&&SendMessage(hwndNormal, BM_GETCHECK,0,0)){
        SendMessage(hwndNormal, BM_SETCHECK, !SendMessage(hwndNormal, BM_GETCHECK,0,0),0);
    }
    if(hwndHard==hwndControl&&SendMessage(hwndHard, BM_GETCHECK,0,0)){
        SendMessage(hwndHard, BM_SETCHECK, !SendMessage(hwndHard, BM_GETCHECK,0,0),0);
    }
    if(hwndInsane==hwndControl&&SendMessage(hwndInsane, BM_GETCHECK,0,0)){
        SendMessage(hwndInsane, BM_SETCHECK, !SendMessage(hwndInsane, BM_GETCHECK,0,0),0);
    }

    if(hwndLazy!=hwndControl&&SendMessage(hwndLazy, BM_GETCHECK,0,0)){
        SendMessage(hwndLazy, BM_SETCHECK, !SendMessage(hwndLazy, BM_GETCHECK,0,0),0);
    }
    if(hwndEasy!=hwndControl&&SendMessage(hwndEasy, BM_GETCHECK,0,0)){
        SendMessage(hwndEasy, BM_SETCHECK, !SendMessage(hwndEasy, BM_GETCHECK,0,0),0);
    }
    if(hwndNormal!=hwndControl&&SendMessage(hwndNormal, BM_GETCHECK,0,0)){
        SendMessage(hwndNormal, BM_SETCHECK, !SendMessage(hwndNormal, BM_GETCHECK,0,0),0);
    }
    if(hwndHard!=hwndControl&&SendMessage(hwndHard, BM_GETCHECK,0,0)){
        SendMessage(hwndHard, BM_SETCHECK, !SendMessage(hwndHard, BM_GETCHECK,0,0),0);
    }
    if(hwndInsane!=hwndControl&&SendMessage(hwndInsane, BM_GETCHECK,0,0)){
        SendMessage(hwndInsane, BM_SETCHECK, !SendMessage(hwndInsane, BM_GETCHECK,0,0),0);
    }
    SendMessage(hwndControl, BM_SETCHECK, !SendMessage(hwndControl, BM_GETCHECK,0,0),0);
    return 1;


}
int CheckRadioP1(HWND hwnd,HWND hwndControl)
{
    HWND hwndRedP1= GetDlgItem(hwnd, REDP1);
    HWND hwndBlueP1= GetDlgItem(hwnd, BLUEP1);
    HWND hwndGreenP1= GetDlgItem(hwnd, GREENP1);
    HWND hwndPurpleP1= GetDlgItem(hwnd, PURPLEP1);
    HWND hwndYellowP1= GetDlgItem(hwnd, YELLOWP1);
    HWND hwndBlackP1= GetDlgItem(hwnd, BLACKP1);
    HWND hwndSurpriseP1= GetDlgItem(hwnd, SURPRISEP1);

    if(hwndRedP1==hwndControl&&SendMessage(hwndRedP1, BM_GETCHECK,0,0)){
        SendMessage(hwndRedP1, BM_SETCHECK, !SendMessage(hwndRedP1, BM_GETCHECK,0,0),0);
    }
    if(hwndBlueP1==hwndControl&&SendMessage(hwndBlueP1, BM_GETCHECK,0,0)){
        SendMessage(hwndBlueP1, BM_SETCHECK, !SendMessage(hwndBlueP1, BM_GETCHECK,0,0),0);
    }
    if(hwndGreenP1==hwndControl&&SendMessage(hwndGreenP1, BM_GETCHECK,0,0)){
        SendMessage(hwndGreenP1, BM_SETCHECK, !SendMessage(hwndGreenP1, BM_GETCHECK,0,0),0);
    }
    if(hwndPurpleP1==hwndControl&&SendMessage(hwndPurpleP1, BM_GETCHECK,0,0)){
        SendMessage(hwndPurpleP1, BM_SETCHECK, !SendMessage(hwndPurpleP1, BM_GETCHECK,0,0),0);
    }
    if(hwndYellowP1==hwndControl&&SendMessage(hwndYellowP1, BM_GETCHECK,0,0)){
        SendMessage(hwndYellowP1, BM_SETCHECK, !SendMessage(hwndYellowP1, BM_GETCHECK,0,0),0);
    }
    if(hwndBlackP1==hwndControl&&SendMessage(hwndBlackP1, BM_GETCHECK,0,0)){
        SendMessage(hwndBlackP1, BM_SETCHECK, !SendMessage(hwndBlackP1, BM_GETCHECK,0,0),0);
    }
    if(hwndSurpriseP1==hwndControl&&SendMessage(hwndSurpriseP1, BM_GETCHECK,0,0)){
        SendMessage(hwndSurpriseP1, BM_SETCHECK, !SendMessage(hwndSurpriseP1, BM_GETCHECK,0,0),0);
    }

    if(hwndRedP1!=hwndControl&&SendMessage(hwndRedP1, BM_GETCHECK,0,0)){
        SendMessage(hwndRedP1, BM_SETCHECK, !SendMessage(hwndRedP1, BM_GETCHECK,0,0),0);
    }
    if(hwndBlueP1!=hwndControl&&SendMessage(hwndBlueP1, BM_GETCHECK,0,0)){
        SendMessage(hwndBlueP1, BM_SETCHECK, !SendMessage(hwndBlueP1, BM_GETCHECK,0,0),0);
    }
    if(hwndGreenP1!=hwndControl&&SendMessage(hwndGreenP1, BM_GETCHECK,0,0)){
        SendMessage(hwndGreenP1, BM_SETCHECK, !SendMessage(hwndGreenP1, BM_GETCHECK,0,0),0);
    }
    if(hwndPurpleP1!=hwndControl&&SendMessage(hwndPurpleP1, BM_GETCHECK,0,0)){
        SendMessage(hwndPurpleP1, BM_SETCHECK, !SendMessage(hwndPurpleP1, BM_GETCHECK,0,0),0);
    }
    if(hwndYellowP1!=hwndControl&&SendMessage(hwndYellowP1, BM_GETCHECK,0,0)){
        SendMessage(hwndYellowP1, BM_SETCHECK, !SendMessage(hwndYellowP1, BM_GETCHECK,0,0),0);
    }
    if(hwndBlackP1!=hwndControl&&SendMessage(hwndBlackP1, BM_GETCHECK,0,0)){
        SendMessage(hwndBlackP1, BM_SETCHECK, !SendMessage(hwndBlackP1, BM_GETCHECK,0,0),0);
    }
    if(hwndSurpriseP1!=hwndControl&&SendMessage(hwndSurpriseP1, BM_GETCHECK,0,0)){
        SendMessage(hwndSurpriseP1, BM_SETCHECK, !SendMessage(hwndSurpriseP1, BM_GETCHECK,0,0),0);
    }
    SendMessage(hwndControl, BM_SETCHECK, !SendMessage(hwndControl, BM_GETCHECK,0,0),0);
        return 1;

}

int CheckRadioP2(HWND hwnd,HWND hwndControl)
{
    HWND hwndRedP2= GetDlgItem(hwnd, REDP2);
    HWND hwndBlueP2= GetDlgItem(hwnd, BLUEP2);
    HWND hwndGreenP2= GetDlgItem(hwnd, GREENP2);
    HWND hwndPurpleP2= GetDlgItem(hwnd, PURPLEP2);
    HWND hwndYellowP2= GetDlgItem(hwnd, YELLOWP2);
    HWND hwndBlackP2= GetDlgItem(hwnd, BLACKP2);
    HWND hwndSurpriseP2= GetDlgItem(hwnd, SURPRISEP2);

    if(hwndRedP2==hwndControl&&SendMessage(hwndRedP2, BM_GETCHECK,0,0)){
        SendMessage(hwndRedP2, BM_SETCHECK, !SendMessage(hwndRedP2, BM_GETCHECK,0,0),0);
    }
    if(hwndBlueP2==hwndControl&&SendMessage(hwndBlueP2, BM_GETCHECK,0,0)){
        SendMessage(hwndBlueP2, BM_SETCHECK, !SendMessage(hwndBlueP2, BM_GETCHECK,0,0),0);
    }
    if(hwndGreenP2==hwndControl&&SendMessage(hwndGreenP2, BM_GETCHECK,0,0)){
        SendMessage(hwndGreenP2, BM_SETCHECK, !SendMessage(hwndGreenP2, BM_GETCHECK,0,0),0);
    }
    if(hwndPurpleP2==hwndControl&&SendMessage(hwndPurpleP2, BM_GETCHECK,0,0)){
        SendMessage(hwndPurpleP2, BM_SETCHECK, !SendMessage(hwndPurpleP2, BM_GETCHECK,0,0),0);
    }
    if(hwndYellowP2==hwndControl&&SendMessage(hwndYellowP2, BM_GETCHECK,0,0)){
        SendMessage(hwndYellowP2, BM_SETCHECK, !SendMessage(hwndYellowP2, BM_GETCHECK,0,0),0);
    }
    if(hwndBlackP2==hwndControl&&SendMessage(hwndBlackP2, BM_GETCHECK,0,0)){
        SendMessage(hwndBlackP2, BM_SETCHECK, !SendMessage(hwndBlackP2, BM_GETCHECK,0,0),0);
    }
    if(hwndSurpriseP2==hwndControl&&SendMessage(hwndSurpriseP2, BM_GETCHECK,0,0)){
        SendMessage(hwndSurpriseP2, BM_SETCHECK, !SendMessage(hwndSurpriseP2, BM_GETCHECK,0,0),0);
    }

    if(hwndRedP2!=hwndControl&&SendMessage(hwndRedP2, BM_GETCHECK,0,0)){
        SendMessage(hwndRedP2, BM_SETCHECK, !SendMessage(hwndRedP2, BM_GETCHECK,0,0),0);
    }
    if(hwndBlueP2!=hwndControl&&SendMessage(hwndBlueP2, BM_GETCHECK,0,0)){
        SendMessage(hwndBlueP2, BM_SETCHECK, !SendMessage(hwndBlueP2, BM_GETCHECK,0,0),0);
    }
    if(hwndGreenP2!=hwndControl&&SendMessage(hwndGreenP2, BM_GETCHECK,0,0)){
        SendMessage(hwndGreenP2, BM_SETCHECK, !SendMessage(hwndGreenP2, BM_GETCHECK,0,0),0);
    }
    if(hwndPurpleP2!=hwndControl&&SendMessage(hwndPurpleP2, BM_GETCHECK,0,0)){
        SendMessage(hwndPurpleP2, BM_SETCHECK, !SendMessage(hwndPurpleP2, BM_GETCHECK,0,0),0);
    }
    if(hwndYellowP2!=hwndControl&&SendMessage(hwndYellowP2, BM_GETCHECK,0,0)){
        SendMessage(hwndYellowP2, BM_SETCHECK, !SendMessage(hwndYellowP2, BM_GETCHECK,0,0),0);
    }
    if(hwndBlackP2!=hwndControl&&SendMessage(hwndBlackP2, BM_GETCHECK,0,0)){
        SendMessage(hwndBlackP2, BM_SETCHECK, !SendMessage(hwndBlackP2, BM_GETCHECK,0,0),0);
    }
    if(hwndSurpriseP2!=hwndControl&&SendMessage(hwndSurpriseP2, BM_GETCHECK,0,0)){
        SendMessage(hwndSurpriseP2, BM_SETCHECK, !SendMessage(hwndSurpriseP2, BM_GETCHECK,0,0),0);
    }
    SendMessage(hwndControl, BM_SETCHECK, !SendMessage(hwndControl, BM_GETCHECK,0,0),0);
    return 1;
}

LRESULT CALLBACK WindowProcedureOptionsWindow (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    HBRUSH new_brush, old_brush;
    char txt[100];
    static HBRUSH hbr=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    HWND hwndBlueP1,hwndBlueP2,hwndRedP1,hwndRedP2,hwndYellowP1,hwndYellowP2,hwndGreenP1,hwndGreenP2,hwndNormal;
    switch (message){
        case WM_CREATE:
        {
            HDC hdc=GetDC(hwndOptions);
            HINSTANCE hInst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

            CreateWindow("static", "Player One: ", WS_CHILD | WS_VISIBLE | WS_EX_TRANSPARENT,
                         30, 90, 80, 20, hwnd, (HMENU)STATIC_BOX1, hInst, NULL);
            CreateWindow("edit", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT,
                         120, 90, 80, 20, hwnd, (HMENU)NAME1_BOX, hInst, NULL);
            CreateWindow("static", "Player Two: ", WS_CHILD | WS_VISIBLE | WS_EX_TRANSPARENT,
                         30, 120, 80, 20, hwnd, (HMENU)STATIC_BOX2, hInst, NULL);
            CreateWindow("edit", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT,
                         120, 120, 80, 20, hwnd, (HMENU)NAME2_BOX, hInst, NULL);
            CreateWindow("button", "Set players", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_PUSHLIKE,
                         210, 90, 80, 50, hwnd, (HMENU)SET_BUTTON, hInst, NULL);

            CreateWindowEx(WS_EX_WINDOWEDGE,"button", " Player 1 Color: ",WS_VISIBLE | WS_CHILD|BS_GROUPBOX,
                    20,200,140,235,hwnd,(HMENU)GROUP_COLORS1,hInst, NULL);

            hwndRedP1=CreateWindow("button", "Red", WS_CHILD | WS_GROUP |WS_VISIBLE | BS_RADIOBUTTON|WS_GROUP,
                     30,220,45,30, hwnd, (HMENU)REDP1, hInst, NULL);
            hwndBlueP1=CreateWindow("button", "Blue", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON ,
                     30,250,48,30, hwnd, (HMENU)BLUEP1, hInst, NULL);
            hwndGreenP1=CreateWindow("button", "Green", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                     30,280,59,30, hwnd, (HMENU)GREENP1, hInst, NULL);
            CreateWindow("button", "Purple", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                     30,310,61,30, hwnd, (HMENU)PURPLEP1, hInst, NULL);
            hwndYellowP1=CreateWindow("button", "Yellow", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                     30,340,64,30, hwnd, (HMENU)YELLOWP1, hInst, NULL);
            CreateWindow("button", "Black", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                     30,370,55,30, hwnd, (HMENU)BLACKP1, hInst, NULL);
            CreateWindow("button", "Surprise me", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                     30,400,98,30, hwnd, (HMENU)SURPRISEP1, hInst, NULL);

            CreateWindowEx(WS_EX_WINDOWEDGE,"button", " Player 2 Color: ",WS_VISIBLE | WS_CHILD|BS_GROUPBOX,
                    158,200,150,235,hwnd,(HMENU)GROUP_COLORS1,hInst, NULL);

            hwndRedP2=CreateWindow("button", "Red", WS_CHILD |WS_GROUP| WS_VISIBLE | BS_RADIOBUTTON|WS_GROUP,
                     180,220,45,30, hwnd, (HMENU)REDP2, hInst, NULL);
            hwndBlueP2=CreateWindow("button", "Blue", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON ,
                     180,250,48,30, hwnd, (HMENU)BLUEP2, hInst, NULL);
            hwndGreenP2=CreateWindow("button", "Green", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                     180,280,59,30, hwnd, (HMENU)GREENP2, hInst, NULL);
            CreateWindow("button", "Purple", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                     180,310,61,30, hwnd, (HMENU)PURPLEP2, hInst, NULL);
            hwndYellowP2=CreateWindow("button", "Yellow", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                     180,340,64,30, hwnd, (HMENU)YELLOWP2, hInst, NULL);
            CreateWindow("button", "Black", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                     180,370,55,30, hwnd, (HMENU)BLACKP2, hInst, NULL);
            CreateWindow("button", "Surprise me", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                     180,400,98,30, hwnd, (HMENU)SURPRISEP2, hInst, NULL);
            CreateWindow("button", "Set color", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON|BS_PUSHLIKE,
                     120,440,80, 30, hwnd, (HMENU)SET_COLOR, hInst, NULL);

            CreateWindow("button", "Lazy", WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
                     10,520,52,30, hwnd, (HMENU)LAZY_BUTTON, hInst, NULL);
            CreateWindow("button", "Easy", WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
                     68,520,52,30, hwnd, (HMENU)EASY_BUTTON, hInst, NULL);
            hwndNormal=CreateWindow("button", "Normal", WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
                     128,520,65,30, hwnd, (HMENU)NORMAL_BUTTON, hInst, NULL);
            CreateWindow("button", "Hard", WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
                     199,520,50,30, hwnd, (HMENU)HARD_BUTTON, hInst, NULL);
            CreateWindow("button", "Insane", WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
                     255,520,63,30, hwnd, (HMENU)INSANE_BUTTON, hInst, NULL);

            CreateWindow("static", "Max. points: ", WS_CHILD | WS_VISIBLE | WS_EX_TRANSPARENT,
                         32, 570, 80, 20, hwnd, (HMENU)STATIC_POINTS, hInst, NULL);
            CreateWindow("edit", "5", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT,
                         120, 570, 80, 20, hwnd, (HMENU)EDIT_POINTS, hInst, NULL);
            CreateWindow("button", "Set points", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_PUSHLIKE,
                         210, 570, 80, 20, hwnd, (HMENU)SET_POINTS, hInst, NULL);

            CreateWindow("static", "Ball size: ", WS_CHILD | WS_VISIBLE | WS_EX_TRANSPARENT,
                         52, 600, 70, 20, hwnd, (HMENU)BALL_STATIC, hInst, NULL);
            CreateWindow("edit", "32", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT,
                         120, 600, 80, 20, hwnd, (HMENU)BALL_EDIT, hInst, NULL);
            CreateWindow("button", "Set size", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_PUSHLIKE,
                         210, 600, 80, 20, hwnd, (HMENU)SET_SIZE, hInst, NULL);

            hEnDis=CreateWindow("button", "Disable sound", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_PUSHLIKE,
                     105, 630, 120, 40, hwnd, (HMENU)ENABLEDISABLE_BUTTON, hInst, NULL);

            if(colorP1==RGB(255,0,0)){SendMessage(hwndRedP1, BM_SETCHECK, !SendMessage(hwndRedP1, BM_GETCHECK,0,0),0);}
            else if(colorP1==RGB(0,255,0)){SendMessage(hwndGreenP1, BM_SETCHECK, !SendMessage(hwndGreenP1, BM_GETCHECK,0,0),0);}
            else if(colorP1==RGB(255,255,0)){SendMessage(hwndYellowP1, BM_SETCHECK, !SendMessage(hwndYellowP1, BM_GETCHECK,0,0),0);}
            else{SendMessage(hwndBlueP1, BM_SETCHECK, !SendMessage(hwndBlueP1, BM_GETCHECK,0,0),0);}

            if(colorP2==RGB(255,0,0)){SendMessage(hwndRedP2, BM_SETCHECK, !SendMessage(hwndRedP2, BM_GETCHECK,0,0),0);}
            else if(colorP2==RGB(0,255,0)){SendMessage(hwndGreenP2, BM_SETCHECK, !SendMessage(hwndGreenP2, BM_GETCHECK,0,0),0);}
            else if(colorP2==RGB(255,255,0)){SendMessage(hwndYellowP2, BM_SETCHECK, !SendMessage(hwndYellowP2, BM_GETCHECK,0,0),0);}
            else{SendMessage(hwndBlueP2, BM_SETCHECK, !SendMessage(hwndBlueP2, BM_GETCHECK,0,0),0);}

            SendMessage(hwndNormal, BM_SETCHECK, !SendMessage(hwndNormal, BM_GETCHECK,0,0),0);

            ReleaseDC(hwnd,hdc);
        }
        case WM_PAINT:
        {
             hdc = BeginPaint(hwnd, &ps);
             new_brush = CreateSolidBrush(RGB(153, 217, 234));
             old_brush = (HBRUSH) SelectObject(hdc, new_brush);

             FillRect(hdc, &ps.rcPaint, new_brush);
             RECT r;
             GetClientRect(hwnd,&r);
             r.top+=50;
             sprintf(txt, "Set Player Names");
             SetBkMode(hdc, TRANSPARENT);
             DrawText(hdc, txt, strlen(txt), &r, DT_CENTER);
             memset(txt, 0, sizeof(txt));

             r.top+=115;
             sprintf(txt, "Set Player Color");
             SetBkMode(hdc, TRANSPARENT);
             DrawText(hdc, txt, strlen(txt), &r, DT_CENTER);
             memset(txt, 0, sizeof(txt));

             r.top+=330;
             sprintf(txt, "Set Game Difficulty and Parameters");
             SetBkMode(hdc, TRANSPARENT);
             DrawText(hdc, txt, strlen(txt), &r, DT_CENTER);
             memset(txt, 0, sizeof(txt));

             SelectObject(hdc, old_brush);
             DeleteObject(new_brush);

             EndPaint(hwnd, &ps);

             break;
        }
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case ENABLEDISABLE_BUTTON:
                {
                    UncheckSelectedButton(hwnd, (HWND)lParam);
                    if(sound == false){
                        bg_music(hwnd);


                        SendMessage(hEnDis, WM_SETTEXT, ENABLEDISABLE_BUTTON, (LPARAM)"Disable sound");
					}
					else{
                        stop_music(hwnd);
                        SendMessage(hEnDis, WM_SETTEXT, ENABLEDISABLE_BUTTON, (LPARAM)"Enable sound");
					}
                    break;
                }
				case SET_BUTTON:
                {
                    UncheckSelectedButton(hwnd, (HWND)lParam);

                    HWND hwndTextBox = GetDlgItem(hwnd, NAME1_BOX);
                    GetWindowText(hwndTextBox,playerOneNameBuffer,20);

                    HWND hwndTextBox1 = GetDlgItem(hwnd, NAME2_BOX);
                    GetWindowText(hwndTextBox1,playerTwoNameBuffer,20);

                    if(playerOneNameBuffer[0]!=0&&playerTwoNameBuffer[0]!=0){
                        memcpy(playerOneName, playerOneNameBuffer, sizeof(playerOneNameBuffer));
                        memcpy(playerTwoName, playerTwoNameBuffer, sizeof(playerTwoNameBuffer));
                        isPlayerSet=true;
                    }
                    else if(playerOneNameBuffer[0]==0&&playerTwoNameBuffer[0]!=0){
                         isPlayerSet=false;
                         memset(poruka1, 0, sizeof(poruka1));
                         MessageBox(NULL,(LPCSTR)"Player one name is not set!",(LPCSTR)"Wrong name entered",MB_ICONEXCLAMATION | MB_OK | MB_DEFBUTTON1);
                    }
                    else if(playerOneNameBuffer[0]!=0&&playerTwoNameBuffer[0]==0){
                         isPlayerSet=false;
                         memset(poruka2, 0, sizeof(poruka2));
                         MessageBox(NULL,(LPCSTR)"Player two name is not set!",(LPCSTR)"Wrong name entered",MB_ICONEXCLAMATION | MB_OK | MB_DEFBUTTON1);
                    }
                    else{
                        isPlayerSet=false;
                        memset(poruka1, 0, sizeof(poruka1));
                        memset(poruka2, 0, sizeof(poruka2));
                        MessageBox(NULL,(LPCSTR)"Player names are not set!",(LPCSTR)"Wrong names entered",MB_ICONEXCLAMATION | MB_OK | MB_DEFBUTTON1);
                    }


                    break;
                }
                case SET_SIZE:
                {
                    HWND hwndSize = GetDlgItem(hwnd, BALL_EDIT);
                    TCHAR buff[1024];
                    GetWindowText(hwndSize, buff, 1024);
                    if(atoi(buff)>=15&&atoi(buff)<=50){
                        ballRadius=atoi(buff);
                        memset(buff, 0, sizeof(buff));
                    }
                    else{
                        MessageBox(NULL,(LPCSTR)"Ball radius out of range! It must be between 15 and 50!",(LPCSTR)"Radius out of range",MB_ICONSTOP | MB_OK | MB_DEFBUTTON1);
                    }
                    break;
                }
                case LAZY_BUTTON:{
                    CheckDifficulty(hwnd, (HWND)lParam);
                    playerAcceleration=2;
                    accelerationGain=2;
                    break;
                }
                case EASY_BUTTON:{
                    CheckDifficulty(hwnd, (HWND)lParam);
                    playerAcceleration=3;
                    accelerationGain=4;
                    accelerationGain=4;
                    break;
                }
                case NORMAL_BUTTON:{
                    CheckDifficulty(hwnd, (HWND)lParam);
                    playerAcceleration=4;
                    accelerationGain=6;
                    break;
                }
                case HARD_BUTTON:{
                    CheckDifficulty(hwnd, (HWND)lParam);
                    playerAcceleration=3;
                    accelerationGain=8;
                    break;
                }
                case INSANE_BUTTON:{
                    CheckDifficulty(hwnd, (HWND)lParam);
                    playerAcceleration=2;
                    accelerationGain=10;
                    break;
                }
                case REDP1:{
                    checkedP1=CheckRadioP1(hwnd, (HWND)lParam);
                    color1=RGB(255,0,0);
                    break;
                }
                case BLUEP1:{
                    checkedP1=CheckRadioP1(hwnd, (HWND)lParam);
                    color1=RGB(0,0,255);
                    break;
                }
                case GREENP1:{
                    checkedP1=CheckRadioP1(hwnd, (HWND)lParam);
                    color1=RGB(0,255,0);
                    break;
                }
                case PURPLEP1:{
                    checkedP1=CheckRadioP1(hwnd, (HWND)lParam);
                    color1=RGB(255,0,255);
                    break;
                }
                case YELLOWP1:{
                    checkedP1=CheckRadioP1(hwnd, (HWND)lParam);
                    color1=RGB(255,255,0);
                    break;
                }
                case BLACKP1:{
                    checkedP1=CheckRadioP1(hwnd, (HWND)lParam);
                    color1=RGB(0,0,0);
                    break;
                }
                case SURPRISEP1:{
                    checkedP1=CheckRadioP1(hwnd, (HWND)lParam);
                    color1=RGB(rand()%255,rand()%255,rand()%255);
                    break;
                }

                case REDP2:{
                    checkedP2=CheckRadioP2(hwnd, (HWND)lParam);
                    color2=RGB(255,0,0);
                    break;
                }
                case BLUEP2:{
                    checkedP2=CheckRadioP2(hwnd, (HWND)lParam);
                    color2=RGB(0,0,255);
                    break;
                }
                case GREENP2:{
                    checkedP2=CheckRadioP2(hwnd, (HWND)lParam);
                    color2=RGB(0,255,0);
                    break;
                }
                case PURPLEP2:{
                    checkedP2=CheckRadioP2(hwnd, (HWND)lParam);
                    color2=RGB(255,0,255);
                    break;
                }
                case YELLOWP2:{
                    checkedP2=CheckRadioP2(hwnd, (HWND)lParam);
                    color2=RGB(255,255,0);
                    break;
                }
                case BLACKP2:{
                    checkedP2=CheckRadioP2(hwnd, (HWND)lParam);
                    color2=RGB(0,0,0);
                    break;
                }
                case SURPRISEP2:{
                    checkedP2=CheckRadioP2(hwnd, (HWND)lParam);
                    color2=RGB(rand()%255,rand()%255,rand()%255);
                    break;
                }

                case SET_COLOR:{
                    colorP1=color1;
                    colorP2=color2;
                    RECT r;
                    GetClientRect(hwnd,&r);
                    InvalidateRect(hwnd,&r,TRUE);
                    UpdateWindow(hwnd);
                    break;

                }
                case SET_POINTS:{
                    HWND hwndPoints = GetDlgItem(hwnd, EDIT_POINTS);
                    TCHAR buff[1024];
                    GetWindowText(hwndPoints, buff, 1024);
                    if(atoi(buff)>0){
                        playPoints=atoi(buff);
                    }
                    else{
                        MessageBox(NULL,(LPCSTR)"Points for victory cannot be smaller than or equal to 0!",(LPCSTR)"Points out of scope",MB_ICONSTOP | MB_OK | MB_DEFBUTTON1);
                    }
                break;
                }
            }
        break;
        }

        case WM_CTLCOLORSTATIC:
            {
                switch (GetDlgCtrlID((HWND)lParam))
                {
                    case STATIC_BOX1:
                    {
                        SetTextColor((HDC) wParam, colorP1);
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case STATIC_BOX2:
                    {
                        SetTextColor((HDC) wParam, colorP2);
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case STATIC_POINTS:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case LAZY_BUTTON:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case EASY_BUTTON:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case NORMAL_BUTTON:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case HARD_BUTTON:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case INSANE_BUTTON:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case GROUP_COLORS1:
                    {
                        SetBkColor((HDC) wParam, RGB(153, 217, 234));
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case GROUP_COLORS2:
                    {
                        SetBkColor((HDC) wParam, RGB(153, 217, 234));
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case BALL_STATIC:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case REDP1:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case BLUEP1:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case GREENP1:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case PURPLEP1:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case YELLOWP1:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case BLACKP1:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case SURPRISEP1:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case REDP2:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case BLUEP2:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case GREENP2:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case PURPLEP2:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case YELLOWP2:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case BLACKP2:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    case SURPRISEP2:
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        return(LRESULT) GetStockObject(HOLLOW_BRUSH);
                    }
                    default:
                        return 0;
                }
        }
        case WM_CLOSE:
		{
            ShowWindow(hwnd, SW_HIDE);
            break;
		}
        case WM_DESTROY:
            DeleteObject(hbr);
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
        case WM_CLOSE:
		{
            ShowWindow(hwnd, SW_HIDE);
            gameActive=false;
            numberOver=false;
            i=0;
            break;
		}
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}

LRESULT CALLBACK WindowProcedureAboutWindow (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    HBRUSH new_brush, old_brush;
    char title[200];
    char msg[2500];

    switch (message)                  /* handle the messages */
    {
        case WM_PAINT:
        {
             hdc = BeginPaint(hwnd, &ps);
             new_brush = CreateSolidBrush(RGB(153, 217, 234));
             old_brush = (HBRUSH) SelectObject(hdc, new_brush);
             RECT r;
             GetClientRect(hwnd,&r);
             FillRect(hdc, &ps.rcPaint, new_brush);

             r.top+=25;
             sprintf(title,"---------------------------------------------------GAME INFO-----------------------------------------------------------");
             SetBkMode(hdc, TRANSPARENT);
             DrawText(hdc, title, strlen(title), &r, DT_CENTER);
             memset(title, 0, sizeof(title));

             r.top+=35;
             sprintf(msg, "  This game was made as the part of the Final exam  from Windows Programming. Its main idea comes from the famous Ice Hockey game. With this implementation, we have brought that experience to Windows.");
             SetBkMode(hdc, TRANSPARENT);
             DrawText(hdc, msg, strlen(msg), &r, DT_WORDBREAK|DT_LEFT);
             memset(msg, 0, sizeof(poruka1));

             r.top+=65;
             sprintf(title,"-----------------------------------------------GAMEPLAY RULES------------------------------------------------------");
             SetBkMode(hdc, TRANSPARENT);
             DrawText(hdc, title, strlen(title), &r, DT_CENTER);
             memset(title, 0, sizeof(title));

             r.top+=30;
             sprintf(msg, "  This game can be played only in two players mode. First you must click on OPTIONS button, then enter the names for the BLUE player and the RED player, and then click on the SET button. You can also change radius of the ball and colors of the players. Note that the radius range is between 15 and 50. You CANNOT make the ball bigger or smaller than that. Option SURPRISE ME paints your player with random color.\n\n  You can also change difficulty of the game. LAZY option gives you slow gameplay if you prefer it. EASY and NORMAL options make the ball and the players somewhat faster. HARD and INSANE options make the game a little bit interesting, by providing slower players and faster ball. \n\n  By clicking on SET POINTS, you can set maximum points that players must reach to win the game. The game can be started by clicking on the PLAY button. After that, the game starts with one ball positioned on the middle of the screen, and with the RED and BLUE player, positioned close to their respective goals. \n\n  The ball can be started by pressing the SPACE button or the ENTER button.\n\n  If you wish to exit the game, you must first close the game window and then click on the EXIT button on the main window.\n\n  BLUE player is controled with keys: 'W', 'A', 'S' and 'D'. RED player is controled with arrow keys: 'Up', 'Left', 'Down' and 'Right'.");
             SetBkMode(hdc, TRANSPARENT);
             DrawText(hdc, msg, strlen(msg), &r, DT_WORDBREAK|DT_LEFT);
             memset(msg, 0, sizeof(msg));

             r.top+=400;
             sprintf(title,"-----------------------------------------------SPECIAL THANKS------------------------------------------------------");
             SetBkMode(hdc, TRANSPARENT);
             DrawText(hdc, title, strlen(title), &r, DT_CENTER);
             memset(title, 0, sizeof(title));

             r.top+=30;
             sprintf(msg, "   We are very grateful to our professor Edin Pjanic who learned us how to create great apps for Windows.");
             SetBkMode(hdc, TRANSPARENT);
             DrawText(hdc, msg, strlen(msg), &r, DT_WORDBREAK|DT_LEFT);
             memset(msg, 0, sizeof(msg));

             r.top+=60;

             sprintf(msg, "----------------------------------- © Copyright by Mladen Zeljic and Alma Gogic -----------------------------------");
             SetBkMode(hdc, TRANSPARENT);
             DrawText(hdc, msg, strlen(msg), &r,DT_CENTER);
             memset(msg, 0, sizeof(msg));

             SelectObject(hdc, old_brush);
             DeleteObject(new_brush);

             EndPaint(hwnd, &ps);

             break;
        }
        case WM_CLOSE:
            ShowWindow(hwnd, SW_HIDE);
            break;
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}
