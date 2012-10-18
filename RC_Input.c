#include "RC_Input.h"


void
RC_ScanKeyboardInput(SDL_Event *e, RC_InputControls *rc_i)
{
    const int etype = e->type;
    switch(etype)
    {
        case SDL_KEYDOWN:
        {
            SDL_KeyboardEvent kb = e->key;
            SDL_keysym k = kb.keysym;
            if(rc_i->kup == k.sym)
            {
                rc_i->kup_state = SDL_KEYDOWN;
            }
            if(rc_i->kdn == k.sym)
            {
                rc_i->kdn_state = SDL_KEYDOWN;
            }
            if(rc_i->krt == k.sym)
            {
                rc_i->krt_state = SDL_KEYDOWN;
            }
            if(rc_i->klf == k.sym)
            {
                rc_i->klf_state = SDL_KEYDOWN;
            }
        }break;
        case SDL_KEYUP:
        {
            SDL_KeyboardEvent kb = e->key;
            SDL_keysym k = kb.keysym;
            if(rc_i->kup == k.sym)
            {
                rc_i->kup_state = SDL_KEYUP;
            }
            if(rc_i->kdn == k.sym)
            {
                rc_i->kdn_state = SDL_KEYUP;
            }
            if(rc_i->krt == k.sym)
            {
                rc_i->krt_state = SDL_KEYUP;
            }
            if(rc_i->klf == k.sym)
            {
                rc_i->klf_state = SDL_KEYUP;
            }
        }break;

    }
}

void
RC_ScanMouseInput(SDL_Event *e, RC_InputControls *rc_i)
{
    const int etype = e->type;
    static int mouse_warp_event = FALSE;

    if(!mouse_warp_event)
    {
        switch(etype)
        {
            case SDL_MOUSEBUTTONDOWN:
                rc_i->m_btn_state = SDL_GetMouseState(NULL, NULL);
                break;
            case SDL_MOUSEMOTION:
                SDL_GetMouseState(&(rc_i->m_x), &(rc_i->m_y));
                SDL_GetRelativeMouseState(&(rc_i->m_relx), &(rc_i->m_rely));
                break;
         /*   default:
            {
                rc_i->m_btn_state = 0;
                rc_i->m_relx = 0;
                rc_i->m_rely = 0;
            }break;*/
        }
    }

}
