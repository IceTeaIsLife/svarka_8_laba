#include <stdbool.h>
#include "stddef.h"
#include "welding_automate.h"

#define TIME_DELTA(t) ((t) * 5)

#if defined(__cplusplus)
extern "C" {
#endif

static void automate_tick(struct Automate *automate, enum InputSignal signal, double value_modifier) {
    switch (automate->state) {
        case AUTOMATE_OFF: {
            if (signal == WELDING_ON) {
                automate->state = AUTOMATE_UP;
                automate->time_tick = 0;
            }
            break;
        }
        case AUTOMATE_UP: {
            if (signal == WELDING_ON) {
                automate->time_tick++;
                if(automate->time_tick >= automate->time_n) {
                    automate->state = AUTOMATE_WELDING;
                } else {
                    automate->value = automate->value + value_modifier;
                }
            } else {
                automate->state = AUTOMATE_DOWN;
            }
            break;
        }
        case AUTOMATE_WELDING: {
            if (signal == WELDING_ON) {
                automate->state = AUTOMATE_WELDING;
            } else {
                automate->state = AUTOMATE_DOWN;
                automate->time_tick = 0;
            }
            break;
        }
        case AUTOMATE_DOWN: {
            automate->time_tick++;
            if(automate->time_tick >= automate->time_c) {
                automate->state = AUTOMATE_COMPLETE;
            } else {
                automate->value = automate->value - value_modifier;
            }
            break;
        }
        case AUTOMATE_COMPLETE: {
            automate->state = AUTOMATE_COMPLETE;
            break;
        }
    }
}

void engine_init(struct Engine *engine) {
    if(engine != NULL) {
        engine_reset(engine);
    }
}

void engine_reset(struct Engine *engine) {
    engine->state = ENGINE_WELDING_OFF;
    engine->time_tick = 0;
    engine->time_n = TIME_DELTA(10);
    engine->time_c = TIME_DELTA(10);
    engine->time_g = TIME_DELTA(5);
    engine->value = 0;
    engine->value_modifier[AUTOMATE_I_GRAPH] = 1.25;
    engine->value_modifier[AUTOMATE_U_GRAPH] = 1;
    engine->value_modifier[AUTOMATE_V_GRAPH] = 1;
    engine->value_modifier[AUTOMATE_GAS_GRAPH] = 0.75;

    engine->automate[AUTOMATE_I].state = AUTOMATE_OFF;
    engine->automate[AUTOMATE_I].time_tick = 0;
    engine->automate[AUTOMATE_I].time_n = TIME_DELTA(8);
    engine->automate[AUTOMATE_I].time_c = TIME_DELTA(8);
    engine->automate[AUTOMATE_I].value = 0;

    engine->automate[AUTOMATE_U].state = AUTOMATE_OFF;
    engine->automate[AUTOMATE_U].time_tick = 0;
    engine->automate[AUTOMATE_U].time_n = TIME_DELTA(5);
    engine->automate[AUTOMATE_U].time_c = TIME_DELTA(5);
    engine->automate[AUTOMATE_U].value = 0;

    engine->automate[AUTOMATE_V].state = AUTOMATE_OFF;
    engine->automate[AUTOMATE_V].time_tick = 0;
    engine->automate[AUTOMATE_V].time_n = TIME_DELTA(3);
    engine->automate[AUTOMATE_V].time_c = TIME_DELTA(3);
    engine->automate[AUTOMATE_V].value = 0;
}

void engine_tick(struct Engine *engine, enum InputSignal signal) {
    switch (engine->state) {
        /*Выключено*/
        case ENGINE_WELDING_OFF: {
            if (signal == WELDING_ON) {
                engine->state = ENGINE_WELDING_GAS_ON;
                engine->time_tick = 0;
            } else {
                engine->state = ENGINE_WELDING_OFF;
            }
            break;
        }
        /*Подача газа*/
        case ENGINE_WELDING_GAS_ON: {
            if (signal == WELDING_ON) {
                engine->time_tick++;
                if(engine->time_tick >= engine->time_n) {
                    engine->state = ENGINE_WELDING_RUN_AUTOMATE;
                    engine->time_tick = 0;
                } else {
                    engine->value = engine->value + engine->value_modifier[AUTOMATE_GAS_GRAPH];
                }
            } else {
                engine->state = ENGINE_WELDING_END_AUTOMATE;
                engine->time_tick = 0;
            }
            break;
        }
        /*Запуск внутренних автоматов*/
        case ENGINE_WELDING_RUN_AUTOMATE: {
            if (signal == WELDING_ON) {
                int i;
                bool welding = true;
                for (i = 0; i < AUTOMATE_COUNT; i++) {
                    automate_tick(&engine->automate[i], signal, engine->value_modifier[i]);
                    welding = welding && (engine->automate[i].state == AUTOMATE_WELDING);
                }
                if (welding) {
                    engine->state = ENGINE_WELDING_RUNNING;
                } else {
                    engine->state = ENGINE_WELDING_RUN_AUTOMATE;
                }
            } else {
                engine->state = ENGINE_WELDING_END_AUTOMATE;
                engine->time_tick = 0;
            }
            break;
        }
        /*Процесс сварки*/
        case ENGINE_WELDING_RUNNING: {
            if (signal == WELDING_OFF) {
                engine->state = ENGINE_WELDING_END_AUTOMATE;
            }
            break;
        }
        /*Выключение внутренних автоматов*/
        case ENGINE_WELDING_END_AUTOMATE: {
            int i;
            bool complete = true;
            for (i = 0; i < AUTOMATE_COUNT; i++) {
                automate_tick(&engine->automate[i], signal, engine->value_modifier[i]);
                complete = complete && (engine->automate[i].state == AUTOMATE_COMPLETE);
            }
            if (complete) {
                engine->state = ENGINE_WELDING_GAS_STABLE;
                engine->time_tick = 0;
            }
            break;
        }
        /*Ожидание прекращения подачи газа (технический момент)*/
        case ENGINE_WELDING_GAS_STABLE: {
            engine->time_tick++;
            if(engine->time_tick >= engine->time_g) {
                engine->state = ENGINE_WELDING_GAS_OFF;
                engine->time_tick = 0;
            }
            break;
        }
        /*Выключение газа*/
        case ENGINE_WELDING_GAS_OFF: {
            engine->time_tick++;
            if(engine->time_tick >= engine->time_c) {
                engine->state = ENGINE_WELDING_COMPLETE;
                engine->time_tick = 0;
            } else {
                /*engine->value--;*/
                engine->value = engine->value - engine->value_modifier[AUTOMATE_GAS_GRAPH];
            }
            break;
        }
        /*Конец работы*/
        case ENGINE_WELDING_COMPLETE: {
            engine_reset(engine);
            break;
        }
    }
}

#if defined(__cplusplus)
}
#endif
