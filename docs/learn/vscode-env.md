# VS Code config without personal paths

`.vscode/launch.json` and `.vscode/settings.json` are committed, so they must work on any machine.

- `${workspaceFolder}` is the repo root, wherever it is cloned.
- `${env:NAME}` is replaced with the environment variable `NAME`. VS Code reads the environment
  **once, when it starts**. A variable set later in a terminal (e.g. by the `ncs` alias) is not seen.
  Set it, then restart VS Code.
- Variables used:

  | Variable | Example | Used for |
  |---|---|---|
  | `ZEPHYR_BASE` | `~/ncs/v3.4.2/zephyr` | SVD file (peripheral registers in the debugger) |
  | `ZEPHYR_SDK_INSTALL_DIR` | `~/zephyr-sdk-1.0.1` | `arm-zephyr-eabi-gdb` |

  These are the standard Zephyr names, so `west` uses the same ones.
- Where to set them: in `~/.zshrc`, as plain `export` lines (not inside an alias). When VS Code
  is started from the desktop, it runs your login shell once (`zsh -i -l`) to copy its
  environment, so it sees what `~/.zshrc` exports. Terminals see it too. Restart VS Code after a change.
  ```sh
  export ZEPHYR_BASE="$HOME/ncs/v3.4.2/zephyr"
  export ZEPHYR_SDK_INSTALL_DIR="$HOME/zephyr-sdk-1.0.1"
  ```
  `~/.profile` also works for the whole desktop session, but zsh itself doesn't read it
  (zsh reads `~/.zprofile` and `~/.zshrc`), and it needs a log out and in.
  Or start VS Code from a terminal where they are set: `code .`
- Check: Debug panel → start "J-Link: attach". If `gdbPath` is wrong, the error shows the path
  with an empty variable, e.g. `/gnu/arm-zephyr-eabi/bin/...`.
