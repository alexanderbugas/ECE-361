# Greenhouse vent: specification

Project 18, ECE 361, Fall 2026. First version (week 2): the states and the
events. The transition table comes in week 5.

## States

Given by the project sheet.

| State | Meaning |
|---|---|
| `CLOSED` | vent shut (initial state) |
| `OPENING` | motor opening |
| `OPEN` | vent fully open |
| `CLOSING` | motor closing |
| `STORM_LOCK` | vent shut and locked because of wind |

## Events

The events cover the greenhouse heating up, cooling down, and the wind. Each
one comes from an operating rule on the project sheet.

| Event | Happens when | Rule |
|---|---|---|
| `COOL_ENABLE` | the temperature is at or above 28.0 °C (too hot) | 1 |
| `HUMID_TRIGGER_HI` | the humidity is at or above 85 %RH (too humid) | 1 |
| `COOL_LIMIT` | the temperature is at or below 24.0 °C | 1 |
| `HUMID_TRIGGER_LOW` | the humidity is at or below 75 %RH | 1 |
| `MOTOR_EN_20` | the motor has been running for 20 ticks | 2 |
| `WIND_ALARM_ON` | the WIND_ALARM switch is on (storm) | 3 |
| `WIND_ALARM_OFF_120` | the WIND_ALARM switch has been off for 120 ticks | 3 |

How the events combine, from rule 1:

- The vent opens on `COOL_ENABLE` **or** `HUMID_TRIGGER_HI`.
- The vent closes only on `COOL_LIMIT` **and** `HUMID_TRIGGER_LOW` together.
- Between the thresholds (above 24.0 and below 28.0 °C, or humidity between
  75 and 85 %RH), the vent stays as it is. This gap is the hysteresis that
  keeps the motor from chattering.

## Inputs

- The wind alarm is one switch: on (storm) or off. There is no wind speed.
- The device accepts every value the sensor can report: about −50 to 150 °C
  and 0 to 100 %RH.

## Still to come

- The transition table: state, event, next state, action (week 5)
- A drawing of the state machine
- The decisions where a rule is silent, for example the temperature falling
  while `OPENING`, high humidity while the temperature is low, and the wind
  alarm going off and on again within the 120 ticks
