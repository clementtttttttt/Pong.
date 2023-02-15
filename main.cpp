#include <exception>
#include <string>
#include <iostream>
#include <SDL.h>
#include <string>
#include <SDL_ttf.h>
#include "main.h"
#include <unistd.h>
#ifdef EMCXX
#include <emscripten.h>
#endif


SDL *sdl;

int
SDL_RenderFillCircle(SDL_Renderer * renderer, int x, int y, int radius)
{
    int offsetx, offsety, d;
    int status;

    offsetx = 0;
    offsety = radius;
    d = radius -1;
    status = 0;

    while (offsety >= offsetx) {

        status += SDL_RenderDrawLine(renderer, x - offsety, y + offsetx,
                                     x + offsety, y + offsetx);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y + offsety,
                                     x + offsetx, y + offsety);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y - offsety,
                                     x + offsetx, y - offsety);
        status += SDL_RenderDrawLine(renderer, x - offsety, y - offsetx,
                                     x + offsety, y - offsetx);

        if (status < 0) {
            status = -1;
            break;
        }

        if (d >= 2*offsetx) {
            d -= 2*offsetx + 1;
            offsetx +=1;
        }
        else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        }
        else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }

    return status;
}



void SDL::drawrect(SDL_Renderer *m_r,int r,int g,int b,int a, SDL_Rect rect){

    SDL_Rect rect2 = rect;

    SDL_SetRenderDrawColor( m_r, r, g, b, a);

    rect2.x = scale_ac_x(rect.x);
    rect2.y = scale_ac_y(rect.y);
    rect2.w = scale_x(rect.w);
    rect2.h = -scale_y(rect.h);

    SDL_RenderFillRect( m_r, &rect2 );

}

SDL_Rect leftpad = {-600,-40,20,80};

SDL_Rect rightpad = {580,-40,20,80};

SDL_Rect centre;

int bx=0,by,bxd=0,byd,br=10;

int bscrnw;

int ai_up = 0, ai_down = 0;

int ai2_up = 0, ai2_down = 0;
TTF_Font* font;

int score1,score2;

void drawText ( SDL_Renderer* screen, char* str, int x, int y, int sz,SDL_Color fgC, SDL_Color bgC,int leftanchor) {


    //SDL_Surface* textSurface = TTF_RenderText_Solid(font, string, fgC);     // aliased glyphs
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, str, fgC);   // anti-aliased glyphs
    SDL_Rect textLocation = {(int)( x - 70*leftanchor*(strlen(str)-1)), y, (int)(70*(strlen(str))), 70};
    SDL_Texture *t = SDL_CreateTextureFromSurface(screen, textSurface);

    SDL_RenderCopy(screen, t, NULL,&textLocation);

    SDL_DestroyTexture(t);
    SDL_FreeSurface(textSurface);

    //printf("[ERROR] Unknown error in drawText(): %s\n", TTF_GetError()); return 1;
}

int dest = 4, dest2 = -2;


void SDL::draw()
{


    // Clear the window with a black background
    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( m_renderer );


    drawrect(m_renderer,255,255,255,255,leftpad);
    drawrect(m_renderer,255,255,255,255,rightpad);

//    SDL_Rect destrect= {-600,dest,20,80};
//    drawrect(m_renderer,255,00,255,255,destrect);


    centre = {-5,-360,5,10};
    for(int i=0;i<30;++i){
        drawrect(m_renderer, 255, 255, 255, 255, centre);
        centre.y += 24;
    }
    SDL_RenderFillCircle(m_renderer,scale_ac_x(bx),scale_ac_y(by), (float)br/1280*bscrnw);


    drawText(m_renderer,(char*) std::to_string(score1).c_str(), scale_ac_x(-320),scale_ac_y(320),30,{0xff,0xff,0xff},{0,0,0}, 0);
    drawText(m_renderer,(char*) std::to_string(score2).c_str(), scale_ac_x(320-70),scale_ac_y(320),30,{0xff,0xff,0xff},{0,0,0}, 1);


    SDL_RenderPresent( m_renderer );

}

bool intersects(int *pcx,int cy,int r, SDL_Rect rect,int xd,int yd)
{
    SDL_Rect ballrect{*pcx-r,cy-r,r*2,r*2};


return  (ballrect.x <= (rect.x + rect.w)) &&
        ((ballrect.x + ballrect.w) >= rect.x) &&
        (ballrect.y < (rect.y + rect.h)) &&
        ((ballrect.y + ballrect.h) > rect.y);
}


int prev,prev2,ret;
int biasedRandom(int rec=0){


    ret = rand()%17;

    if(ret == prev || ret == prev2){
        return biasedRandom(1);
    }
    if(!rec){
        prev2 = prev;

        prev = ret;
    }
    return ret;
}

void ai2_calc_dest(){
    int yd = byd;
    int cx = bx+bxd;
    int cy = by+yd;

    while(cx < 580 && !(((cy+br) >= 360) || ((cy-br) <= -360))){


        cx += bxd;
        cy += yd;
    }

    dest2 = cy + biasedRandom()*3 * ((biasedRandom()%2)?1:-1);


}

