# WSLC CLI vs Docker CLI - Command Comparison

> Based on `feature/wsl-for-apps` branch as of 2026-03-17

## Legend

| Symbol | Meaning |
|--------|---------|
| ✅ | Fully implemented |
| ⚠️ | Partial / has TODOs |
| 📝 | Stub (declared but empty) |
| ❌ | Not available |

---

## Container Commands

| Docker Command | Docker Description | WSLC Equivalent | WSLC Status | Notes |
|---|---|---|---|---|
| `docker run` | Create and run a container | `wslc run` / `wslc container run` | ✅ | Supports -d, -it, -e, -p, -v, --rm, --name, --entrypoint, --dns, --pull, --tmpfs, --user |
| `docker create` | Create a container | `wslc create` / `wslc container create` | ✅ | Supports --rm flag |
| `docker start` | Start a stopped container | `wslc start` / `wslc container start` | ⚠️ | `--interactive` and `--session` marked NYI |
| `docker stop` | Stop a running container | `wslc stop` / `wslc container stop` | ✅ | Supports --signal, --time |
| `docker kill` | Kill a running container | `wslc kill` / `wslc container kill` | ✅ | Supports --signal |
| `docker rm` | Remove a container | `wslc rm` / `wslc container remove` | ✅ | Supports --force |
| `docker exec` | Execute command in container | `wslc exec` / `wslc container exec` | ✅ | Supports -d, -it, -e, --env-file, --user |
| `docker attach` | Attach to a running container | `wslc attach` / `wslc container attach` | ✅ | |
| `docker ps` / `docker container ls` | List containers | `wslc ps` / `wslc container ls` | ✅ | Supports --all, --format (json/table), --quiet |
| `docker logs` | View container logs | `wslc logs` / `wslc container logs` | ✅ | Supports --follow. TODO: Ctrl-C handling |
| `docker inspect` (container) | Container details | `wslc inspect` / `wslc container inspect` | ✅ | JSON output, supports multiple IDs |
| `docker commit` | Create image from container | ❌ | ❌ | Not available |
| `docker cp` | Copy files between container and host | ❌ | ❌ | Not available |
| `docker diff` | Inspect filesystem changes | ❌ | ❌ | Not available |
| `docker export` | Export container filesystem | ❌ | ❌ | Not available |
| `docker pause` | Pause a container | ❌ | ❌ | Not available |
| `docker unpause` | Unpause a container | ❌ | ❌ | Not available |
| `docker port` | List port mappings | ❌ | ❌ | Port mapping is set at create/run time via -p |
| `docker rename` | Rename a container | ❌ | ❌ | Not available |
| `docker restart` | Restart a container | ❌ | ❌ | Can be done manually with stop + start |
| `docker stats` | Live resource usage | ❌ | ❌ | Not available |
| `docker top` | Display running processes | ❌ | ❌ | Not available |
| `docker update` | Update container resources | ❌ | ❌ | Not available |
| `docker wait` | Wait for container to stop | ❌ | ❌ | Not available |
| `docker container prune` | Remove stopped containers | ❌ | ❌ | Not available |

---

## Image Commands

| Docker Command | Docker Description | WSLC Equivalent | WSLC Status | Notes |
|---|---|---|---|---|
| `docker build` | Build image from Dockerfile | `wslc build` / `wslc image build` | ✅ | Supports --tag, --file |
| `docker pull` | Pull image from registry | `wslc pull` / `wslc image pull` | ✅ | Supports --scheme, --progress |
| `docker images` / `docker image ls` | List images | `wslc images` / `wslc image ls` | ✅ | Supports --format, --quiet, --verbose |
| `docker rmi` / `docker image rm` | Remove image | `wslc image delete` | ✅ | Supports --force, --no-prune |
| `docker image inspect` | Image details | `wslc image inspect` | ✅ | JSON output, supports multiple IDs |
| `docker load` | Load image from tar archive | `wslc image load` | ⚠️ | Supports --input. TODO: stdin support |
| `docker push` | Push image to registry | ❌ | 📝 | `ImageService::Push()` declared but empty |
| `docker save` | Save image to tar archive | ❌ | 📝 | `ImageService::Save()` declared but empty |
| `docker tag` | Tag an image | ❌ | 📝 | `ImageService::Tag()` declared but empty |
| `docker history` | Show image build history | ❌ | ❌ | Not available |
| `docker import` | Import from tarball | ❌ | ❌ | Available in SDK (`WslcSessionImageImport`) but not exposed in CLI |
| `docker image prune` | Remove unused images | ❌ | 📝 | `ImageService::Prune()` declared but empty |
| `docker search` | Search Docker Hub | ❌ | ❌ | Not available |

---

## Session Commands (WSLC-specific, no Docker equivalent)

