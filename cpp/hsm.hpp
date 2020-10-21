/**
* hsm.hpp -- Hierarchical State Machine interface in C++
*
* Copyright 2000 Miro Samek. All rights reserved.
*
* This software is licensed under the following open source MIT license:
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*
* Contact information:
* miro@quantum-leaps.com
*/
#ifndef hsm_hpp
#define hsm_hpp

typedef int Event;
struct Msg {
    Event evt;
};

class Hsm;                                           /* forward declaration */
typedef Msg const *(Hsm::*EvtHndlr)(Msg const *);

class State {
    State *super;                                  /* pointer to superstate */
    EvtHndlr hndlr;                             /* state's handler function */
    char const *name;
  public:
    State(char const *name, State *super, EvtHndlr hndlr);
  private:
    Msg const *onEvent(Hsm *ctx, Msg const *msg) {
        return (ctx->*hndlr)(msg);
    }
    friend class Hsm;
};

class Hsm {                        /* Hierarchical State Machine base class */
    char const *name;                             /* pointer to static name */
    State *curr;                                           /* current state */
protected:
    State *next;                  /* next state (non 0 if transition taken) */
    State *source;                   /* source state during last transition */
    State top;                                     /* top-most state object */
public:
    Hsm(char const *name, EvtHndlr topHndlr);                       /* Ctor */
    void onStart();                        /* enter and start the top state */
    void onEvent(Msg const *msg);                 /* "state machine engine" */
protected:
    unsigned char toLCA_(State *target);
    void exit_(unsigned char toLca);
    State *STATE_CURR() { return curr; }
    void STATE_START(State *target) {
        //assert(next == 0);
        next = target;
    }
};

# define STATE_TRAN(target_) if (1) {   \
    static unsigned char toLca_ = 0xFF; \
    assert(next == 0);                  \
    if (toLca_ == 0xFF)                 \
        toLca_ = toLCA_(target_);       \
    exit_(toLca_);                      \
    next = (target_);                   \
} else ((void)0)

#define START_EVT ((Event)(-1))
#define ENTRY_EVT ((Event)(-2))
#define EXIT_EVT  ((Event)(-3))

#endif /* hsm_hpp */
