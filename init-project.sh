#!/bin/bash
# init-project.sh: Standardizes new project creation with Gemini CLI guardrails

PROJECT_NAME=$1

if [ -z "$PROJECT_NAME" ]; then
  echo "Usage: ./init-project.sh <project-name>"
  exit 1
fi

echo "🚀 Initializing $PROJECT_NAME with Gemini CLI Standards..."

# 1. Create Directory Structure
mkdir -p "$PROJECT_NAME/docs/notes"
touch "$PROJECT_NAME/bugs.md"
touch "$PROJECT_NAME/check.md"
touch "$PROJECT_NAME/skills.md"
touch "$PROJECT_NAME/metrics.md"
touch "$PROJECT_NAME/enhancements.md"

# 2. Copy Global GEMINI.md as Template
if [ -f "GEMINI.md" ]; then
  cp "GEMINI.md" "$PROJECT_NAME/GEMINI.md"
  echo "✅ Copied GEMINI.md template."
else
  echo "⚠️ Root GEMINI.md not found. Creating a baseline."
  cat <<EOT > "$PROJECT_NAME/GEMINI.md"
# Project: $PROJECT_NAME
- Follow Research-First & Strategy-First workflows.
- Mandatory TDD for bug fixes.
- PRs should be < 200 lines.
EOT
fi

echo "✨ Project $PROJECT_NAME is ready. Run 'cd $PROJECT_NAME' to start."