| WSLC Command | Description | Status | Notes |
|---|---|---|---|
| `wslc session ls` | List active sessions | ✅ | Supports --verbose. Shows SessionId, CreatorPid, DisplayName |
| `wslc session shell` | Attach to a session shell | ✅ | Opens /bin/sh with TTY support |

> Docker has no equivalent — Docker Desktop runs a single daemon. WSLC uses sessions to manage lightweight VM instances.

---

## Network Commands

| Docker Command | Docker Description | WSLC Equivalent | Status |
|---|---|---|---|
| `docker network create` | Create a network | ❌ | ❌ |
| `docker network connect` | Connect container to network | ❌ | ❌ |
| `docker network disconnect` | Disconnect from network | ❌ | ❌ |
| `docker network inspect` | Network details | ❌ | ❌ |
| `docker network ls` | List networks | ❌ | ❌ |
| `docker network rm` | Remove a network | ❌ | ❌ |
| `docker network prune` | Remove unused networks | ❌ | ❌ |

> WSLC supports networking modes (none/bridged) set at container create time, but has no standalone network management commands.

---

## Volume Commands

| Docker Command | Docker Description | WSLC Equivalent | Status |
|---|---|---|---|
| `docker volume create` | Create a volume | ❌ | ❌ |
| `docker volume inspect` | Volume details | ❌ | ❌ |
| `docker volume ls` | List volumes | ❌ | ❌ |
| `docker volume rm` | Remove a volume | ❌ | ❌ |
| `docker volume prune` | Remove unused volumes | ❌ | ❌ |

> WSLC supports volume mounts via `--volume` flag at container create/run time, but has no standalone volume management commands.

---

## System Commands

| Docker Command | Docker Description | WSLC Equivalent | Status |
|---|---|---|---|
| `docker version` | Show version info | `wslc --version` | ✅ |
| `docker info` | System-wide information | ❌ | ❌ |
| `docker system df` | Show disk usage | ❌ | ❌ |
| `docker system events` | Real-time events | ❌ | ❌ |
| `docker system prune` | Remove all unused data | ❌ | ❌ |

---

## Compose / Orchestration

| Docker Command | Docker Description | WSLC Equivalent | Status |
|---|---|---|---|
| `docker compose` | Multi-container orchestration | ❌ | ❌ |
| `docker swarm` | Cluster management | ❌ | ❌ |
| `docker service` | Swarm services | ❌ | ❌ |
| `docker stack` | Swarm stacks | ❌ | ❌ |
| `docker context` | Manage contexts | ❌ | ❌ |
| `docker plugin` | Manage plugins | ❌ | ❌ |

---

## Summary Statistics

### WSLC CLI Implementation Coverage

| Category | Docker Commands | WSLC Implemented | WSLC Partial | WSLC Stub | WSLC Missing |
|---|---|---|---|---|---|
| Container | 24 | 11 | 1 | 0 | 12 |
| Image | 13 | 5 | 1 | 4 | 3 |
| Session | — | 2 (WSLC-only) | 0 | 0 | — |
| Network | 7 | 0 | 0 | 0 | 7 |
| Volume | 5 | 0 | 0 | 0 | 5 |
| System | 5 | 1 | 0 | 0 | 4 |
| Compose/Orchestration | 6+ | 0 | 0 | 0 | 6+ |

### Overall

- **Fully implemented:** 19 commands + 2 WSLC-specific
- **Partial:** 2 (container start, image load)
- **Stub (declared, not implemented):** 4 (push, save, tag, prune)
- **Not available:** 37+

### Key Gaps for Developer Workflows

| Priority | Missing Feature | Impact |
|---|---|---|
| **High** | `image push` | Cannot publish images to a registry |
| **High** | `image tag` | Cannot re-tag images for versioning |
| **High** | `image save` | Cannot export images for transfer |
| **Medium** | `cp` | Cannot copy files to/from running containers |
| **Medium** | `image prune` / `system prune` | No way to clean up old dangling images |
| **Medium** | `restart` | Must manually stop + start |
| **Low** | `stats` / `top` | No runtime monitoring |
| **Low** | `network` commands | No standalone network management |
| **Low** | `compose` | No multi-container orchestration |

### Known TODOs in Codebase

| File | Issue |
|---|---|
| `ContainerService.cpp:337` | Ctrl-C handling not implemented in Logs |
| `ContainerService.cpp:218` | Error message and detach keys not handled in Run |
| `ContainerService.cpp:247` | Error message and detach keys not handled in Start |
| `ContainerStartCommand.cpp:31-32` | `--interactive` and `--session` marked NYI |
| `ImageTasks.cpp:147` | stdin support not implemented for Image Load |
| `ConsoleService.cpp:51` | Detach sequence hardcoded (ctrl-p, ctrl-q) |
| `ConsoleService.cpp:93` | CR vs LF issue in interactive console |
| `SessionModel.cpp:24` | Configuration file support not implemented |
