# Contributing to Splitstream

Thank you for your interest in contributing to Splitstream! This document provides guidelines and best practices for contributing to the project.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Process](#development-process)
- [Coding Standards](#coding-standards)
- [Pull Request Process](#pull-request-process)
- [Commit Guidelines](#commit-guidelines)
- [Testing Guidelines](#testing-guidelines)
- [Documentation](#documentation)

---

## Code of Conduct

### Our Pledge

We are committed to providing a welcoming and inspiring community for all. Please be respectful and constructive in your interactions.

### Expected Behavior

- Be respectful and inclusive
- Welcome newcomers and help them get started
- Be open to constructive criticism
- Focus on what is best for the community and project

### Unacceptable Behavior

- Harassment, discrimination, or trolling
- Personal attacks or insulting comments
- Spam or off-topic content
- Sharing private information without permission

---

## Getting Started

### Prerequisites

Before contributing, ensure you have:

1. ✅ Read the [README.md](../README.md)
2. ✅ Followed the [QUICK_START.md](QUICK_START.md) guide
3. ✅ Set up your development environment
4. ✅ Successfully built and run the project

### Finding Something to Work On

**Good First Issues:**
- Look for issues labeled `good first issue` on GitHub
- These are suitable for newcomers to the project

**Bug Fixes:**
- Check issues labeled `bug`
- Reproduce the bug before starting work
- Add test cases if possible

**Feature Requests:**
- Look for issues labeled `enhancement`
- Discuss implementation approach before starting
- Break large features into smaller tasks

**Documentation:**
- Improve existing documentation
- Add examples and tutorials
- Fix typos and unclear explanations

### Claiming an Issue

1. Comment on the issue: "I'd like to work on this"
2. Wait for maintainer approval
3. Start working within a reasonable timeframe
4. Ask questions if you're stuck

---

## Development Process

### 1. Fork and Clone

```bash
# Fork the repository on GitHub (click "Fork" button)

# Clone your fork
git clone https://github.com/YOUR_USERNAME/Splitstream.git
cd Splitstream

# Add upstream remote
git remote add upstream https://github.com/Traveler3114/Splitstream.git
```

### 2. Create a Branch

```bash
# Update your local main branch
git checkout main
git pull upstream main

# Create a feature branch
git checkout -b feature/your-feature-name
# Or for bug fixes
git checkout -b fix/issue-description
```

**Branch Naming Conventions:**
- `feature/` - New features
- `fix/` - Bug fixes
- `docs/` - Documentation changes
- `refactor/` - Code refactoring
- `test/` - Adding tests
- `perf/` - Performance improvements

### 3. Make Changes

- Follow [Coding Standards](#coding-standards)
- Write clean, maintainable code
- Add comments for complex logic
- Update documentation if needed

### 4. Test Your Changes

```bash
# Build the project
Ctrl + Shift + B (in Visual Studio)

# Test in editor
Alt + P (Play in Editor)

# Test multiplayer (if applicable)
Set Number of Players: 2

# Run automated tests (if available)
Window → Test Automation → Run Tests
```

### 5. Commit Changes

```bash
# Stage your changes
git add .

# Commit with descriptive message
git commit -m "Add feature: description of what you did"

# Push to your fork
git push origin feature/your-feature-name
```

### 6. Create Pull Request

1. Go to your fork on GitHub
2. Click **Pull Request** button
3. Fill out the PR template
4. Link related issues
5. Submit for review

---

## Coding Standards

### C++ Style Guide

Follow [Epic's Coding Standard](https://docs.unrealengine.com/5.7/en-US/epic-cplusplus-coding-standard-for-unreal-engine/) with these highlights:

#### Naming Conventions

**Classes:**
```cpp
AMyActor           // Actor classes start with 'A'
UMyObject          // UObject classes start with 'U'
UMyComponent       // Component classes start with 'U'
FMyStruct          // Structs start with 'F'
EMyEnum            // Enums start with 'E'
IMyInterface       // Interfaces start with 'I'
TMyTemplate        // Templates start with 'T'
```

**Variables:**
```cpp
int32 MyVariable              // PascalCase for most variables
int32 MemberVariable          // Member variables
bool bIsActive                // Boolean prefix 'b'
float fTemperature            // Float prefix 'f' (optional)
TArray<AActor*> ActorArray    // Descriptive names
```

**Functions:**
```cpp
void DoSomething()                      // PascalCase
void OnEventTriggered()                 // Event callbacks with 'On' prefix
bool IsValid() const                    // Boolean getters with 'Is', 'Has', 'Can'
AActor* GetOwner() const                // Getters with 'Get' prefix
void SetHealth(float NewHealth)         // Setters with 'Set' prefix
```

**Constants:**
```cpp
const float MAX_HEALTH = 100.0f;        // ALL_CAPS for constants
static const FName TAG_Enemy;           // Static constants
```

#### Code Formatting

**Braces:**
```cpp
// Always use braces, even for single-line blocks
if (bCondition)
{
    DoSomething();
}
else
{
    DoSomethingElse();
}

// Functions
void MyFunction()
{
    // Function body
}

// Classes
class AMyActor : public AActor
{
    GENERATED_BODY()
    
public:
    // Public members
    
protected:
    // Protected members
    
private:
    // Private members
};
```

**Indentation:**
- Use tabs, not spaces
- Tab size: 4 spaces equivalent
- Visual Studio handles this automatically

**Line Length:**
- Aim for 120 characters max
- Break long lines logically

**Comments:**
```cpp
// Single-line comment for brief explanations

/**
 * Multi-line comment for function/class documentation
 * @param ParamName Description of parameter
 * @return Description of return value
 */
void MyFunction(int32 ParamName);
```

#### UPROPERTY Specifiers

```cpp
// EditAnywhere: Editable in editor and instances
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "My Category")
float MyValue;

// VisibleAnywhere: Read-only in editor
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "My Category")
UMyComponent* MyComponent;

// Replicated: Synchronized across network
UPROPERTY(Replicated, BlueprintReadOnly, Category = "Networking")
int32 PlayerScore;
```

#### UFUNCTION Specifiers

```cpp
// Blueprint callable C++ function
UFUNCTION(BlueprintCallable, Category = "My Category")
void MyFunction();

// Blueprint implementable event
UFUNCTION(BlueprintImplementableEvent, Category = "Events")
void OnSomethingHappened();

// Blueprint native event (can be overridden)
UFUNCTION(BlueprintNativeEvent, Category = "Events")
void OnInteract();
virtual void OnInteract_Implementation();

// Network RPCs
UFUNCTION(Server, Reliable, WithValidation)
void ServerDoSomething();

UFUNCTION(Client, Reliable)
void ClientNotify();

UFUNCTION(NetMulticast, Reliable)
void MulticastEffect();
```

### Blueprint Best Practices

**Organization:**
- Use comments to explain complex logic
- Group related nodes
- Use functions to break down complex graphs
- Name variables descriptively

**Performance:**
- Avoid Tick when possible
- Use timers instead of constant checks
- Cache references instead of repeated GetComponent calls
- Use interfaces for polymorphism

**Networking:**
- Understand authority and replication
- Minimize replicated properties
- Use RPCs appropriately
- Validate client input on server

### Asset Naming Conventions

Follow consistent naming for assets:

```
T_AssetName        # Textures
M_AssetName        # Materials
MI_AssetName       # Material Instances
SM_AssetName       # Static Meshes
SK_AssetName       # Skeletal Meshes
SK_AssetName_Anim  # Animations
BP_AssetName       # Blueprints
WBP_AssetName      # Widget Blueprints
DA_AssetName       # Data Assets
```

**Folder Structure:**
```
Content/
├── Blueprints/
├── Characters/
├── Maps/
├── Materials/
├── Meshes/
│   ├── Environment/
│   └── Characters/
├── Textures/
└── Audio/
```

---

## Pull Request Process

### Before Submitting

- [ ] Code follows style guidelines
- [ ] All changes are tested
- [ ] No compiler warnings
- [ ] Documentation updated (if needed)
- [ ] Commits are clean and descriptive
- [ ] Branch is up to date with main

### PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Related Issues
Fixes #123

## Testing
How to test these changes

## Screenshots (if applicable)
Add screenshots for UI changes

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-reviewed code
- [ ] Commented complex code
- [ ] Updated documentation
- [ ] No new warnings
- [ ] Tested changes
```

### Review Process

1. **Automated Checks:**
   - Build must succeed
   - No compiler errors/warnings

2. **Code Review:**
   - At least one approval required
   - Address review comments
   - Update PR as needed

3. **Testing:**
   - Verify changes work as expected
   - Test edge cases
   - Check multiplayer (if applicable)

4. **Merge:**
   - Squash commits if needed
   - Merge to main branch
   - Delete feature branch

---

## Commit Guidelines

### Commit Message Format

```
<type>: <subject>

<body>

<footer>
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting)
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Adding tests
- `chore`: Maintenance tasks

### Examples

**Good Commits:**
```bash
feat: Add hacking mini-game for future timeline

- Implement grid-based puzzle
- Add timer and difficulty levels
- Integrate with ability system

Closes #45

---

fix: Correct AI detection range calculation

The detection range was incorrectly using squared distance.
Changed to use actual distance for consistent behavior.

Fixes #67

---

docs: Update README with multiplayer setup instructions

Added section on Steam integration and testing local multiplayer.
```

**Bad Commits:**
```bash
# Too vague
"Update stuff"
"Fix bug"
"Changes"

# Too long/unfocused
"Fix AI, update UI, refactor inventory, add new feature"
```

### Commit Best Practices

- One logical change per commit
- Write clear, concise messages
- Use present tense ("Add feature" not "Added feature")
- Reference issues/PRs when relevant
- Keep commits small and focused

---

## Testing Guidelines

### Manual Testing

**Required Tests:**

1. **Functionality:**
   - Test the feature/fix thoroughly
   - Test edge cases
   - Test error conditions

2. **Regression:**
   - Ensure existing features still work
   - Test related systems

3. **Performance:**
   - Check frame rate impact
   - Profile if making performance changes

4. **Multiplayer:**
   - Test as client and server
   - Test with different player counts
   - Test network lag scenarios

### Automated Testing

If adding C++ unit tests:

```cpp
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMyFeatureTest,
    "Splitstream.MyFeature.BasicTest",
    EAutomationTestFlags::ApplicationContextMask | 
    EAutomationTestFlags::ProductFilter
)

bool FMyFeatureTest::RunTest(const FString& Parameters)
{
    // Arrange
    int32 Expected = 42;
    
    // Act
    int32 Actual = MyFunction();
    
    // Assert
    TestEqual("Function returns correct value", Actual, Expected);
    
    return true;
}
```

---

## Documentation

### Code Documentation

**Classes:**
```cpp
/**
 * Manages player inventory and item interactions.
 * Handles adding, removing, and equipping items.
 * Replicates inventory state across network.
 */
UCLASS()
class SPLITSTREAM_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()
    
public:
    /**
     * Adds an item to the inventory.
     * @param Item The item data asset to add
     * @param Quantity Number of items to add
     * @return true if item was successfully added
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(UItemDataAsset* Item, int32 Quantity = 1);
};
```

**Complex Logic:**
```cpp
// Calculate detection level based on distance and visibility
// Uses exponential falloff for more realistic detection
float DetectionValue = FMath::Exp(-Distance / DetectionRange);
if (bHasLineOfSight)
{
    DetectionValue *= VisibilityMultiplier;
}
```

### README Updates

When adding features, update:
- Feature list
- Usage examples
- Configuration options
- Troubleshooting section

### Changelog

Major changes should be noted for release notes:
- New features
- Breaking changes
- Important bug fixes
- Deprecations

---

## Questions?

- **Documentation:** Check [README.md](../README.md), [ARCHITECTURE.md](ARCHITECTURE.md), [BUILD_GUIDE.md](BUILD_GUIDE.md)
- **Issues:** Search existing GitHub issues
- **Discussion:** Open a GitHub Discussion
- **Contact:** Create an issue with your question

---

## License

By contributing, you agree that your contributions will be licensed under the same license as the project (All Rights Reserved).

---

Thank you for contributing to Splitstream! 🎮

---

**Last Updated**: January 2025
