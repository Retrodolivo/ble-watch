# CI with GitHub Actions and a self-hosted runner (Step 1b)

## Words
- **Workflow**: a YAML file in `.github/workflows/`. It says *when* to run (`on:`) and *what* to run (`jobs:`).
- **Job**: a list of steps that run on one machine. **Step**: one command (`run:`) or one ready-made action (`uses:`).
- **Runner**: the program that picks up jobs and runs them. GitHub has its own (GitHub-hosted); ours runs on the VPS (self-hosted).
- **Label**: a tag on a runner. `runs-on: [self-hosted, linux, x64, ble-watch]` picks a runner that has all these labels.
- **Artifact**: files a job saves (here the firmware). You download them from the run page.

## How our build works
1. A push to `main` creates a job. The runner on the VPS (user `retroci`, systemd service) takes it.
2. `actions/checkout` puts the repo in `~/actions-runner/_work/ble-watch/ble-watch`.
3. `ci/build.sh` starts Nordic's toolchain container `ghcr.io/nrfconnect/sdk-nrf-toolchain:v3.4.2`.
   The image has only the tools (west, CMake, Zephyr SDK). The NCS sources live outside the container,
   in `~/ncs-cache/v3.4.2`, so they are downloaded only once (`west update --narrow -o=--depth=1`: shallow copies, less disk).
4. `west build -p` builds MCUboot + the app (sysbuild), and `actions/upload-artifact` saves the files.

The NCS version is in one place: `NCS_VERSION` in `ci/build.sh`. It picks both the image tag and the NCS sources, so they always match.

## Why a container
The VPS runs Debian 11 with old system libraries. Inside the container it's Ubuntu 24.04 with exactly the toolchain NCS v3.4.2 was tested with.
`--user $(id -u):$(id -g)` makes the build files belong to `retroci`, not root, so the next checkout can clean them.

## Security
- A self-hosted runner runs any code a workflow gives it, and `retroci` can use Docker, which is as good as root on the VPS (and the VPN containers live there too).
- A pull request from a fork can bring its own workflow file. So: the repo setting *Settings > Actions > General > "Require approval for all external contributors"* must be on, and our workflow has no `pull_request` trigger.
- Only people with push access can start a build.
- `permissions: contents: read`: the job's GitHub token can only read the repo.

## Limits of the VPS
1 CPU, 2 GB RAM (+2.5 GB swap). `concurrency:` makes builds wait in line instead of running at the same time.
Never run a global `docker system prune -a` there: it could delete the VPN images.
