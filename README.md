# Mini-JARVIS-AI-Assistant
A personal AI voice assistant built using C, Python, and PowerShell.
# 🤖 Mini JARVIS — AI Voice Assistant

A personal voice assistant project built using **C, Python, and PowerShell**, designed to bring useful assistant features to your Windows desktop.

## 📌 About the Project

Mini JARVIS is a personal assistant project that combines C-based modules with Python voice assistant scripts and PowerShell functionality. The project is organized into separate files to make its components easier to manage and develop.

This project is an ongoing learning and development project.

## ✨ Project Components

- **C Programming:** Main program and separate modules for tasks, notes, and student-related functionality.
- **Python:** Male and female voice assistant scripts.
- **PowerShell:** Voice-related script for Windows.
- **Shared Header:** `common.h` for shared C declarations and definitions.

## 🛠️ Technologies Used

- C
- Python
- PowerShell
- Git and GitHub
- Visual Studio Code

## 📁 Project Structure

```text
Mini-JARVIS-AI-Assistant/
├── .gitignore
├── common.h
├── main.c
├── notes.c
├── student.c
├── tasks.c
├── jarvis_male.py
├── jarvis_female.py
└── jarvis_voice.ps1
```

## 💻 Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/SuryanshXGit/Mini-JARVIS-AI-Assistant.git
```

### 2. Open the Project

```bash
cd Mini-JARVIS-AI-Assistant
```

Open the folder in Visual Studio Code.

### 3. Set Up the Development Environment

Install the tools required for the components you want to run:

- A C compiler, such as GCC.
- Python 3.
- Windows PowerShell for the PowerShell script.

Check the Python scripts for their imports and install any required third-party packages before running them.

### 4. Compile the C Program

If the C source files contain the program's `main()` function and all required implementations, you can try:

```bash
gcc main.c notes.c student.c tasks.c -o jarvis.exe
```

Run the resulting program on Windows with:

```powershell
.\jarvis.exe
```

If compilation fails, check the compiler output and ensure all required source files and dependencies are included.

### 5. Run the Python Assistant

Depending on which assistant you want to test, run:

```bash
python jarvis_male.py
```

Or:

```bash
python jarvis_female.py
```

The scripts may require additional packages or configuration depending on their implementation.

## 🚀 Future Improvements

- Improve voice interaction and command recognition.
- Expand task and notes management.
- Improve integration between the C and Python components.
- Add clearer setup instructions and usage examples.
- Add screenshots and demonstrations of working features.

## 🎯 Project Goal

The goal of Mini JARVIS is to build a useful personal desktop assistant while improving programming, modular development, and software integration skills.

## 👨‍💻 Author

**Suryansh**

GitHub: [@SuryanshXGit](https://github.com/SuryanshXGit)

## 📄 License

No license has been added yet. Please contact the author before reusing or redistributing this project.

