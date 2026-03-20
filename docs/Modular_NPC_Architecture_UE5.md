# Modular NPC Architecture for UE5 – Scalable Guards & Roaming AI

## Overview

A scalable, fully data-driven NPC AI system using UE5 State Trees and EQS:

- Supports multiple NPC roles: Guards, Civilians, Managers, Scientists, Engineers.
- Each NPC instance can have a unique behavior set and decision strategy.
- Behavior parameters (StopChance, duration, movement speed, etc.) are fully configurable via DataTable or DataAsset.
- AI logic is modular, reusable, and event-driven, avoiding hardcoded states or duplicate code per NPC type.
- Decision strategies (Sequential, RandomWeighted, Hybrid) allow per-NPC behavior variation.

---

## Key Principles

- **Base Character Classes:** Single Blueprint/C++ class per role (e.g., Guard, Civilian).
- **Data-Driven Behavior Config:** Each NPC instance has a config specifying AllowedBehaviors, parameters, and DecisionMode.
- **Modular Behaviors:** Implemented as StateTree tasks/subtrees (WalkAround, StandIdle, DrinkWater, etc.).
- **EQS Integration:** For spatial reasoning (patrol points, chairs, computers, interactable objects).
- **Event-Driven Transitions:** Task completion or probabilities trigger events to select the next state.
- **Randomized & Configurable Decisions:** StopChance, Idle durations, node selection, and next behavior are fully configurable.

---

## 1. Decision Strategy

Each NPC config defines `DecisionMode`, determining how the Root Selector chooses the next behavior:

| DecisionMode         | Description                                     |
|----------------------|-------------------------------------------------|
| RandomWeighted       | Pick next behavior randomly, optionally using weights. |
| Sequential           | Cycle through behaviors in a predefined order.  |
| ConditionalPriority  | Pick behavior based on priority and conditions. |
| Hybrid               | Filter by conditions → then RandomWeighted selection. |

**Example BehaviorConfig snippet:**

```yaml
NPCName: Guard_Patroller
DecisionMode: RandomWeighted
AllowedBehaviors:
  - WalkAround
    Weight: 5
  - StandIdle
    Weight: 2
  - DrinkWater
    Weight: 1
```

---

## 2. StateTree Structure (Conceptual)
```
[Root Selector / Behavior Selector]
        |
        ├─ [BehaviorDecisionTask] # Picks next behavior using DecisionMode & AllowedBehaviors
        |
        ├─ [WalkAround]
        |       - EQS: select next NavNode (filtered by NavNodeTags)
        |       - On StopPoint? → Roll StopChance
        |           - True → Send StopChanceEvent → StandIdle
        |           - False → Send WalkNextNodeEvent → WalkAround
        |
        ├─ [StandIdle]
        |       - Idle duration from config (DurationRange)
        |       - On complete → IdleCompleteEvent → Root Selector
        |
        ├─ [SitAtLocation]
        |       - EQS: find chair by tag
        |       - Duration from config
        |
        ├─ [DrinkWater]
        |       - EQS: find cooler by tag
        |       - Duration from config
        |
        ├─ [WorkAtComputer]
        |       - EQS: find computer by tag
        |       - Duration from config
        |
        └─ [InteractAtObject]
                - EQS: find object by tag
```

- `BehaviorDecisionTask` evaluates DecisionMode and chooses next behavior.
- Tasks only perform actions; transitions are event-driven, avoiding duplicated bool checks in tasks.
- StopChance, durations, and EQS parameters are data-driven per NPC instance.

---

## 3. Behavior Config Examples

**Guard_Patroller (RandomWeighted)**
```yaml
AllowedBehaviors:
  - WalkAround
  - StandIdle
  - DrinkWater
DecisionMode: RandomWeighted
Params:
  WalkAround:
    NavNodeTags: ["PatrolPathA"]
    Speed: 150
    StopChance: 0.5
  DrinkWater:
    ObjectTags: ["LobbyCooler"]
    DurationRange: [3,6]
  StandIdle:
    DurationRange: [5,15]
```

