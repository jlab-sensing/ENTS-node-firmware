---
name: pre-push-lint
description: >-
  Validates code formatting and linting across Python and firmware code before committing and pushing.
  Use this skill before pushing git commits, updating PRs, or when checking code quality to ensure all
  formatting and linting standards pass without errors.
---

# Pre-Push Formatting and Linting Verification

This skill provides automated checks and runbooks to ensure that all Python and firmware code adheres to the project's formatting and linting standards prior to creating commits, pushing to remotes, or submitting pull requests.

## Tools & Linters Used

- **`ruff check`**: Fast Python linter enforcing code quality, PEP standards, dead code elimination, and import sorting.
- **`ruff format`**: Black-compatible Python code formatter.
- **`pio check`** (optional): PlatformIO static analysis for STM32 and ESP32 embedded firmware.

---

## Quick Execution

### Run All Checks

Execute the bundled verification script from the repository root:

```bash
.agents/skills/pre-push-lint/scripts/check_lint.sh
```

### Auto-Fix Formatting and Linting

To automatically reformat Python files and resolve fixable linting issues:

```bash
.agents/skills/pre-push-lint/scripts/check_lint.sh --fix
```

### Check Firmware Too (Optional)

```bash
.agents/skills/pre-push-lint/scripts/check_lint.sh --firmware
```

---

## Step-by-Step Procedure

When preparing code changes for commit or pull request:

### Step 1: Run Python Linter
Run `ruff check python` from the repository root:
```bash
ruff check python
```
If errors are reported:
- Attempt auto-fix: `ruff check --fix python`
- For manual fixes:
  - Prefix unused unpacked variables with an underscore (e.g. `_fig, ax = plt.subplots(...)`).
  - Avoid broad exception catching (`BLE001`) or add `# noqa: BLE001` where necessary.
  - Remove unused imports or extraneous `f` prefixes from static strings.

### Step 2: Verify Python Formatting
Check formatting without modifying files:
```bash
ruff format --check python
```
To apply formatting across all Python files:
```bash
ruff format python
```

### Step 3: Verify Embedded Firmware (If Applicable)
For changes touching `stm32/` or `esp32/`:
```bash
cd stm32 && pio check -e stm32
cd ../esp32 && pio check -e release
```

### Step 4: Final Verification
Re-run the verification script to ensure all checks return exit code 0:
```bash
.agents/skills/pre-push-lint/scripts/check_lint.sh
```
Only proceed with `git add`, `git commit`, and `git push` once the script outputs:
`SUCCESS: All pre-push checks passed cleanly!`
