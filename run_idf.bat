@echo off
set "MSYSTEM="
set "IDF_PATH=C:\Users\home\Documents\Codex\2026-09-15\https-github-com-folotoy-ai-passport\work\esp-idf"
set "IDF_TOOLS_PATH=C:\Users\home\Documents\Codex\2026-09-15\https-github-com-folotoy-ai-passport\work\idf-tools"
set "IDF_PYTHON_ENV_PATH=C:\Users\home\Documents\Codex\2026-09-15\https-github-com-folotoy-ai-passport\work\idf-tools\python_env\idf5.5_py3.12_env"
set "PYTHON=%IDF_PYTHON_ENV_PATH%\Scripts\python.exe"

set "PATH=C:\Users\home\Documents\Codex\2026-09-15\https-github-com-folotoy-ai-passport\work\idf-tools\tools\ninja\1.12.1;C:\Users\home\Documents\Codex\2026-09-15\https-github-com-folotoy-ai-passport\work\idf-tools\tools\cmake\3.30.2\bin;C:\Users\home\Documents\Codex\2026-09-15\https-github-com-folotoy-ai-passport\work\idf-tools\tools\riscv32-esp-elf\esp-14.2.0_20251107\riscv32-esp-elf\bin;%IDF_PYTHON_ENV_PATH%\Scripts;C:\Windows\system32;C:\Windows"

"%PYTHON%" "%IDF_PATH%\tools\idf.py" %*
