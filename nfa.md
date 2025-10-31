# NFA 可视化

<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [1. 单字符NFA](#1-单字符nfa)
- [2. 多字符NFA](#2-多字符nfa)
- [3. 量词](#3-量词)
  - [3.1. ?](#31-)
  - [3.2. +](#32-)
  - [3.3. \*](#33-)
- [4. 字符集](#4-字符集)
- [5. 选择](#5-选择)
- [6. 复杂案例](#6-复杂案例)

<!-- /code_chunk_output -->

## 1. 单字符NFA

a:

```mermaid
stateDiagram-v2
direction LR

[*] --> [*] : a
```

## 2. 多字符NFA

abc:

```mermaid
stateDiagram-v2
direction LR

[*] --> state_1 : a
state_1 --> state_2
state_2 --> state_3 : b
state_3 --> state_4
state_4 --> [*] : c
```

## 3. 量词

### 3.1. ?

a?:

```mermaid
stateDiagram-v2
direction LR

state_0 --> state_1 : a
state_1 --> [*]
[*] --> state_0
[*] --> [*]
```

### 3.2. +

a+:

```mermaid
stateDiagram-v2
direction LR

state_0 --> state_1 : a
state_1 --> [*]
state_1 --> state_0
[*] --> state_0
```

### 3.3. *

a*:

```mermaid
stateDiagram-v2
direction LR

state_0 --> state_1 : a
state_1 --> [*]
state_1 --> state_0
[*] --> state_0
[*] --> [*]
```

## 4. 字符集

[abc]:

```mermaid
stateDiagram-v2
direction LR

[*] --> [*] : a
[*] --> [*] : b
[*] --> [*] : c
```

## 5. 选择

a|b|c:

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

## 6. 复杂案例

[abc]*|(d|e|f)ghi[2-5]+:

```mermaid
stateDiagram-v2
direction LR

state_2 --> state_3 : a
state_2 --> state_3 : b
state_2 --> state_3 : c
state_3 --> state_5
state_3 --> state_2
state_4 --> state_2
state_4 --> state_5
state_5 --> [*]
state_10 --> state_11 : d
state_11 --> state_15
state_12 --> state_13 : e
state_13 --> state_15
state_14 --> state_10
state_14 --> state_12
state_15 --> state_19
state_16 --> state_17 : f
state_17 --> state_19
state_18 --> state_14
state_18 --> state_16
state_19 --> state_20
state_20 --> state_21 : g
state_21 --> state_22
state_22 --> state_23 : h
state_23 --> state_24
state_24 --> state_25 : i
state_25 --> state_28
state_26 --> state_27 : 2
state_26 --> state_27 : 3
state_26 --> state_27 : 4
state_26 --> state_27 : 5
state_27 --> state_29
state_27 --> state_26
state_28 --> state_26
state_29 --> [*]
[*] --> state_4
[*] --> state_18
```
