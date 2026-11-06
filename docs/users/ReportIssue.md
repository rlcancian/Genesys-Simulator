# Reporting a problem or suggesting an improvement

In the main GenESyS GUI, select **About → Report Issue**.

1. Choose **Bug** (default), **Suggestion / Enhancement**, or **Question**.
2. Enter a **Title** (up to 256 characters) and **Description**. Both are required.
3. Select **Continue on GitHub** if the application is using its normal browser workflow. Your browser opens the GenESyS GitHub issue form with those fields filled in. You may need to sign in, and **you must submit the issue on GitHub**. Opening the browser does not create an issue.
4. Some maintained installations have a configured report service and show **Send Issue** instead. Successful creation is displayed only when the service returns a verified issue number and GitHub URL.

**Privacy:** No model, SimulLang source, logs, screenshots, file paths, contact information, or simulation/console output are collected automatically. In browser mode, the title and description are placed into the URL and may be stored in your browser history. Avoid including private information in the report.

If a description is too long for a safe browser URL, GenESyS opens GitHub without the description and copies the full text to your clipboard so you can paste it manually. If even the title is too long, both fields must be pasted manually. The dialog reports this explicitly.

If the browser cannot be opened, the dialog stays open and your text remains available. If the report service fails, you may retry or explicitly choose the browser workflow. A service timeout does **not** establish whether an issue was created; check GitHub before retrying.

Direct link to the issue page: https://github.com/rlcancian/Genesys-Simulator/issues/new
