# Localization Contract

A header-only **contract** for localization in [O3DE](https://o3de.org): *"give me the text for this key"*, *"which language is current"* and *"the texts have changed"*. It contains no implementation, no module and no DLL.

Gems that show text to the player depend on this contract, not on a particular localization system. The text itself comes from whichever gem implements the contract, for example [LocalizationGem](https://github.com/Dmytro-Personal-O3DE-Gems/LocalizationGem). If no implementation is active, consumers simply show their own default texts.

- A request interface: `GetLocalizedString(key)` and `GetCurrentLanguage()`, reachable both as an `AZ::Interface` and as an EBus.
- A notification bus: `OnLocalizedStringsChanged()`, sent when the language or the texts change.
- `LOCALIZATION_KEY("door.open")`: a marker for keys written in C++, so tools can find them.

## Contents

- [Why a separate contract gem](#why-a-separate-contract-gem)
- [What is inside](#what-is-inside)
- [Requirements](#requirements)
- [Installation](#installation)
- [Using it: showing text](#using-it-showing-text)
- [Implementing it](#implementing-it)
- [Notes](#notes)
- [Status and roadmap](#status-and-roadmap)
- [License](#license)

## Why a separate contract gem

A UI gem such as [InteractionSystemUI](https://github.com/Dmytro-Personal-O3DE-Gems/InteractionSystemUI) needs translated text, but it should not force one specific localization system on every project that uses it. So the dependency points at a small set of headers instead:

```
InteractionSystemUI ──┐                            ┌── LocalizationGem   (implements it)
another UI gem ───────┼──> LocalizationContract <──┤
your game code ───────┘                            └── (or your own implementation)
```

- A consumer gem builds and runs with **no** localization at all: it shows its default texts.
- The implementation can be replaced without touching any consumer.
- The contract compiles to nothing, so depending on it costs nothing at runtime.

## What is inside

```
LocalizationContract/
├── Code/Include/LocalizationContract/
│   ├── LocalizationContractBus.h               # LocalizationContractRequests: GetLocalizedString, GetCurrentLanguage
│   │                                           # + LocalizationContractInterface and LocalizationContractRequestBus
│   ├── LocalizationContractNotificationBus.h   # LocalizationContractNotifications: OnLocalizedStringsChanged
│   ├── LocalizationContractTypeIds.h           # the TypeId of the request interface
│   └── LocalizationKey.h                       # the LOCALIZATION_KEY marker macro
├── Code/CMakeLists.txt                         # one INTERFACE target: Gem::LocalizationContract.API
└── gem.json
```

## Requirements

| | |
|---|---|
| Engine | O3DE **26.05** (developed and tested against tag `2605.0`) |
| Gems | none |
| For actual texts | a gem that implements the contract, for example [LocalizationGem](https://github.com/Dmytro-Personal-O3DE-Gems/LocalizationGem) |

## Installation

1. Clone the repository somewhere outside the engine, for example `D:\O3DE\UserGems\LocalizationContract`.
2. Register it with the engine:
   ```
   o3de register --gem-path D:\O3DE\UserGems\LocalizationContract
   ```
3. Enable it in your project together with the gem that implements it: Project Manager → *Configure Gems* → **Localization Contract**, or add `"LocalizationContract"` to `gem_names` in `project.json`.

To use it in your own gem or project, both of these are required:

- `"LocalizationContract"` in the `dependencies` of your `gem.json`. Otherwise a project that enables only your gem has no `Gem::LocalizationContract.API` target, and CMake fails.
- The API target in CMake:

  ```cmake
  BUILD_DEPENDENCIES
      PRIVATE   # PUBLIC if one of your headers derives from a handler of the contract's buses
          Gem::LocalizationContract.API
  ```

## Using it: showing text

### Requesting a text

The usual pattern: the consumer stores a **key** and a **default text**, asks for the key and falls back to its default.

```cpp
#include <LocalizationContract/LocalizationContractBus.h>

AZStd::string GetDisplayText(const AZStd::string& key, const AZStd::string& defaultText)
{
    // nullptr when no implementation is active: then the default text is all there is.
    // Do not keep the pointer: ask again each time.
    if (auto* localization = LocalizationContract::LocalizationContractInterface::Get())
    {
        if (AZStd::optional<AZStd::string> text = localization->GetLocalizedString(key))
        {
            return *text;
        }
    }
    return defaultText;
}
```

The same request through the bus. Initialize the result to "not found" first, because with no implementation connected `BroadcastResult` leaves it untouched:

```cpp
AZStd::optional<AZStd::string> text; // "not found"
LocalizationContract::LocalizationContractRequestBus::BroadcastResult(
    text, &LocalizationContract::LocalizationContractRequests::GetLocalizedString, key);
```

- `GetLocalizedString(key)` returns the text in the current language, or an empty `optional` if there is none. An implementation may apply its own fallback first (LocalizationGem tries English), so "empty" means "nothing at all": show your default text.
- The text is returned **by value** on purpose: the implementation may rebuild its tables when the language changes.
- `GetCurrentLanguage()` returns the current code, for example `"en"`, `"es"`, `"uk"`, or an empty string if it is unknown.

### Reacting to a language change

UI that sets its text once (a menu, a static label) should listen and request its texts again. UI that asks for its text every frame does not need to.

```cpp
#include <LocalizationContract/LocalizationContractNotificationBus.h>

// A sketch: AZ_COMPONENT, Reflect() and the rest of the component are left out.
class MyMenuComponent
    : public AZ::Component
    , public LocalizationContract::LocalizationContractNotificationBus::Handler
{
    void Activate() override { LocalizationContract::LocalizationContractNotificationBus::Handler::BusConnect(); }
    void Deactivate() override { LocalizationContract::LocalizationContractNotificationBus::Handler::BusDisconnect(); }

    void OnLocalizedStringsChanged() override
    {
        // Request every text this menu shows again.
    }
};
```

The event carries no data on purpose: listeners ask for what they need. It has an empty default body, so new events can be added later without breaking existing listeners.

### Writing keys in code: `LOCALIZATION_KEY`

```cpp
#include <LocalizationContract/LocalizationKey.h>

label.m_key = LOCALIZATION_KEY("door.open");
label.m_defaultText = "Open";
```

```cpp
#define LOCALIZATION_KEY(key) "" key
```

- **For the compiler** it is just the string `"door.open"`: two string literals next to each other are joined into one, so it costs nothing.
- **For tools** it is a marker. LocalizationGem's checking tool finds the keys in source code by searching for `LOCALIZATION_KEY("…")`.
- **The `""` in front** makes anything but a string literal fail to compile: `LOCALIZATION_KEY(myVariable)` becomes `"" myVariable`, which is not valid C++. A key a tool could not see (a variable, a key built at runtime) is rejected right away.

Write every key in C++ through it, and keep keys simple: Latin letters, digits, `.` and `_`.

## Implementing it

An implementation is usually the system component of a localization gem. The rules, from `LocalizationContractBus.h`:

1. Provide **both** access paths: register in `LocalizationContractInterface` **and** connect to `LocalizationContractRequestBus`. Consumers may use either.
2. Only **one** implementation may be active at a time.
3. Send `OnLocalizedStringsChanged` after the current language or the texts change.

```cpp
#include <LocalizationContract/LocalizationContractBus.h>
#include <LocalizationContract/LocalizationContractNotificationBus.h>

// A sketch: AZ_COMPONENT, Reflect(), services and the rest of the component are left out.
class MyLocalizationSystemComponent
    : public AZ::Component
    , public LocalizationContract::LocalizationContractRequestBus::Handler
{
public:
    AZStd::optional<AZStd::string> GetLocalizedString(const AZStd::string& key) const override;
    AZStd::string GetCurrentLanguage() const override;

protected:
    void Activate() override
    {
        // Load the texts first, then go live, so nobody can reach a half-ready component.
        LocalizationContract::LocalizationContractRequestBus::Handler::BusConnect();

        // Register() does not log anything on failure; it only returns an Outcome.
        const auto outcome = LocalizationContract::LocalizationContractInterface::Register(this);
        AZ_Error("MyLocalization", outcome.IsSuccess(),
            "Failed to register as the LocalizationContract implementation: %s", outcome.GetError().c_str());
    }

    void Deactivate() override
    {
        if (LocalizationContract::LocalizationContractInterface::Get() == this)
        {
            LocalizationContract::LocalizationContractInterface::Unregister(this);
        }
        LocalizationContract::LocalizationContractRequestBus::Handler::BusDisconnect();
    }
};

// After a successful language change:
LocalizationContract::LocalizationContractNotificationBus::Broadcast(
    &LocalizationContract::LocalizationContractNotifications::OnLocalizedStringsChanged);
```

A second implementation is caught by `Register`: it fails and returns the reason, which is why the sketch logs it.

## Notes

- **Nothing in this gem is compiled.** An `INTERFACE` target has no `.cpp` files, so a mistake in these headers shows up only when a gem that includes them is built.
- **New header?** Add it to `Code/localizationcontract_api_files.cmake`, otherwise it is not part of the `.API` target.
- **C++ only for now.** A header-only gem has no module and cannot reflect its buses to Script Canvas or Lua by itself.

## Status and roadmap

Tested in the Editor (O3DE 26.05, Windows): LocalizationGem as the implementation, InteractionSystemUI as a consumer through `LocalizationContractInterface`, the `LOCALIZATION_KEY` marker in a project.

Planned or not yet verified:
- Requesting text through `LocalizationContractRequestBus` (the `AZ::Interface` path is the tested one).
- A real listener of `OnLocalizedStringsChanged` (the implementation sends it; no consumer listens yet).
- Script Canvas / Lua access.

## License

[MIT](LICENSE) © 2026 Dmytro Shyshkin
