/**
* hsm.c -- Hierarchical State Machine implementation
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
#include "hsm.h"

static Msg const startMsg = { START_EVT };
static Msg const entryMsg = { ENTRY_EVT };
static Msg const exitMsg  = { EXIT_EVT };
#define MAX_STATE_NESTING 8

/* State Ctor...............................................................*/
void StateCtor(State *me, char const *name, State *super, EvtHndlr hndlr) {
    me->name  = name;
    me->super = super;
    me->hndlr = hndlr;
}

/* Hsm Ctor.................................................................*/
void HsmCtor(Hsm *me, char const *name, EvtHndlr topHndlr) {
    StateCtor(&me->top, "top", 0, topHndlr);
    me->name = name;
}

/* enter and start the top state............................................*/
void HsmOnStart(Hsm *me) {
    me->curr = &me->top;
    me->next = 0;
    StateOnEvent(me->curr, me, &entryMsg);
    while (StateOnEvent(me->curr, me, &startMsg), me->next) {
        State *entryPath[MAX_STATE_NESTING];
        register State **trace = entryPath;
        register State *s;
        *trace = 0;
        for (s = me->next; s != me->curr; s = s->super) {
            *(++trace) = s;                         /* trace path to target */
        }
        while (s = *trace--) {                 /* retrace entry from source */
            StateOnEvent(s, me, &entryMsg);
        }
        me->curr = me->next;
        me->next = 0;
    }
}

/* state machine "engine"...................................................*/
void HsmOnEvent(Hsm *me, Msg const *msg) {
    State *entryPath[MAX_STATE_NESTING];
    register State **trace;
    register State *s;
    for (s = me->curr; s; s = s->super) {
        me->source = s;                 /* level of outermost event handler */
        msg = StateOnEvent(s, me, msg);
        if (msg == 0) {
            if (me->next) {                      /* state transition taken? */
                trace = entryPath;
                *trace = 0;
                for (s = me->next; s != me->curr; s = s->super) {
                    *(++trace) = s;                 /* trace path to target */
                }
                while (s = *trace--) {            /* retrace entry from LCA */
                    StateOnEvent(s, me, &entryMsg);
                }
                me->curr = me->next;
                me->next = 0;
                while (StateOnEvent(me->curr, me, &startMsg), me->next) {
                    trace = entryPath;
                    *trace = 0;
                    for (s = me->next; s != me->curr; s = s->super) {
                        *(++trace) = s;            /* record path to target */
                    }
                    while (s = *trace--) {             /* retrace the entry */
                        StateOnEvent(s, me, &entryMsg);
                    }
                    me->curr = me->next;
                    me->next = 0;
                }
            }
            break;                                       /* event processed */
        }
    }
}

/* exit current states and all superstates up to LCA .......................*/
void HsmExit_(Hsm *me, unsigned char toLca) {
    register State *s = me->curr;
    while (s != me->source) {
        StateOnEvent(s, me, &exitMsg);
        s = s->super;
    }
    while (toLca--) {
        StateOnEvent(s, me, &exitMsg);
        s = s->super;
    }
    me->curr = s;
}

/* find # of levels to Least Common Ancestor................................*/
unsigned char HsmToLCA_(Hsm *me, State *target) {
    State *s, *t;
    unsigned char toLca = 0;
    if (me->source == target) {
        return 1;
    }
    for (s = me->source; s; ++toLca, s = s->super) {
        for (t = target; t; t = t->super) {
            if (s == t) {
                return toLca;
            }
        }
    }
    return 0;
}
