/**
* hsm.cpp -- Hierarchical State Machine implementation
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
#include <assert.h>
#include "hsm.hpp"

static Msg const startMsg = { START_EVT };
static Msg const entryMsg = { ENTRY_EVT };
static Msg const exitMsg  = { EXIT_EVT };
#define MAX_STATE_NESTING 8

/* State Ctor...............................................................*/
State::State(char const *n, State *s, EvtHndlr h)
        : name(n), super(s), hndlr(h)
{}

/* Hsm Ctor.................................................................*/
Hsm::Hsm(char const *n, EvtHndlr topHndlr)
        : top("top", 0, topHndlr), name(n)
{}

/* enter and start the top state............................................*/
void Hsm::onStart() {
    curr = &top;
    next = 0;
    curr->onEvent(this, &entryMsg);
    while (curr->onEvent(this, &startMsg), next) {
        State *entryPath[MAX_STATE_NESTING];
        register State **trace = entryPath;
        register State *s;
        *trace = 0;
        for (s = next; s != curr; s = s->super) {
            *(++trace) = s;                         /* trace path to target */
        }
        while (s = *trace--) {                 /* retrace entry from source */
            s->onEvent(this, &entryMsg);
        }
        curr = next;
        next = 0;
    }
}

/* state machine "engine"...................................................*/
void Hsm::onEvent(Msg const *msg) {
    State *entryPath[MAX_STATE_NESTING];
    register State **trace;
    register State *s;
    for (s = curr; s; s = s->super) {
        source = s;                     /* level of outermost event handler */
        msg = s->onEvent(this, msg);
        if (msg == 0) {                                       /* processed? */
            if (next) {                          /* state transition taken? */
                trace = entryPath;
                *trace = 0;
                for (s = next; s != curr; s = s->super) {
                    *(++trace) = s;                 /* trace path to target */
                }
                while (s = *trace--) {            /* retrace entry from LCA */
                    s->onEvent(this, &entryMsg);
                }
                curr = next;
                next = 0;
                while (curr->onEvent(this, &startMsg), next) {
                    trace = entryPath;
                    *trace = 0;
                    for (s = next; s != curr; s = s->super) {
                        *(++trace) = s;            /* record path to target */
                    }
                    while (s = *trace--) {             /* retrace the entry */
                        s->onEvent(this, &entryMsg);
                    }
                    curr = next;
                    next = 0;
                }
            }
            break;                                       /* event processed */
        }
    }
}

/* exit current states and all superstates up to LCA .......................*/
void Hsm::exit_(unsigned char toLca) {
    register State *s = curr;
    while (s != source) {
        s->onEvent(this, &exitMsg);
        s = s->super;
    }
    while (toLca--) {
        s->onEvent(this, &exitMsg);
        s = s->super;
    }
    curr = s;
}

/* find # of levels to Least Common Ancestor................................*/
unsigned char Hsm::toLCA_(State *target) {
    State *s, *t;
    unsigned char toLca = 0;
    if (source == target) {
        return 1;
    }
    for (s = source; s; ++toLca, s = s->super) {
        for (t = target; t; t = t->super) {
            if (s == t) {
                return toLca;
            }
        }
    }
    return 0;
}
