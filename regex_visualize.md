# 正则表达式可视化

[TOC]

## 1. 单字符

regexp: `a`

nfa:

```mermaid
stateDiagram-v2
direction LR

[*] --> [*] : a
```

dfa:

```mermaid
stateDiagram-v2
direction LR

start_0 --> final_1 : a
```

minimized_dfa:

```mermaid
stateDiagram-v2
direction LR

start_1 --> final_0 : a
```

## 2. 多字符

regexp: `abc`

nfa:

```mermaid
stateDiagram-v2
direction LR

[*] --> state_1 : a
state_1 --> state_2
state_2 --> state_3 : b
state_3 --> state_4
state_4 --> [*] : c
```

dfa:

```mermaid
stateDiagram-v2
direction LR

start_0 --> state_1 : a
state_1 --> state_2 : b
state_2 --> final_3 : c
```

minimized_dfa:

```mermaid
stateDiagram-v2
direction LR

start_1 --> state_2 : a
state_2 --> state_3 : b
state_3 --> final_0 : c
```

## 3. 量词

### 3.1. ?

regexp: `a?`

nfa:

```mermaid
stateDiagram-v2
direction LR

state_0 --> state_1 : a
state_1 --> [*]
[*] --> state_0
[*] --> [*]
```

dfa:

```mermaid
stateDiagram-v2
direction LR

start_0 --> final_1 : a
```

minimized_dfa:

```mermaid
stateDiagram-v2
direction LR

start_0 --> final_1 : a
```

### 3.2. +

regexp: `a+`

nfa:

```mermaid
stateDiagram-v2
direction LR

state_0 --> state_1 : a
state_1 --> [*]
state_1 --> state_0
[*] --> state_0
```

dfa:

```mermaid
stateDiagram-v2
direction LR

start_0 --> final_1 : a
final_1 --> final_1 : a
```

minimized_dfa:

```mermaid
stateDiagram-v2
direction LR

start_1 --> final_0 : a
final_0 --> final_0 : a
```

### 3.3. *

regexp: `a*`

nfa:

```mermaid
stateDiagram-v2
direction LR

state_0 --> state_1 : a
state_1 --> [*]
state_1 --> state_0
[*] --> state_0
[*] --> [*]
```

dfa:

```mermaid
stateDiagram-v2
direction LR

start_0 --> final_1 : a
final_1 --> final_1 : a
```

minimized_dfa:

```mermaid
stateDiagram-v2
direction LR

start_0 --> start_0 : a
```

## 4. 字符集

regexp: `[abc]`

nfa:

```mermaid
stateDiagram-v2
direction LR

[*] --> [*] : a
[*] --> [*] : b
[*] --> [*] : c
```

dfa:

```mermaid
stateDiagram-v2
direction LR

start_0 --> final_1 : a
start_0 --> final_1 : b
start_0 --> final_1 : c
```

minimized_dfa:

```mermaid
stateDiagram-v2
direction LR

start_1 --> final_0 : a
start_1 --> final_0 : b
start_1 --> final_0 : c
```

## 5. 选择

regexp: `a|b|c`

nfa:

```mermaid
stateDiagram-v2
direction LR

state_4 --> state_5 : a
state_5 --> state_9
state_6 --> state_7 : b
state_7 --> state_9
state_8 --> state_4
state_8 --> state_6
state_9 --> [*]
state_10 --> state_11 : c
state_11 --> [*]
[*] --> state_8
[*] --> state_10
```

dfa:

```mermaid
stateDiagram-v2
direction LR

start_0 --> final_1 : a
start_0 --> final_2 : b
start_0 --> final_3 : c
```

minimized_dfa:

```mermaid
stateDiagram-v2
direction LR

start_1 --> final_0 : a
start_1 --> final_0 : b
start_1 --> final_0 : c
```

## 6. 复杂案例

regexp: `([abc]abc)*|(d|e|f)ghi[2-5]+`

nfa:

```mermaid
stateDiagram-v2
direction LR

state_2 --> state_3 : a
state_2 --> state_3 : b
state_2 --> state_3 : c
state_3 --> state_4
state_4 --> state_5 : a
state_5 --> state_6
state_6 --> state_7 : b
state_7 --> state_8
state_8 --> state_9 : c
state_9 --> state_11
state_9 --> state_2
state_10 --> state_2
state_10 --> state_11
state_11 --> [*]
state_16 --> state_17 : d
state_17 --> state_21
state_18 --> state_19 : e
state_19 --> state_21
state_20 --> state_16
state_20 --> state_18
state_21 --> state_25
state_22 --> state_23 : f
state_23 --> state_25
state_24 --> state_20
state_24 --> state_22
state_25 --> state_26
state_26 --> state_27 : g
state_27 --> state_28
state_28 --> state_29 : h
state_29 --> state_30
state_30 --> state_31 : i
state_31 --> state_34
state_32 --> state_33 : 2
state_32 --> state_33 : 3
state_32 --> state_33 : 4
state_32 --> state_33 : 5
state_33 --> state_35
state_33 --> state_32
state_34 --> state_32
state_35 --> [*]
[*] --> state_10
[*] --> state_24
```

dfa:

```mermaid
stateDiagram-v2
direction LR

start_0 --> state_1 : a
start_0 --> state_1 : b
start_0 --> state_1 : c
start_0 --> state_2 : d
start_0 --> state_3 : e
start_0 --> state_4 : f
state_1 --> state_9 : a
state_2 --> state_5 : g
state_3 --> state_5 : g
state_4 --> state_5 : g
state_5 --> state_6 : h
state_6 --> state_7 : i
state_7 --> final_8 : 2
state_7 --> final_8 : 3
state_7 --> final_8 : 4
state_7 --> final_8 : 5
state_9 --> state_10 : b
state_10 --> final_11 : c
final_11 --> state_1 : a
final_11 --> state_1 : b
final_11 --> state_1 : c
final_8 --> final_8 : 2,3,4,5
```

minimized_dfa:

```mermaid
stateDiagram-v2
direction LR

start_0 --> state_3 : a
start_0 --> state_3 : b
start_0 --> state_3 : c
start_0 --> state_4 : d
start_0 --> state_4 : e
start_0 --> state_4 : f
final_2 --> state_3 : a
final_2 --> state_3 : b
final_2 --> state_3 : c
state_3 --> state_8 : a
state_4 --> state_5 : g
state_5 --> state_6 : h
state_6 --> state_7 : i
state_7 --> final_1 : 2
state_7 --> final_1 : 3
state_7 --> final_1 : 4
state_7 --> final_1 : 5
state_8 --> state_9 : b
state_9 --> final_2 : c
final_1 --> final_1 : 2,3,4,5
```

