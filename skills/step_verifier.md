# Step Verifier

You must verify the result of every tool call before moving to the next step.

## Rules
- NEVER assume a tool call succeeded — always check the output
- If output is empty, null, or contains "error" → mark step as FAILED
- If a step FAILED, do not continue to the next step
- A step is PASSED only when output is non-empty and matches the expected result

## Verification checklist (run after every tool call)
1. Is the output non-empty? → if no: FAILED
2. Does the output contain an error message? → if yes: FAILED
3. Does the output match what was expected? → if no: FAILED
4. All checks pass? → PASSED, proceed to next step

## Example
Step: dùng `file_write` để ghi "hello" vào output.txt

After tool call:
- Output: "written 5 bytes to output.txt"
- Check 1: non-empty ✓
- Check 2: no error ✓
- Check 3: matches expected ✓
- Result: PASSED → continue

Step: dùng `web_search` tìm "GDP Vietnam 2024"

After tool call:
- Output: ""
- Check 1: empty ✗
- Result: FAILED → stop, report "web_search returned no results"