void ai_calc_dest(){
    int yd = byd;
    int cx = bx+bxd;
    int cy = by+yd;


    while(cx > -580 && !(((cy+br) >= 360 )|| ((cy-br) <= -360))){


        cx += bxd;
        cy += yd;
    }

    dest = cy + biasedRandom()*3* ((biasedRandom()%2)?1:-1);

}

    Mix_Chunk* beep;

void pad_beep(float len_ims, int freq){

    Mix_FreeChunk(beep);
    unsigned short *audio;

    audio = new unsigned short[(unsigned int)(len_ims/1000*48000)];

    for(unsigned int i=0;i < (unsigned int)(len_ims/1000*48000); ++i){

        if(i%(unsigned int)(1.0/freq*48000) >= ((unsigned int)(1.0/freq*48000)/2)){
            audio[i] = 0x8888;


        }
        else{
                audio[i] = 0;

        }
    }

    beep = Mix_QuickLoad_RAW((unsigned char*)audio, (unsigned int)(len_ims/1000*48000) * sizeof(unsigned short));

    Mix_PlayChannel(-1,beep, 0);

    delete[] audio;
}

int beeping = 0;

int ai_enable,ai2_enable;

void balltick(){
    if(bxd == 0){

        bxd = 20;
    }


    if(((by+br) >= 360 || (by-br) <= (-360)) && !((bx+br >= 640 || bx-br <= -640))){
        byd = -byd;

        pad_beep(100,123);
        if(bxd < 0){
                ai_calc_dest();

        }
        else ai2_calc_dest();
    }

    if(bx+br >= 640 || bx-br <= -640){
        if(!beeping){
            beeping = 1;
            pad_beep(1000,122);

            ++(bx+br>=0?score1:score2);

            if(score1 > 99) score1 = 0;

            if (score2 > 99) score2 = 0;
        }

    }
    if(((bx+br >= 640+(bxd*61))&&bxd>=0) || ((bx-br <= -640+(bxd*61))&&(bxd<0)) ){

            bx = 0; by=0; byd=0;

            if(bxd < 0){
                bxd = -bxd;

            }

            beeping=0;
            ai2_calc_dest();
    }

    bx += bxd;
    by += byd;

    if(intersects(&bx,by,br,rightpad,bxd,byd)){
            bx = rightpad.x - br;

            if (bxd >= 0)
                bxd = -bxd;


            byd = by - (rightpad.y+rightpad.h/2);
            byd /= 2;

                    ai_calc_dest();

            pad_beep(100,246);

    }
    if(intersects(&bx,by,br,leftpad,bxd,byd)){
            bx = leftpad.x + leftpad.w + br;

            if (bxd < 0)

                bxd = -bxd;

            byd = by - (leftpad.y+leftpad.h/2);
            byd /= 2;
            ai2_calc_dest();
            pad_beep(100,246);

    }

}
int frames=0;


void aitick(){



        if((leftpad.y+leftpad.h/2) < dest-8){

            ai_up = 1;
        }
        else ai_up = 0;

        if((leftpad.y+leftpad.h/2) > dest+8){

            ai_down = 1;

        }
        else ai_down = 0;



}

void ai2tick(){
    if((rightpad.y+rightpad.h/2) < dest2-8){

        ai2_up = 1;
    }
    else ai2_up = 0;

    if((rightpad.y+rightpad.h/2) > dest2+8){

        ai2_down = 1;
    }
    else ai2_down = 0;

}



 int game_tick(){

        SDL_Event event;

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:

                    return 1;
                    break;

                case SDL_KEYDOWN:

                    switch(event.key.keysym.sym){
                        case SDLK_q:
                            ai_enable = !ai_enable;
                            ai_up = 0;
                            ai_down = 0;

                            break;

                        case SDLK_e:
                            ai2_enable = !ai2_enable;
                            ai2_up = 0;
                            ai2_down = 0;

                            break;

                    }

            }

        }

        ++frames;

        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

        if(currentKeyStates[SDL_SCANCODE_W]|| (ai_up)){
            if(leftpad.y+leftpad.h < 360)
                leftpad.y += 15;

        }
        if(currentKeyStates[SDL_SCANCODE_S]|| ai_down){
            if(leftpad.y > -360)
            leftpad.y -= 15;
        }

        if(currentKeyStates[SDL_SCANCODE_UP] || ai2_up){
            if(rightpad.y+rightpad.h < 360)
            rightpad.y += 15;

        }
        if(currentKeyStates[SDL_SCANCODE_DOWN] || ai2_down){
            if(rightpad.y > -360)
            rightpad.y -= 15;
        }

        balltick();
        if(ai_enable)
        aitick();

        if(ai2_enable)
        ai2tick();


        sdl->draw();

        return 0;
}

int main( int argc, char * argv[] )
{


    SDL sdl1( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);
    bscrnw = sdl1.scrnw;

    srand(time(0));

    sdl = &sdl1;


    TTF_Init();

    // Remember to call TTF_Init(), TTF_Quit(), before/after using this function.
    font = TTF_OpenFont("./font.ttf", 30);
    if(!font) {
        printf("[ERROR] TTF_OpenFont() Failed with: %s\n", TTF_GetError());
        exit(2);
    }

    #ifdef EMCXX
        emscripten_set_main_loop((void(*)())game_tick,60,1);
    #else
        while(!game_tick());

    #endif
    TTF_CloseFont(font);

    TTF_Quit();

    return 1;
}
