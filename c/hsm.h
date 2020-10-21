/**
* hsm.h -- Hierarchical State Machine interface
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
#ifndef hsm_h
#define hsm_h

typedef int Event;
typedef struct {
    Event evt;
} Msg;

typedef struct Hsm Hsm;
typedef Msg const *(*EvtHndlr)(Hsm*, Msg const*);

typedef struct State State;
struct State {
    State *super;                                  /* pointer to superstate */
    EvtHndlr hndlr;                             /* state's handler function */
    char const *name;
};

void StateCtor(State *me, char const *name, State *super, EvtHndlr hndlr);
#define StateOnEvent(me_, ctx_, msg_) \
    (*(me_)->hndlr)((ctx_), (msg_))

struct Hsm {                       /* Hierarchical State Machine base class */
    char const *name;                             /* pointer to static name */
    State *curr;                                           /* current state */
    State *next;                  /* next state (non 0 if transition taken) */
    State *source;                   /* source state during last transition */
    State top;                                     /* top-most state object */
};

void HsmCtor(Hsm *me, char const *name, EvtHndlr topHndlr);
void HsmOnStart(Hsm *me);                  /* enter and start the top state */
void HsmOnEvent(Hsm *me, Msg const *msg);                   /* "HSM engine" */

/* protected: */
unsigned char HsmToLCA_(Hsm *me, State *target);
void HsmExit_(Hsm *me, unsigned char toLca);
                                                       /* get current state */
#define STATE_CURR(me_) (((Hsm *)me_)->curr)
                     /* take start transition (no states need to be exited) */
#define STATE_START(me_, target_) (((Hsm *)me_)->next = (target_))
                     /* take a state transition (exit states up to the LCA) */
#define STATE_TRAN(me_, target_) if (1) { \
    static unsigned char toLca_ = 0xFF; \
    assert(((Hsm *)me_)->next == 0); \
    if (toLca_ == 0xFF) \
        toLca_ = HsmToLCA_((Hsm *)(me_), (target_)); \
    HsmExit_((Hsm *)(me_), toLca_); \
    ((Hsm *)(me_))->next = (target_); \
} else ((void)0)

#define START_EVT ((Event)(-1))
#define ENTRY_EVT ((Event)(-2))
#define EXIT_EVT  ((Event)(-3))

#endif /* hsm_h */
