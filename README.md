# BuildDemo

Proof-of-concept for integrating container image builds into the Visual Studio MSBuild pipeline with full incremental build support.

## Projects

| Project | Type | Description |
|---------|------|-------------|
| CppClient | C++ (.vcxproj) | Windows console app that communicates with containers via TCP sockets |
| CSharpClient | C# (.csproj) | C# console app for validating the C# incremental build path |

## Container Images

| Image | Port | Description |
|-------|------|-------------|
| echo-server | 9000 | TCP server that echoes messages back unchanged |
| uppercase-worker | 9001 | TCP server that returns messages in uppercase |

Container source files are in `CppClient/container/`.

## How It Works

Projects declare `<WslcImage>` items and import `WslcBuildImage.targets`. Docker images are built automatically as part of the VS build, with incremental support:

- Only images whose source files changed are rebuilt
- VS detects container file changes via tlog (C++) or CPS UpToDateCheck (C#)
- MSBuild `Inputs`/`Outputs` with marker files provides per-image granularity

## Build

Open `TestApp.sln` in Visual Studio 2022 and build, or from command line:

```bash
# C++ project
msbuild CppClient/CppClient.vcxproj -p:Configuration=Debug -p:Platform=x64

# C# project
dotnet build CSharpClient/CSharpClient.csproj
```
