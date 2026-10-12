/*
 * fsm: the vent's state machine. States: CLOSED, OPENING, OPEN, CLOSING,
 * and STORM_LOCK. A transition table, indexed by state and event, gives the
 * next state and action. Events come from the readings (too hot or humid,
 * cool and dry enough), from the motor finishing after 20 ticks, and from
 * the WIND_ALARM switch.
 */
#ifndef FSM_H
#define FSM_H

#endif /* FSM_H */
