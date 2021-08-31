#ifndef PROJECT_4_WELDING_AUTOMATE_H
#define PROJECT_4_WELDING_AUTOMATE_H

#if defined(__cplusplus)
extern "C" {
#endif

enum InputSignal {
    WELDING_OFF = 0,
    WELDING_ON = 1
};

enum AutomateState {
    AUTOMATE_OFF = 0,
    AUTOMATE_UP = 1,
    AUTOMATE_WELDING = 2,
    AUTOMATE_DOWN = 3,
    AUTOMATE_COMPLETE = 4
};

enum AutomateGraph{
    AUTOMATE_I_GRAPH = 0,
    AUTOMATE_U_GRAPH = 1,
    AUTOMATE_V_GRAPH = 2,
    AUTOMATE_GAS_GRAPH = 3,
};

/*Вложенный автомат*/
struct Automate {
    enum AutomateState state;
    int time_tick;
    int time_n;
    int time_c;
    double value;
};

enum AutomateParameter {
    AUTOMATE_I = 0,     /*ток сварки*/
    AUTOMATE_U = 1,     /*напряжение дуги*/
    AUTOMATE_V = 2,     /*скорость подачи проволоки*/
    AUTOMATE_COUNT = 3
};

enum  EngineState {
    ENGINE_WELDING_OFF = 0,
    ENGINE_WELDING_GAS_ON = 1,
    ENGINE_WELDING_RUN_AUTOMATE = 2,
    ENGINE_WELDING_RUNNING = 3,
    ENGINE_WELDING_END_AUTOMATE = 4,
    ENGINE_WELDING_GAS_STABLE = 5,
    ENGINE_WELDING_GAS_OFF = 6,
    ENGINE_WELDING_COMPLETE = 7
};

/*Основной автомат*/
struct Engine {
    enum EngineState state;
    int time_tick;
    int time_n;     /*время насыщения*/
    int time_g;     /*время окончательной подачи газа*/
    int time_c;     /*время спада*/
    struct Automate automate[3];
    double value;
    double value_modifier[4];
};

void engine_init(struct Engine *engine);

void engine_tick(struct Engine *engine, enum InputSignal signal);

void engine_reset(struct Engine *engine);

#if defined(__cplusplus)
}
#endif

#endif //PROJECT_4_WELDING_AUTOMATE_H
