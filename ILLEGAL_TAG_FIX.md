# Illegal Tag Issue - Fix Documentation

## Problem Description

The illegal tag (`Character.Status.Illegal`) was sometimes getting stuck on players even when they were no longer in illegal areas or performing illegal actions. This issue occurred with the Blueprint `BP_IllegalArea` which uses collision overlap events to apply/remove a gameplay effect (`GE_Illegal`).

## Root Causes Identified

1. **Inconsistent Tag Usage in C++**
   - The code used string-based `FGameplayTag::RequestGameplayTag("Character.Status.Illegal")` instead of the native tag constant `TAG_Character_Status_Illegal`
   - This could cause issues with tag hierarchy matching and is less efficient
   - Found in 3 locations in `DefaultCharacter.cpp`

2. **Blueprint Overlap Event Issues**
   - Overlap events can fire multiple times for the same actor
   - Without proper tracking, the same gameplay effect could be applied multiple times
   - If applied multiple times but only removed once, the tag remains stuck
   - Network replication timing could cause mismatched apply/remove calls

3. **No Duplicate Prevention**
   - The Blueprint implementation had no safeguard against applying the same effect multiple times
   - No tracking of which actors currently have the effect applied

## Changes Made

### 1. Fixed C++ Tag References (DefaultCharacter.cpp)

**Line 120** - Detection tick check:
```cpp
// Before:
bool bIsIllegal = AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Character.Status.Illegal"));

// After:
bool bIsIllegal = AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(TAG_Character_Status_Illegal);
```

**Line 183** - Tag event registration:
```cpp
// Before:
AbilitySystemComponent->RegisterGameplayTagEvent(
    FGameplayTag::RequestGameplayTag("Character.Status.Illegal"),
    EGameplayTagEventType::NewOrRemoved
).AddUObject(this, &ADefaultCharacter::OnIllegalTagChanged);

// After:
AbilitySystemComponent->RegisterGameplayTagEvent(
    TAG_Character_Status_Illegal,
    EGameplayTagEventType::NewOrRemoved
).AddUObject(this, &ADefaultCharacter::OnIllegalTagChanged);
```

**Line 248** - OnDetected check:
```cpp
// Before:
if (!AbilitySystemComponent || !AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Character.Status.Illegal")))

// After:
if (!AbilitySystemComponent || !AbilitySystemComponent->HasMatchingGameplayTag(TAG_Character_Status_Illegal))
```

### 2. Created New C++ IllegalArea Class

Created `AIllegalArea` (IllegalArea.h/cpp) as a robust C++ implementation that:

**Key Features:**
- Tracks active gameplay effect handles per actor using a `TMap`
- Prevents duplicate effect applications by checking if actor already has an effect
- Ensures effects are properly removed by storing and using the effect handle
- Server-authoritative (effects only applied on server)
- Automatic cleanup of invalid handles

**Implementation Highlights:**
```cpp
// Prevent duplicate applications
if (ActiveEffectHandles.Contains(OtherActor))
{
    return;
}

// Store handle for later removal
FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
if (ActiveHandle.IsValid())
{
    ActiveEffectHandles.Add(OtherActor, ActiveHandle);
}

// Properly remove using stored handle
FActiveGameplayEffectHandle* HandlePtr = ActiveEffectHandles.Find(OtherActor);
if (HandlePtr && HandlePtr->IsValid())
{
    ASC->RemoveActiveGameplayEffect(*HandlePtr);
}
```

## How to Use the Fix

### Option 1: Use the New C++ Class (Recommended)

1. In your level, replace `BP_IllegalArea` instances with `AIllegalArea`
2. Set the `IllegalAreaEffect` property to your gameplay effect class (GE_Illegal)
3. Adjust the `AreaVolume` extent to match your desired illegal zone size
4. The class will handle everything automatically

### Option 2: Update the Blueprint

If you prefer to keep using `BP_IllegalArea`, update it to follow the same pattern:

1. Add a Map variable to track active effect handles per actor
2. In BeginOverlap:
   - Check if actor is already in the map
   - If yes, don't apply effect again
   - If no, apply effect and store the handle
3. In EndOverlap:
   - Look up the handle from the map
   - Remove the effect using that specific handle
   - Remove the actor from the map

### Option 3: Create a Blueprint Child

You can create a Blueprint child of `AIllegalArea` if you need Blueprint customization while keeping the robust C++ logic.

## Tag Hierarchy Explanation

The illegal tag system uses a hierarchy:
- `Character.Status.Illegal` (parent tag)
- `Character.Status.Illegal.Action` (child - applied by illegal abilities)
- `Character.Status.Illegal.Area` (child - applied by illegal areas)

When checking `HasMatchingGameplayTag(TAG_Character_Status_Illegal)`, Unreal will return true if:
- The exact parent tag exists, OR
- Any child tag exists (Action or Area)

The `OnIllegalTagChanged` callback receives a count parameter:
- Count = 0: No illegal tags present
- Count = 1: One illegal tag present (either Action or Area)
- Count = 2: Both Action and Area tags present

This is why removing just one source doesn't clear the "illegal" status if multiple sources applied tags.

## Testing the Fix

1. **Single Entry/Exit Test**
   - Enter illegal area → tag should be applied
   - Exit illegal area → tag should be removed
   - Verify with debug display or status UI

2. **Multiple Entry Test**
   - Enter and exit the same illegal area multiple times rapidly
   - Tag should never get stuck
   - Each exit should properly remove the tag

3. **Overlapping Areas Test**
   - Create two overlapping illegal areas
   - Enter area 1 → tag applied
   - Move to overlap both → still tagged
   - Move to only area 2 → still tagged
   - Exit both → tag removed

4. **Action + Area Test**
   - Enter illegal area → Area tag applied
   - Perform illegal action (hack/lockpick) → Action tag applied
   - Stop action → only Area tag remains
   - Exit area → all tags removed

5. **Multiplayer Test**
   - Test in multiplayer session
   - Verify tag application/removal works for all clients
   - Check that tags replicate correctly

## Additional Notes

- The C++ class is network-safe and server-authoritative
- Effect handles ensure proper cleanup even if actors are destroyed while in the area
- The implementation follows Unreal best practices for gameplay ability system
- Native tags are more efficient and type-safe than string-based lookups

## Future Improvements

Consider these enhancements if issues persist:

1. Add logging to track when effects are applied/removed
2. Implement a periodic cleanup check to remove orphaned tags
3. Add visual debug information showing active illegal areas
4. Create a debug command to force-clear all illegal tags
