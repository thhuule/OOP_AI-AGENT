# Error Recovery

When a step fails, do not give up immediately. Try to recover before reporting failure.

## Rules
- Maximum 2 retry attempts per step
- Each retry must use a DIFFERENT approach than the previous attempt
- If all retries fail, report clearly: which step failed, what was tried, what the error was
- Never retry the exact same tool call with the exact same input

## Recovery strategies by error type

| Error | Strategy |
|---|---|
| Tool returned empty output | Retry with simpler/shorter input |
| Tool returned error message | Switch to a different tool that can do the same job |
| File not found | Use `file_write` to create it first, then retry |
| Calculation wrong | Re-check the input values, retry with corrected input |
| Web search no results | Simplify the search query, retry |

## Steps
1. Step FAILED → identify the error type from the table above
2. Choose recovery strategy
3. Retry (attempt 1) with new approach
4. If still failed → retry (attempt 2) with another approach
5. If still failed after 2 retries → stop and report:
   - Step that failed
   - Error message
   - What was tried (attempt 1, attempt 2)

## Example
Step: `web_search("latest GDP growth rate of Vietnam in Q3 2024")` → empty result

Recovery:
- Attempt 1: simplify query → `web_search("Vietnam GDP 2024")` → got result ✓
- Proceed to next step

Step: `file_read("data/config.json")` → "file not found"

Recovery:
- Attempt 1: `file_write("data/config.json", "{}")` → created ✓
- Retry original step: `file_read("data/config.json")` → "{}" ✓
- Proceed to next step