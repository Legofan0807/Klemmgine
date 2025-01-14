# Changes

## Version 1.14.2

### Fixes

- Code cleanup, stability improvements.
- Updated GitHub workflow to use actions/download-artifact@v4

## Version 1.14.1

### Changes

- Improved thread creation for baking lighting.
- Added `CameraComponent::IsUsed()`, `CameraComponent::GetForwardFromScreenPosition()`, `CameraComponent::WorldToScreenPos()`

### Fixes

- Fixed an issue where Windows 10 immersive dark mode wouldn't apply when the application launches.
- Fixed an issue where having an empty scene open would display it as ".jscn" in the app title bar.

## Version 1.14.0

### Changes

- Added support for compiling with CMake (On Windows and Linux). It is now the default on Linux.
- Slightly reworked the settings page in the editor.
- Fixed an issue where UI would be placed above the debug console.
- Removed `Project::UseNetworkFunctions`. To enable network functions now, load the `NetworkSubsystem` subsystem.
- Enabled `u8char_t` again, added `StrUtil::UnicodeToAscii` and `StrUtil::AsciiToUnicode` functions to convert between `u8char_t` and `char`.
- Made the title bar dark when using dark mode using
  [DWMWA_USE_IMMERSIVE_DARK_MODE](https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/apply-windows-themes#enable-a-dark-mode-title-bar-for-win32-applications).
  The engine now requires the Windows 11 SDK (Build 22000 or higher)
  because of this, since the `DWMWA_USE_IMMERSIVE_DARK_MODE` enum value didn't exist before.

### Fixes

- Replaced some instances of `int` with `int*_t` or `size_t`.
- Fixed an incorrect compare function for the shader map.
- Fixed the engine failing to compile with `KLEMMGINE_NO_CSHARP`.
- Fixed an issue where the UI wouldn't be redrawn entirely when it should've been.

## Version 1.13.2

### Changes

- Networking fixes and improvements.
- Updated all dependencies to their latest versions.

## Version 1.13.1

### Changes

- Added a lot more documentation.
- Moved everything in CSharpCore.dll into the `Engine.Core` namespace.
- Added PointLightComponent to C#.
- Added `CollisionComponent::SetActive()`.
- Added `Engine.SceneObject.New<T>()` to C#.
- Added `Engine.NativeObject.New()` to C#.

### Fixes

- Fixed missing arguments for `Texture::CreateTexture()`.
- Fixed a hardcoded max lights value.

## Version 1.13.0

### Changes

- Moved all window functions from `Application::` to `Window::` in `AppWindow.h`.
- Moved many functions from `Application.cpp` into other files.
- Code cleanup.
- Renamed Renderable to Drawable.

### Fixes

- Fixed a crash related to saving editor panel layouts.
- Fixed some incorrect padding.
- Removed all `UIBox::SetPadding(0)` calls because they're unnecessary after 1.11.0.
- Fixed `OS::GetMemUsage()` returning incorrect values on Linux.
- Fixed spamming empty console commands when using some Linux terminals.

## Version 1.12.0

### Changes

- Added the option to save the editor layout.
- Added more documentation. (Subsystems, Sound, Editor documentation)
- Improvements to the performance graph.
- Added a constructor to Sound::SoundBuffer which constructs sounds from a byte array.

### Fixes

- Fixed issues with nested objects in save files.
- Fixed the server creating an empty .keproj file if no .sln file exists.
- Fixed the server not working with the file structure of release builds.

## Version 1.11.0

### Changes

- Renamed WorldObject to SceneObject in both C++ and C#.
- Added a performance graph to the debug UI that can be toggled with Right Shift.
- Set default padding value to 0.

## Version 1.10.15

### Changes

- The server port is no longer a constant.
- Added an error message to the crash handler if it crashes.
- Added `Sound::Enabled`.

### Fixes

- Fixed auto complete being in reverse alphabetical order.
- Fixed a server crash if the current port was already in use.
- Removed some unused #include-statements.
- Fixed `RenderSubsystem.h` being considered as a ClCompile item by msbuild.

## Version 1.10.14

### Changes

- Code cleanup.
- Improved console auto complete.
- Updated some documentation.

## Version 1.10.13

### Fixes

- Fixed some spelling mistakes in the source code.
- Removed an unused function from Build.cpp.
- Fixed a bug introduced by the changes to text wrapping in 1.10.12.

## Version 1.10.12

### Changes

- Brought over `UIBox::SizeMode::AspectRelative` and `UIBox::SizeMode::PixelRelative` from KlemmUI.
- Changed the way text wrapping works.

## Version 1.10.11

### Changes

- Renamed some variables to have clearer names.
- The `save` console command now has an optional parameter for the scene file name,

### Fixes

- Ran VC++ code analysis on entire project, fixed all found issues that weren't from 3rd party libraries.
- `ToUnicodeString()` now reserves the string length to increase the speed of the function.

## Version 1.10.10

### Changes

- Added EditorSubsystem class.
- Added a list of matching commands to the console in Editor and Debug builds.
- Moved `UICanvas.h`.
- Renamed some console commands to be more consistent with the rest.

### Fixes

- Fixed some spelling mistakes in the source code.
- `StrUtil::Format()` can now accept format arguments with unlimited size.
- When loading a new scene, if it didn't have a lightmap the lightmap of the previous scene would remain active.
- Turned off warnings as errors for JoltPhysics dependency since that would cause the build to fail.

## Version 1.10.9

### Changes

- Merged some namespaces into the `Stats` namespace.

### Fixes

- Fixed the error handler crashing.

## Version 1.10.8

### Fixes

- Renamed game build output folder to `GameBuild`
- Improved how builds on Linux work.


## Version 1.10.7

### Fixes

- Fixed some bad/incorrect math related to rotations.
- Fixed an issue where materials wouldn't be saved correctly.
- Fixed a `GL_INVALID_VALUE` error with UI rendering.


## Version 1.10.6

### Fixes

- Fixed some issues that appeared with 1.10.5


## Version 1.10.5

### Changes

- UI performance improvements. Only the necessary areas of the screen will be redrawn.


## Version 1.10.4

### Changes

- Rewritten save format parser.
- Changed some getters/setters to properties in C#.
- Lightmap format is now binary instead of text-based.


## Version 1.10.3

### Changes

- Exposed camera FOV functions to C#.

### Fixes

- Fixed issues where sometimes a scene wouldn't auto-save when running it.
- Fixed a memory leak with instanced meshes.


## Version 1.10.2

### Fixes

- Fixed leaking the formatted string in StrUtil::Format().
- Networking stability improvements.


## Version 1.10.1

### Fixes

- Fixed crashes on Linux.
- Many small stability improvements.


## Version 1.10.0

### Changes

- Improved build tool will now not assume class declarations from header file names but scan header files for class declarations.
- Added `Subsystem` interfaces.
- Log files will now be written to disk in {WorkingDirectory}/Logs/
- Improvements to text fields.


## Version 1.9.4

### Changes

- Added FXAA anti aliasing. (`aa_enabled` convar to enabled/disable)
- Removed "C#" toolbar option on projects that don't use C#.
- Added filtering options to the billboard component.

### Fixes

- Fixed issues with more than 8 lights at once.
- Fixed an issue with shadows in framebuffers that aren't the main one.
- Fixed an issue with Boolean editor properties.
- Fixed an issue with occlusion culling with meshes that have a center position that isn't 0.


## Version 1.9.3

### Changes

- Networking stability improvements.

### Fixes

- Fixed a crash related to lists in object properties.
- Fixed a crash with the physics system in certain conditions.
- Fixed scaling for some shape types for collision visualization.
- Fixed glitchy behavior of MoveComponent when colliding with multiple polygons at once.
- Fixed sound falloff being too abrupt.


## Version 1.9.2

### Changes

- Changed behavior of ResolutionScale.
- Added editor graphics settings.
- Added editor display settings.
- Text is now culled when not visible in a scroll area.
- Added a grid display in the editor.
- Added collision visualization.

### Fixes

- Improved how baked point light shadows are calculated.
- Fixed the MovementComponent sometimes letting the object to go through walls.
- Fixed a crash when a MeshComponent has no mesh loaded.
- Fixed a crash when reloading the C# assembly.


## Version 1.9.1

### Changes

- Added `bool PhysicsComponent::Attached`.
- Mesh object performance optimizations.
- Better sound falloff.

### Fixes

- Fixed some dialogs not appearing.


## Version 1.9.0

### Changes

- Added a new physics/collision system, using Jolt physics.
- Removed most of the old collision system.
- Completely rewrote MoveComponent.

### Fixes
- Fixed an issue where release builds on Linux would fail to load any C# assembly.


## Version 1.8.2

### Fixes

- Fixed a crash with rebuilding C# in the editor.
- Fixed an issue with public EditorProperties.
- Fixed an issue where C# UICanvases would be garbage collected.
- Fixed some incomplete/incorrect documentation.


## Version 1.8.1

### Changes

- `.tar.gz` files for Linux builds.
- Added UIButton C# bindings.
- Added UICanvas C# function.
- Added C# `Engine.Stats.EngineConfig`.
- Added more documentation and updated README.md to mention the Linux version.

### Fixes

- Multiple small fixes to the editor.
- Linux builds can now properly create releases.


## Version 1.8.0

### Changes

- Large editor UI rewrite.
- Added documentation using Doxygen.
- Moved engine functions into `KlemmgineCSharp` project.
- Renamed some C# functions to match their C++ equivalent.
- Added Enum `Input::GamepadType` and `Input::GetGamepadType(Input::Gamepad* From)`.
- `UIButton` and `UITextField` now inherit from `UIBackground`.
- Added `Engine.NativeObject` and a few more utility functions to C#.
- Added C# `EditorProperty` attribute and functionality.
- Upgrade to new KlemmBuild version.
- Added `setup.sh` for Linux builds.


## Version 1.7.2

### Fixes

- Fixed a crash with the server.


## Version 1.7.1

### Changes
- When adding a C# class, the path will now default to the path opened in the class browser.

### Fixes
- Fixed the `Mem` display in the editor being broken on Linux.
- Fixed server not launching on Linux when pressing `Run` in the editor.


## Version 1.7.0

### Changes
- Added controller input support. (Engine/Gamepad.h)
- Added C# bindings for controller input.
- Added Linux support.
- Added pre-built binaries for Linux (maybe?)

### Fixes
- Fixed some issues with point lights.
- Fixed issues with shadows.
- Fixed an issue with the default movement.
