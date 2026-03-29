# GitHub Actions & CI/CD Blueprint: The "Auto-Pip" System
This document explains how we build and ship `rate-limiter-toolkit` as a ready-to-install Python package.

## 1. The Build Philosophy (Cibuildwheel)
Because we use **C++**, the user needs a binary (a "Wheel").
- We use **GitHub Actions** as our factory.
- We use **cibuildwheel** to compile the C++ code for every OS (Windows, Linux, macOS).
- **Result:** When you push code, GitHub builds 10+ versions of your package and uploads them to PyPI.

## 2. Step-by-Step CI Setup
1.  **Repo Setup:** We create `.github/workflows/release.yml`.
2.  **Environment Secrets:** You add your `PYPI_TOKEN` to GitHub Secrets.
3.  **Docker (Linux):** GitHub uses a Docker container (manylinux) to ensure the package works on all Linux distributions. You don't need Docker on your machine; GitHub handles it.
4.  **Trigger:** Whenever you create a new "Release" or "Tag", the build starts automatically.

## 3. Local "Quick-Build" (No Docker)
To build and test on your machine *right now*:
```bash
pip install pybind11
cmake -B build
cmake --build build --config Release
# This creates the .pyd (Windows) or .so (Linux) file.
```

## 4. Why this matters for the end user?
They run: `pip install rate-limiter-toolkit`.
They **don't** need C++, CMake, or GCC. They only need Python. Our CI system does all the hard work for them.
