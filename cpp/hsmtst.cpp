/**  hsmtest.c -- Hierarchical State Machine test harness.
 *   This is an implementation of the example found in
 *   Practical StateCharts in C/C++ by Miro Samek.
 *   Intent of program is to exercise the state machine implementation.
 *   An earlier implementation published in ESP August 2000 had errors
 *   that were identified by kevin.fleming@philips.com
 *   M. Samek 02-11-25
 */

#include "hsm.hpp"

#include <assert.h>
#include <stdio.h>

class HsmTest : public Hsm {
    int myFoo;
protected:
    State s1;
      State s11;
    State s2;
      State s21;
        State s211;
public:
    HsmTest();
    Msg const *topHndlr(Msg const *msg);
    Msg const *s1Hndlr(Msg const *msg);
    Msg const *s11Hndlr(Msg const *msg);
    Msg const *s2Hndlr(Msg const *msg);
    Msg const *s21Hndlr(Msg const *msg);
    Msg const *s211Hndlr(Msg const *msg);
};

enum HsmTestEvents {
    A_SIG, B_SIG, C_SIG, D_SIG, E_SIG, F_SIG, G_SIG, H_SIG
};

Msg const *HsmTest::topHndlr(Msg const *msg) {
    switch (msg->evt) {
    case START_EVT:
        printf("top-INIT;");
        STATE_START(&s1);
        return 0;
    case ENTRY_EVT:
        printf("top-ENTRY;");
        return 0;
    case EXIT_EVT:
        printf("top-EXIT;");
        return 0;
    case E_SIG:
        printf("top-E;");
        STATE_TRAN(&s211);
        return 0;
    }
    return msg;
}

Msg const *HsmTest::s1Hndlr(Msg const *msg) {
    switch (msg->evt) {
    case START_EVT:
        printf("s1-INIT;");
        STATE_START(&s11);
        return 0;
    case ENTRY_EVT:
        printf("s1-ENTRY;");
        return 0;
    case EXIT_EVT:
        printf("s1-EXIT;");
        return 0;
    case A_SIG:
        printf("s1-A;");
        STATE_TRAN(&s1);
        return 0;
    case B_SIG:
        printf("s1-B;");
        STATE_TRAN(&s11);
        return 0;
    case C_SIG:
        printf("s1-C;");
        STATE_TRAN(&s2);
        return 0;
    case D_SIG:
        printf("s1-D;");
        STATE_TRAN(&top);
        return 0;
    case F_SIG:
        printf("s1-F;");
        STATE_TRAN(&s211);
        return 0;
    }
    return msg;
}

Msg const *HsmTest::s11Hndlr(Msg const *msg) {
    switch (msg->evt) {
    case ENTRY_EVT:
        printf("s11-ENTRY;");
        return 0;
    case EXIT_EVT:
        printf("s11-EXIT;");
        return 0;
    case G_SIG:
        printf("s11-G;");
        STATE_TRAN(&s211);
        return 0;
    case H_SIG:
        if (myFoo) {
            printf("s11-H;");
            myFoo = 0;
            return 0;
        }
        break;
    }
    return msg;
}

Msg const *HsmTest::s2Hndlr(Msg const *msg) {
    switch (msg->evt) {
    case START_EVT:
        printf("s2-INIT;");
        STATE_START(&s21);
        return 0;
    case ENTRY_EVT:
        printf("s2-ENTRY;");
        return 0;
    case EXIT_EVT:
        printf("s2-EXIT;");
        return 0;
    case C_SIG:
        printf("s2-C;");
        STATE_TRAN(&s1);
        return 0;
    case F_SIG:
        printf("s2-F;");
        STATE_TRAN(&s11);
        return 0;
    }
    return msg;
}

Msg const *HsmTest::s21Hndlr(Msg const *msg) {
    switch (msg->evt) {
    case START_EVT:
        printf("s21-INIT;");
        STATE_START(&s211);
        return 0;
    case ENTRY_EVT:
        printf("s21-ENTRY;");
        return 0;
    case EXIT_EVT:
        printf("s21-EXIT;");
        return 0;
    case B_SIG:
        printf("s21-B;");
        STATE_TRAN(&s211);
        return 0;
    case H_SIG:
        if (!myFoo) {
            printf("s21-H;");
            myFoo = 1;
            STATE_TRAN(&s21);
            return 0;
        }
        break;
    }
    return msg;
}

Msg const *HsmTest::s211Hndlr(Msg const *msg) {
    switch (msg->evt) {
    case ENTRY_EVT:
        printf("s211-ENTRY;");
        return 0;
    case EXIT_EVT:
        printf("s211-EXIT;");
        return 0;
    case D_SIG:
        printf("s211-D;");
        STATE_TRAN(&s21);
        return 0;
    case G_SIG:
        printf("s211-G;");
        STATE_TRAN(&top);
        return 0;
    }
    return msg;
}

HsmTest::HsmTest()
: Hsm("HsmTest", (EvtHndlr)topHndlr),
    s1("s1", &top, (EvtHndlr)&HsmTest::s1Hndlr),
    s11("s11", &s1, (EvtHndlr)&HsmTest::s11Hndlr),
    s2("s2", &top, (EvtHndlr)&HsmTest::s2Hndlr),
    s21("s21", &s2, (EvtHndlr)&HsmTest::s21Hndlr),
    s211("s211", &s21, (EvtHndlr)&HsmTest::s211Hndlr)
{
    myFoo = 0;
}

const Msg HsmTestMsg[] = {
    A_SIG,B_SIG,C_SIG,D_SIG,E_SIG,F_SIG,G_SIG,H_SIG
};

int main() {
    HsmTest hsmTest;

    printf("Events:\n"
        "a-h for triggering events\n"
        "x to exit\n\n");

    hsmTest.onStart();
    for (;;) {
        char c;
        printf("\nEvent<-");
        c = getc(stdin);
        getc(stdin);
        if (c < 'a' || 'h' < c) {
            break;
        }
        hsmTest.onEvent(&HsmTestMsg[c - 'a']);
    }
    return 0;
}