**Guard_Monitoring (Sequential)**
```yaml
AllowedBehaviors:
  - WalkAround
  - WorkAtComputer
  - SitAtLocation
  - DrinkWater
  - StandIdle
DecisionMode: Sequential
Params:
  WalkAround:
    NavNodeTags: ["PatrolPathA"]
  WorkAtComputer:
    ComputerTags: ["VaultComputer", "CameraStation"]
    DurationRange: [30,70]
  SitAtLocation:
    ChairTags: ["RestroomChair"]
```

**Guard_Static (Sequential or RandomWeighted)**
```yaml
AllowedBehaviors:
  - StandIdle
  - InteractAtObject
DecisionMode: Sequential
Params:
  StandIdle:
    DurationRange: [20,80]
  InteractAtObject:
    ObjectTags: ["WaterCooler", "VaultDoor"]
```

---

## 4. WalkAround / StopChance Logic

- EQS chooses next NavNode filtered by NavNodeTags.
- If node is a StopPoint, roll StopChance (0–1 probability):
  - **True:** Trigger StandIdle.
  - **False:** Continue to next node.
- Node selection is fully random, independent of distance.

---

## 5. Workflow for NPC Behavior Execution

1. **NPC Initialization:** Assign BehaviorConfig (DataTable row).
2. **Root Selector Execution:** Reads AllowedBehaviors + DecisionMode.
3. **BehaviorDecisionTask:**
    - Evaluates DecisionMode: RandomWeighted / Sequential / ConditionalPriority.
    - Outputs selected behavior tag.
4. **Execute Behavior Task:**
    - WalkAround → EQS patrol nodes.
    - StandIdle → Wait configured duration.
    - Sit/Drink/Work/Interact → EQS object → perform action → wait duration.
5. **Behavior Completion Event:**
    - Task triggers completion event → Root Selector → pick next behavior according to DecisionMode.

---

## 6. EQS Integration

- **WalkAround:** Randomly pick valid NavNode filtered by tags.
- **SitAtLocation / WorkAtComputer / InteractAtObject:** EQS finds valid targets based on ObjectTags or ChairTags.
- **Decision Filtering:** Only behaviors allowed in AllowedBehaviors are considered by BehaviorDecisionTask.

---

## 7. Instance Customization & Scalability

- Each NPC instance can have unique behaviors, decision mode, and parameters via DataTable row.
- Swap configs at runtime for scenario changes (alert, patrol, emergency).
- Single StateTree asset supports all role variants.
- Adding new behaviors: just add to StateTree + config → no code changes.

---

## 8. Advantages of This Architecture

- **Scalable:** One StateTree, many roles/variants.
- **Flexible:** Per-NPC DecisionMode supports Random, Sequential, or Hybrid selection.
- **Designer-Friendly:** Behavior tuning via DataTable; no Blueprint duplication.
- **Data-Driven:** StopChance, durations, EQS targets fully configurable.
- **Event-Driven:** Reduces task complexity; Root Selector manages behavior transitions.
- **Extensible:** Easily add behaviors, NPC types, or decision strategies.

---

## 9. Summary

- **Roles:** Single class per NPC type, reused for variants.
- **Behaviors:** Modular StateTree tasks/subtrees.
- **DecisionMode:** Configurable per NPC (Sequential / RandomWeighted / Hybrid).
- **Transitions:** Event-driven, data-driven, EQS-based.
- **Variants:** Purely data-defined; no code duplication.
- **Randomness & StopChance:** Handled in tasks/events, fully configurable.
- **Scalability:** Unlimited behavior/role combinations using shared assets.

---

## 10. Appendix: StateTree Diagram (Final)

```
[Root Selector / Behavior Selector]
   ├─ BehaviorDecisionTask (evaluates DecisionMode + AllowedBehaviors)
   ├─ WalkAround
   │     ├─ EQS: select next NavNode
   │     └─ StopPoint? → StopChanceEvent → StandIdle / WalkNextNodeEvent → WalkAround
   ├─ StandIdle
   │     └─ Duration → IdleCompleteEvent → Root Selector
   ├─ SitAtLocation
   │     └─ EQS → Duration → CompletionEvent
   ├─ DrinkWater
   │     └─ EQS → Duration → CompletionEvent
   ├─ WorkAtComputer
   │     └─ EQS → Duration → CompletionEvent
   └─ InteractAtObject
         └─ EQS → Duration → CompletionEvent
```

Root Selector uses BehaviorDecisionTask to pick next allowed behavior per NPC instance.

Supports both randomized and sequential behavior flow.