extern "C"{

#include <SDL_mixer.h>

}




class InitError : public std::exception
{
    std::string msg;
public:
    InitError();
    InitError( const std::string & );
    virtual ~InitError() throw();
    virtual const char * what() const throw();
};

InitError::InitError() :
    exception(),
    msg( SDL_GetError() )
{
}

InitError::InitError( const std::string & m ) :
    exception(),
    msg( m )
{
}

InitError::~InitError() throw()
{
}

const char * InitError::what() const throw()
{
    return msg.c_str();
}


class SDL
{
    SDL_Window * m_window;
    SDL_Renderer * m_renderer;
public:
    SDL( Uint32 flags = 0 );
    virtual ~SDL();
    void draw();
    void drawrect(SDL_Renderer *m_r,int r,int g,int b,int a, SDL_Rect rect);
    int scrnw,scrnh;

    int scale_ac_x(float x){
        return (x/1280+0.5)*(float)scrnw;
    }

    int scale_ac_y(float y){
        return ((-y)/720+0.5)*(float)scrnh;
    }
    int scale_x(float y){
        return ((y)/1280)*(float)scrnw;
    }
    int scale_y(float y){
        return ((y)/720)*(float)scrnh;
    }
};

SDL::SDL( Uint32 flags )
{
    if ( SDL_Init( flags ) != 0 )
        throw InitError();
SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT,"#canvas");
    if ( SDL_CreateWindowAndRenderer( 858, 525, SDL_WINDOW_SHOWN,
                                      &m_window, &m_renderer ) != 0 )
        throw InitError();

    if( Mix_OpenAudio( 48000, AUDIO_U16LSB, 2, 2048 ) == -1 )
    {
        throw InitError();
    }


    SDL_SetWindowTitle(m_window, "Pong.");


    SDL_GetWindowSize(m_window, &scrnw,&scrnh);
}

SDL::~SDL()
{
    SDL_DestroyWindow( m_window );
    SDL_DestroyRenderer( m_renderer );
    SDL_Quit();
}
